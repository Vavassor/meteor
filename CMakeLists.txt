cmake_minimum_required (VERSION 2.8)
project (Meteor)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

if (WIN32)
set (FMOD_SDK_PATH "C:/Program Files (x86)/FMOD SoundSystem/FMOD Programmers API Windows/api")
elseif (UNIX)
set (FMOD_SDK_PATH "/usr/local")
set (FMODEX_DIR "/fmodex")
endif ()

if (CMAKE_SIZEOF_VOID_P EQUAL 8)
set (64_BIT TRUE BOOL)
else (CMAKE_SIZEOF_VOID_P EQUAL 8)
set (32_BIT TRUE BOOL)
endif ()

if (WIN32)
	set (OPENGL TRUE BOOL)
	if (MSVC)
		set (DIRECTX TRUE BOOL)
	endif (MSVC)
elseif (UNIX)
	set (OPENGL TRUE BOOL)
endif ()

# ----- INCLUDE DIRECTORIES -----
set (INCLUDE_PATHS)

if (WIN32)
list (APPEND INCLUDE_PATHS ${FMOD_SDK_PATH}/inc)
elseif (UNIX)
list (APPEND INCLUDE_PATHS ${FMOD_SDK_PATH}/include${FMODEX_DIR})
endif ()

include_directories (${INCLUDE_PATHS})

# ----- LINK DIRECTORIES -----
set (LINK_PATHS ${FMOD_SDK_PATH}/lib${FMODEX_DIR})

if (WIN32)
list (APPEND LINK_PATHS ${FMOD_SDK_PATH}/lib)
elseif (UNIX)
list (APPEND LINK_PATHS ${FMOD_SDK_PATH}/lib${FMODEX_DIR})
endif ()

link_directories (${LINK_PATHS})

# ----- INCLUDE FILES -----
set (INCLUDES
	Camera.h
	CameraData.h
	Game.h
	PlatformDefines.h
	RenderPhase.h
	Sound.h
)
source_group ("Header Files" FILES ${INCLUDES})

set (UTILITIES_INCLUDES
	utilities/BitManipulation.h
	utilities/CharTypes.h
	utilities/Collision.h
	utilities/Conversion.h
	utilities/DataTypes.h
	utilities/DeviceGUID.h
	utilities/FileHandling.h
	utilities/FileStream.h
	utilities/GLMath.h
	utilities/GLUtils.h
	utilities/Logging.h
	utilities/Macros.h
	utilities/Maths.h
	utilities/MeshLoading.h
	utilities/Noise.h
	utilities/Parsing.h
	utilities/Sorting.h
	utilities/stb_image.h
	utilities/String.h
	utilities/StringUtils.h
	utilities/Textblock.h
	utilities/Timer.h
	utilities/Unicode.h
)
source_group ("Header Files\\utilities" FILES ${UTILITIES_INCLUDES})

set (COLLECTIONS_INCLUDES
	utilities/collections/AutoArray.h
	utilities/collections/DenseArray.h
	utilities/collections/HandleManager.h
	utilities/collections/Queue.h
)
source_group ("Header Files\\utilities\\collections" FILES ${COLLECTIONS_INCLUDES})

set (CONCURRENT_INCLUDES
	utilities/concurrent/Benaphore.h
	utilities/concurrent/Interlocked.h
	utilities/concurrent/LinkedQueue.h
	utilities/concurrent/MemoryFences.h
	utilities/concurrent/Mutex.h
	utilities/concurrent/Semaphore.h
)
source_group ("Header Files\\utilities\\concurrent" FILES ${CONCURRENT_INCLUDES})

set (INPUT_INCLUDES
	utilities/input/Input.h
	utilities/input/InternalGlobals.h
	utilities/input/WindowsInput.h
	utilities/input/LinuxInput.h
	utilities/input/LinuxEvDevUtils.h
)
source_group ("Header Files\\utilities\\input" FILES ${INPUT_INCLUDES})

set (GL_INCLUDES
	gl/gl_core_3_3.h
	gl/GLFont.h
	gl/GLGUI.h
	gl/GLInfo.h
	gl/GLMesh.h
	gl/GLModel.h
	gl/GLPrimitives.h
	gl/GLRenderer.h
	gl/GLShader.h
	gl/GLTexture.h
	gl/GLUniformBuffer.h
	gl/Terrain.h
)
source_group ("Header Files\\gl" FILES ${GL_INCLUDES})

