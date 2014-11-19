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

if (DIRECTX)
set (DIRECTX_SDK_DIR "C:/Program Files (x86)/Microsoft DirectX SDK (June 2010)")
endif ()

# ----- INCLUDE DIRECTORIES -----
set (INCLUDE_PATHS)

if (WIN32)
list (APPEND INCLUDE_PATHS ${FMOD_SDK_PATH}/inc)
elseif (UNIX)
list (APPEND INCLUDE_PATHS ${FMOD_SDK_PATH}/include${FMODEX_DIR})
endif ()

if (MSVC AND DIRECTX)
list (APPEND INCLUDE_PATHS ${DIRECTX_SDK_DIR}/Include)
endif ()

include_directories (${INCLUDE_PATHS})

# ----- LINK DIRECTORIES -----
set (LINK_PATHS ${FMOD_SDK_PATH}/lib${FMODEX_DIR})

if (WIN32)
list (APPEND LINK_PATHS ${FMOD_SDK_PATH}/lib)
elseif (UNIX)
list (APPEND LINK_PATHS ${FMOD_SDK_PATH}/lib${FMODEX_DIR})
endif ()

if (MSVC AND DIRECTX)
list (APPEND LINK_PATHS ${DIRECTX_SDK_DIR}/Lib/x86)
endif ()

link_directories (${LINK_PATHS})

# ----- SOURCE FILES -----
set (SOURCES
    Main.cpp
    Game.cpp
    Camera.cpp
    Sound.cpp
)
source_group ("" FILES ${SOURCES})

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
source_group ("utilities" FILES ${UTILITIES_SOURCES})

set (COLLECTIONS_SOURCES
	utilities/collections/HandleManager.cpp
)
source_group ("utilities\\collections" FILES ${COLLECTIONS_SOURCES})

set (CONCURRENT_SOURCES
	utilities/concurrent/Benaphore.cpp
	utilities/concurrent/Mutex.cpp
)
source_group("utilities\\concurrent" FILES ${CONCURRENT_SOURCES})

set (INPUT_SOURCES
	utilities/input/Input.cpp
	utilities/input/WindowsInput.cpp
	utilities/input/LinuxInput.cpp
	utilities/input/LinuxEvDevUtils.cpp
)
source_group ("utilities\\input" FILES ${INPUT_SOURCES})

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

set (WGL_SOURCES
	windows/wgl_extensions.c
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

set (WINDOWS_SOURCES
    windows/WindowsWindow.cpp
)

set (GLX_SOURCES
	x11/glx_extensions.c
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
	source_group ("gl" FILES ${GL_SOURCES})
	list (APPEND SOURCES ${GL_SOURCES})
	
	if (WIN32)
	list (APPEND WINDOWS_SOURCES ${WGL_SOURCES})
	endif (WIN32)
	
	if(UNIX)
	list (APPEND X11_SOURCES ${GLX_SOURCES})
	endif (UNIX)
endif ()

if (DIRECTX)
source_group ("dx" FILES ${DX_SOURCES})
list (APPEND SOURCES ${DX_SOURCES})
endif ()

if (WIN32)
source_group ("windows" FILES ${WINDOWS_SOURCES})
list (APPEND SOURCES ${WINDOWS_SOURCES})
endif ()

if (UNIX)
source_group ("x11" FILES ${X11_SOURCES})
list (APPEND SOURCES ${X11_SOURCES})
endif ()

add_executable (Meteor ${SOURCES})

# ----- DEFINES -----
if (CMAKE_COMPILER_IS_GNUCXX)
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif ()
if (MSVC)
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /GR-")
endif ()

set (DEFINITIONS
    _UNICODE
    UNICODE
)
set (DEBUG_DEFINITIONS
    _DEBUG
)
set (RELEASE_DEFINITIONS
    NDEBUG
)

if (OPENGL)
list (APPEND DEFINITIONS GRAPHICS_OPENGL)
endif ()

if (DIRECTX)
list (APPEND DEFINITIONS GRAPHICS_DIRECTX)
endif ()

if (UNIX)
list (APPEND DEFINITIONS X11)
endif ()

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
set (WINDOWS_LIBRARIES xinput)
set (LINUX_LIBRARIES X11 udev pthread)

if (MSVC)
	list (APPEND LIBRARIES fmodex_vc)
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

if (UNIX)
list (APPEND LIBRARIES ${LINUX_LIBRARIES})
endif ()

target_link_libraries (Meteor ${LIBRARIES})
