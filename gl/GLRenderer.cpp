#include "GLRenderer.h"

#include "GLInfo.h"
#include "GLPrimitives.h"
#include "GLGUI.h"
#include "GLMesh.h"
#include "GLModel.h"
#include "GLUniformBuffer.h"
#include "GLShader.h"
#include "Terrain.h"

#include "../utilities/Macros.h"
#include "../utilities/Logging.h"
#include "../utilities/GLMath.h"
#include "../utilities/GLUtils.h"
#include "../utilities/Collision.h"
#include "../utilities/collections/DenseArray.h"

int gl_version, gl_max_texture_size;
float gl_max_texture_max_anisotropy_ext;
bool wgl_context_forward_compatible;
int glsl_version;
int gl_max_combined_texture_image_units;

enum ViewportLayer
{
	LAYER_WORLD,
	LAYER_SCREEN,
	LAYER_GUI,
};

namespace GLRenderer
{
	enum UniformBlockBinding
	{
		BINDING_MATERIAL,
		BINDING_OBJECT,
	};

	GLuint fbo;
	GLuint colorBuffer, depthBuffer;

	int width, height;
	mat4x4 screenViewProjection;
	CameraData cameraData;
	Frustum frustum;

	GLModel wonk;

	static const int NUM_MODELS = 10;
	static const int NUM_SUBMESHES = 2;
	Handle mershHandles[NUM_MODELS * NUM_SUBMESHES];

	OBB boundingBoxes[NUM_MODELS * NUM_SUBMESHES];
	bool boundsVisible[NUM_MODELS * NUM_SUBMESHES];

	GLuint materialUniformBuffer = 0;
	GLuint objectUniformBuffer = 0;

	GLTexture blankTexture;
	GLTexture scaleTexture;
	GLShader defaultShader, alphaTestShader;

	DenseArray<GLMesh> renderQueue(128);

	void RenderScene();
	void RenderFinal();
}