set (WGL_INCLUDES
	windows/wgl_extensions.h
)

set (GLX_INCLUDES
	x11/glx_extensions.h
)

set (DX_INCLUDES
	dx/DXInfo.h
	dx/DXModel.h
	dx/DXPrimitives.h
	dx/DXRenderer.h
	dx/DXShader.h
	dx/DXTexture.h
	dx/DXUtils.h
	dx/ShaderConstant.h
)
source_group ("Header Files\\dx" FILES ${DX_INCLUDES})

set (WINDOWS_INCLUDES
	windows/resource.h
	windows/WindowsWindow.h
)

set (X11_INCLUDES
	x11/X11Window.h
	x11/XPM.h
	x11/XPMNamedColor.h
)

list (APPEND INCLUDES
	${UTILITIES_INCLUDES}
	${COLLECTIONS_INCLUDES}
	${CONCURRENT_INCLUDES}
	${INPUT_INCLUDES}
)

if (OPENGL)
	list (APPEND INCLUDES ${GL_INCLUDES})

	if (WIN32)
	list (APPEND WINDOWS_INCLUDES ${WGL_INCLUDES})
	endif (WIN32)

	if (UNIX)
	list (APPEND X11_INCLUDES ${GLX_INCLUDES})
	endif (UNIX)
endif ()

if (DIRECTX)
list (APPEND INCLUDES ${DX_INCLUDES})
endif ()

if (WIN32)
source_group ("Header Files\\windows" FILES ${WINDOWS_INCLUDES})
list (APPEND INCLUDES ${WINDOWS_INCLUDES})
endif ()

if (UNIX)
source_group ("Header Files\\x11" FILES ${X11_INCLUDES})
list (APPEND INCLUDES ${X11_INCLUDES})
endif ()

# ----- SOURCE FILES -----
set (SOURCES
    Main.cpp
    Game.cpp
    Camera.cpp
    Sound.cpp
)
source_group ("Source Files" FILES ${SOURCES})

set (UTILITIES_SOURCES
    utilities/String.cpp
    utilities/Logging.cpp
    utilities/Timer.cpp
    utilities/GLMath.cpp
    utilities/Maths.cpp
    utilities/Unicode.cpp
    utilities/FileHandling.cpp
    utilities/Conversion.cpp
    utilities/Parsing.cpp
    utilities/FileStream.cpp
    utilities/Textblock.cpp
    utilities/stb_image.c
    utilities/MeshLoading.cpp
	utilities/GLUtils.cpp
    utilities/Collision.cpp
    utilities/Noise.cpp
	utilities/BitManipulation.cpp
)
source_group ("Source Files\\utilities" FILES ${UTILITIES_SOURCES})

set (COLLECTIONS_SOURCES
	utilities/collections/HandleManager.cpp
)
source_group ("Source Files\\utilities\\collections" FILES ${COLLECTIONS_SOURCES})

set (CONCURRENT_SOURCES
	utilities/concurrent/Benaphore.cpp
	utilities/concurrent/Mutex.cpp
)
source_group("Source Files\\utilities\\concurrent" FILES ${CONCURRENT_SOURCES})

set (INPUT_SOURCES
	utilities/input/Input.cpp
	utilities/input/WindowsInput.cpp
	utilities/input/LinuxInput.cpp
	utilities/input/LinuxEvDevUtils.cpp
)
source_group ("Source Files\\utilities\\input" FILES ${INPUT_SOURCES})

set (GL_SOURCES
	gl/gl_core_3_3.c
    gl/GLTexture.cpp
    gl/GLUniformBuffer.cpp
    gl/GLShader.cpp
    gl/GLMesh.cpp
    gl/GLModel.cpp
    gl/GLPrimitives.cpp
    gl/Terrain.cpp
    gl/GLFont.cpp
    gl/GLGUI.cpp
    gl/GLRenderer.cpp
)
source_group ("Source Files\\gl" FILES ${GL_SOURCES})

set (WGL_SOURCES
	windows/wgl_extensions.c
)

set (GLX_SOURCES
	x11/glx_extensions.c
)