bool GLRenderer::Initialize()
{
	// get OpenGL version info
	int major, minor;
	sscanf((char*)glGetString(GL_VERSION), "%d.%d", &major, &minor);
	gl_version = major * 10 + minor;

	sscanf((char*)glGetString(GL_SHADING_LANGUAGE_VERSION), "%d.%d", &major, &minor);
	glsl_version = major * 100 + minor;

	// get OpenGL metrics
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &gl_max_texture_size);
	glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &gl_max_combined_texture_image_units);

	if(GLEW_EXT_texture_filter_anisotropic)
	{
		glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &gl_max_texture_max_anisotropy_ext);
	}
	else
	{
		Log::Add(Log::INFO, "%s", "GL extension EXT_texture_filter_anisotropic is not available.");
	}

	// gen fbo and screen buffers
	glGenFramebuffers(1, &fbo);
	glGenTextures(1, &colorBuffer);
	glGenTextures(1, &depthBuffer);

	// initialize opengl state
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glDepthFunc(GL_LEQUAL);

	// texture initialization
	GLTexture::Initialize();

	blankTexture.Load("tex/blank.png");
	scaleTexture.Load("tex/tile4.png");

	// uniform buffers
	objectUniformBuffer = GLUniformBuffer::Create(sizeof(ObjectBlock));
	materialUniformBuffer = GLUniformBuffer::Create(sizeof(MaterialBlock));

	// shader setup
	defaultShader.Load("default_vertex.vert", "default_frag.frag");
	alphaTestShader.Load("default_vertex.vert", "alpha_test.frag");

	defaultShader.Bind();
	defaultShader.AddAttribute("position");
	defaultShader.AddAttribute("textureCoordinate");

	defaultShader.BindUniformBuffer("MaterialBlock", materialUniformBuffer, BINDING_MATERIAL);
	defaultShader.BindUniformBuffer("ObjectBlock", objectUniformBuffer, BINDING_OBJECT);
	defaultShader.SetUniformInt("texture", 0);

	alphaTestShader.Bind();
	alphaTestShader.AddAttribute("position");
	alphaTestShader.AddAttribute("textureCoordinate");

	alphaTestShader.BindUniformBuffer("MaterialBlock", materialUniformBuffer, BINDING_MATERIAL);
	alphaTestShader.BindUniformBuffer("ObjectBlock", objectUniformBuffer, BINDING_OBJECT);
	alphaTestShader.SetUniformInt("texture", 0);

	// models Init
	wonk.LoadAsMesh("meshes/Fiona.obj");

	for(int j = 0; j < NUM_MODELS; j++)
	{
		for(int i = 0; i < NUM_SUBMESHES; i++)
		{
			Handle meshHandle = renderQueue.Claim();
			mershHandles[j*NUM_SUBMESHES+i] = meshHandle;

			GLMesh* mesh = renderQueue.Get(meshHandle);
			mesh->phase = wonk.materials[i].phase;
			mesh->vertexArray = wonk.vertexArray;
			mesh->viewportLayer = LAYER_WORLD;
			mesh->material = i + 1;

			mesh->textureID = wonk.materials[i].texture;
			mesh->model = translation_matrix(j * 8, 0, 0) * rotation_matrix(j * 15, UNIT_Y);

			mesh->startIndex = wonk.materials[i].startIndex;
			GLuint endIndex = (i == NUM_SUBMESHES - 1) ? wonk.numIndices : wonk.materials[i + 1].startIndex;
			mesh->numIndices = endIndex - wonk.materials[i].startIndex;
		}
	}

	for(int i = 0; i < NUM_MODELS * NUM_SUBMESHES; i++)
	{
		boundingBoxes[i].center = vec3((i / 2) * 8, 0.0f, 0.0f);
		boundingBoxes[i].extents = vec3(2, 2, 2);
		boundingBoxes[i].axes[0] = UNIT_X;
		boundingBoxes[i].axes[1] = UNIT_Y;
		boundingBoxes[i].axes[2] = UNIT_Z;
	}

	// terrain initialization
	Terrain::Initialize();

	// camera init
	cameraData.projection = MAT_I;
	cameraData.isOrtho = false;

	screenViewProjection = MAT_I;

	GLPrimitives::Initialize();
	GUI::Initialize();

	return true;
}

void GLRenderer::Terminate()
{
	Terrain::Terminate();

	GUI::Terminate();
	GLPrimitives::Terminate();

	GLUniformBuffer::Destroy(objectUniformBuffer);
	GLUniformBuffer::Destroy(materialUniformBuffer);

	wonk.Unload();

	for(int i = 0; i < NUM_MODELS * NUM_SUBMESHES; i++)
	{
		renderQueue.Relinquish(mershHandles[i]);
	}

	defaultShader.Unload();
	alphaTestShader.Unload();

	blankTexture.Unload();
	scaleTexture.Unload();

	GLTexture::Terminate();

	glDeleteTextures(1, &colorBuffer);
	glDeleteTextures(1, &depthBuffer);
	glDeleteFramebuffers(1, &fbo);
}

void GLRenderer::Resize(int dimX, int dimY)
{
	width = dimX;
	height = dimY;

	// resize screen buffers
	glBindTexture(GL_TEXTURE_2D, colorBuffer);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, dimX, dimY, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glBindTexture(GL_TEXTURE_2D, 0);

	glBindTexture(GL_TEXTURE_2D, depthBuffer);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, dimX, dimY, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glBindTexture(GL_TEXTURE_2D, 0);

	// resize viewport and framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glViewport(0, 0, dimX, dimY);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorBuffer, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthBuffer, 0);

	GLenum buffers[] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers(1, buffers);

	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if(status != GL_FRAMEBUFFER_COMPLETE)
	{
		Log::Add(Log::ISSUE, "OpenGL Error: Framebuffer Incomplete! - %s", gl_fbstatus_text(status));
	}

	// resize projection in viewport block
	screenViewProjection = orthogonal_projection_matrix(0, width, 0, height, -2.0f * height, 0);

	// resize GUI
	GUI::Resize(dimX, dimY);
}

void GLRenderer::SetCameraState(const CameraData& camera)
{
	if(camera.isOrtho != cameraData.isOrtho)
	{
		glDepthFunc(camera.isOrtho ? GL_GEQUAL : GL_LEQUAL);
		glCullFace(camera.isOrtho ? GL_FRONT : GL_BACK);
	}
	cameraData = camera;

	frustum = Frustum(
		camera.position,
		camera.viewX,
		camera.viewY,
		camera.viewZ,
		camera.fov,
		float(width) / float(height),
		camera.nearPlane,
		camera.farPlane);
}

void GLRenderer::Render()
{
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
	RenderScene();

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	RenderFinal();
}

struct CompareMeshes
{
	bool operator()(const GLMesh& lhs, const GLMesh& rhs) const
	{
		return lhs.viewportLayer < rhs.viewportLayer
			|| lhs.phase < rhs.phase
			|| lhs.sortingLayer < rhs.sortingLayer
			|| lhs.depth < rhs.depth
			|| lhs.material < rhs.material;
	}
};

void GLRenderer::RenderScene()
{
	// set depth to read/write
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);

	// clear framebuffer
	const float clearDepth = (cameraData.isOrtho) ? 0.0f : 1.0f;
	glClearBufferfv(GL_DEPTH, 0, &clearDepth);

	const float clearColor[] = { 0.0f, 1.0f, 1.0f, 1.0f };
	glClearBufferfv(GL_COLOR, 0, clearColor);

	cull_frustum_obb_list(frustum, boundingBoxes, NUM_MODELS * NUM_SUBMESHES, boundsVisible);
	
	// sort meshes
	//renderQueue.Sort(CompareMeshes());
	
	// loop through and draw meshes
	mat4x4 view = view_matrix(cameraData.viewX, cameraData.viewY, cameraData.viewZ, cameraData.position);
	mat4x4 viewProjection = cameraData.projection * view;

	RenderPhase phase = PHASE_TRANSPARENT;
	int material = 0;
	ViewportLayer viewportLayer = LAYER_GUI;

	int cnt = 0;
	FOR_EACH(mesh, renderQueue)
	{
		//*** VIEWPORT LAYER CHANGE ***
		if(mesh->viewportLayer != viewportLayer)
		{
			viewportLayer = (ViewportLayer) mesh->viewportLayer;
			switch(viewportLayer)
			{
				case LAYER_SCREEN:
				case LAYER_GUI:
				{
					glDisable(GL_DEPTH_TEST);
					break;
				}
				case LAYER_WORLD:
				{
					break;
				}
			}
		}

		//*** PHASE CHANGE ***
		if(mesh->phase != phase)
		{
			phase = (RenderPhase) mesh->phase;
			switch(phase)
			{
				case PHASE_SOLID:
				{
					defaultShader.Bind();
					break;
				}
				case PHASE_MASKED:
				{
					alphaTestShader.Bind();
					break;
				}
				case PHASE_BACKGROUND:
				{
					glDepthMask(GL_FALSE);
					defaultShader.Bind();
					break;
				}
				case PHASE_TRANSPARENT:
				{
					glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
					break;
				}
				case PHASE_COMMUTATIVE:
				{
					break;
				}
			}
		}

		//*** MATERIAL CHANGE ***
		if(mesh->material != material)
		{
			GLUniformBuffer::BufferData(materialUniformBuffer, &mesh->materialBlock, sizeof mesh->materialBlock);
			glBindTexture(GL_TEXTURE_2D, mesh->textureID);
		}
		
		//*** DRAW CALL ***
		mesh->objectBlock.modelViewProjection = viewProjection * mesh->model;
		GLUniformBuffer::BufferData(objectUniformBuffer, &mesh->objectBlock, sizeof mesh->objectBlock);

		if(boundsVisible[cnt++])
		mesh->Draw();
	}

	ObjectBlock objectBlock;
	objectBlock.modelViewProjection = viewProjection * MAT_I;
	GLUniformBuffer::BufferData(objectUniformBuffer, &objectBlock, sizeof objectBlock);

	glBindTexture(GL_TEXTURE_2D, scaleTexture);
	Terrain::Render();

	
	for(int i = 0; i < NUM_MODELS * NUM_SUBMESHES; i++)
	{
		GLPrimitives::AddLineBox(boundingBoxes[i].center, 2 * boundingBoxes[i].extents);
	}
	GLPrimitives::Draw();

	/*
	const vec4 verts[] =
	{
		vec4(-20, -10, 0, 1),
		vec4(-10, -10, 0, 1),
		vec4(-20, 10, 0, 1),
		vec4(-20, 10, 0, 1),
		vec4(-10, -10, 0, 1),
		vec4(-10, 10, 0, 1),
	};
	const vec2 coords[] =
	{
		vec2(0.0f, 0.0f),
		vec2(1.0f, 0.0f),
		vec2(0.0f, 1.0f),
		vec2(0.0f, 1.0f),
		vec2(1.0f, 0.0f),
		vec2(1.0f, 1.0f),
	};
	GLPrimitives::AddTris(verts, coords, 6);
	GLPrimitives::Draw();

	GLPrimitives::AddLineBox(VEC3_ONE, vec3(10, 10, 10), QUAT_I);
	GLPrimitives::AddLineBox(vec3(10, 0, 0), vec3(10, 10, 10), QUAT_I);
	GLPrimitives::AddLineBox(vec3(20, 0, 0), vec3(10, 10, 10), QUAT_I);
	GLPrimitives::AddLineSphere(vec3(30, 10, 0), 10.0f);
	GLPrimitives::Draw();
	*/

	//*** SKY PASS ***
	/*
	defaultShader.Bind();
	defaultShader.SetTexture(0, blankTexture);

	const float farthest = (isOrtho) ? -0.999999f : 0.999999f;
	const vec4 skyVerts[] =
	{
		vec4(-1, -1, farthest, 1),
		vec4(1, -1, farthest, 1),
		vec4(-1, 1, farthest, 1),
		vec4(-1, 1, farthest, 1),
		vec4(1, -1, farthest, 1),
		vec4(1, 1, farthest, 1),
	};
	const vec2 skyCoords[] =
	{
		vec2(0.0f, 0.0f),
		vec2(1.0f, 0.0f),
		vec2(0.0f, 1.0f),
		vec2(0.0f, 1.0f),
		vec2(1.0f, 0.0f),
		vec2(1.0f, 1.0f),
	};
	GLPrimitives::AddTris(skyVerts, skyCoords, ARRAY_LENGTH(skyVerts));
	GLPrimitives::Draw();
	*/
}