set (DX_SOURCES
    dx/DXUtils.cpp
    dx/DXTexture.cpp
    dx/ShaderConstant.cpp
    dx/DXShader.cpp
    dx/DXModel.cpp
    dx/DXPrimitives.cpp
    dx/DXRenderer.cpp
)
source_group ("Source Files\\dx" FILES ${DX_SOURCES})

set (WINDOWS_SOURCES
    windows/WindowsWindow.cpp
)

set (X11_SOURCES
	x11/X11Window.cpp
	x11/XPM.cpp
)

list (APPEND SOURCES
	${UTILITIES_SOURCES}
	${COLLECTIONS_SOURCES}
	${CONCURRENT_SOURCES}
	${INPUT_SOURCES}
)

if (OPENGL)
	list (APPEND SOURCES ${GL_SOURCES})
	
	if (WIN32)
	list (APPEND WINDOWS_SOURCES ${WGL_SOURCES})
	endif (WIN32)
	
	if(UNIX)
	list (APPEND X11_SOURCES ${GLX_SOURCES})
	endif (UNIX)
endif ()

if (DIRECTX)
list (APPEND SOURCES ${DX_SOURCES})
endif ()

if (WIN32)
source_group ("Source Files\\windows" FILES ${WINDOWS_SOURCES})
list (APPEND SOURCES ${WINDOWS_SOURCES})
endif ()

if (UNIX)
source_group ("Source Files\\x11" FILES ${X11_SOURCES})
list (APPEND SOURCES ${X11_SOURCES})
endif ()

add_executable (Meteor ${SOURCES} ${INCLUDES})

# ----- DEFINES -----
if (CMAKE_COMPILER_IS_GNUCXX)
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif ()
if (MSVC)
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /GR-")
endif ()

set (DEFINITIONS)

set (DEBUG_DEFINITIONS
    _DEBUG
)
set (RELEASE_DEFINITIONS
    NDEBUG
)

set_target_properties (Meteor PROPERTIES COMPILE_DEFINITIONS "${DEFINITIONS}")
set_target_properties (Meteor PROPERTIES COMPILE_DEFINITIONS_DEBUG "${DEBUG_DEFINITIONS}")
set_target_properties (Meteor PROPERTIES COMPILE_DEFINITIONS_RELEASE "${RELEASE_DEFINITIONS}")
set_target_properties (Meteor PROPERTIES COMPILE_DEFINITIONS_RELWITHDEBINFO ${RELEASE_DEFINITIONS})
set_target_properties (Meteor PROPERTIES COMPILE_DEFINITIONS_MINSIZEREL ${RELEASE_DEFINITIONS})

# ----- LINKER OPTIONS -----
if (MINGW)
set (CMAKE_EXE_LINKER_FLAGS
    "${CMAKE_EXE_LINKER_FLAGS} -municode -mwindows")
elseif (MSVC)
set (CMAKE_EXE_LINKER_FLAGS
    "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS /INCREMENTAL:NO")
endif ()

# ----- LIBRARIES -----
set (LIBRARIES)
set (DX_LIBRARIES d3d11 dxgi)
set (WINDOWS_LIBRARIES xinput9_1_0)
set (LINUX_LIBRARIES X11 udev pthread)

if (MSVC)
	if (32_BIT)
		list (APPEND LIBRARIES fmodex_vc)
	elseif (64_BIT)
		list (APPEND LIBRARIES fmodex64_vc)
	endif (32_BIT)
else ()
	if (32_BIT)
		list (APPEND LIBRARIES fmodex)
	elseif (64_BIT)
		list (APPEND LIBRARIES fmodex64)
	endif (32_BIT)
endif ()

if (WIN32)
set (GL_LIBRARIES opengl32 glu32)
elseif (UNIX)
set (GL_LIBRARIES GL GLU)
endif ()

if (OPENGL)
list (APPEND LIBRARIES ${GL_LIBRARIES})
endif ()

if (DIRECTX)
list (APPEND LIBRARIES ${DX_LIBRARIES})
endif ()

if (WIN32)
list (APPEND LIBRARIES ${WINDOWS_LIBRARIES})
endif ()

if (APPLE)
elseif (UNIX)
list (APPEND LIBRARIES ${LINUX_LIBRARIES})
endif ()

target_link_libraries (Meteor ${LIBRARIES})