void GLRenderer::RenderFinal()
{
	glClear(GL_COLOR_BUFFER_BIT);

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);

	glBindTexture(GL_TEXTURE_2D, colorBuffer);

	ObjectBlock objectBlock;
	objectBlock.modelViewProjection = MAT_I;
	GLUniformBuffer::BufferData(objectUniformBuffer, &objectBlock, sizeof objectBlock);

	MaterialBlock materialBlock;
	materialBlock.color = VEC4_ONE;
	GLUniformBuffer::BufferData(materialUniformBuffer, &materialBlock, sizeof materialBlock);

	const vec4 frameVerts[] =
	{
		vec4(-1, -1, 0, 1),
		vec4(1, -1, 0, 1),
		vec4(-1, 1, 0, 1),
		vec4(-1, 1, 0, 1),
		vec4(1, -1, 0, 1),
		vec4(1, 1, 0, 1),
	};
	const vec2 frameCoords[] =
	{
		vec2(0.0f, 0.0f),
		vec2(1.0f, 0.0f),
		vec2(0.0f, 1.0f),
		vec2(0.0f, 1.0f),
		vec2(1.0f, 0.0f),
		vec2(1.0f, 1.0f),
	};
	GLPrimitives::AddTris(frameVerts, frameCoords, ARRAY_COUNT(frameVerts));
	GLPrimitives::Draw();
}
