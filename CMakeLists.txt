cmake_minimum_required (VERSION 2.8)
project (METEOR)

if (WIN32)
set (FMOD_SDK_DIR "C:/Program Files (x86)/FMOD SoundSystem/FMOD Programmers API Windows/api")
elseif (UNIX)
set (FMOD_SDK_DIR "/usr/local")
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
set (INCLUDE_PATHS
    ${METEOR_SOURCE_DIR}
    ${FMOD_SDK_DIR}/inc
)
include_directories (${INCLUDE_PATHS})

# ----- LINK DIRECTORIES -----
set (LINK_PATHS ${FMOD_SDK_DIR}/lib)
link_directories (${LINK_PATHS})

# ----- SOURCE FILES -----
set (SOURCES
    Main.cpp
    Game.cpp
    Camera.cpp
    Sound.cpp

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
    utilities/concurrent/Benaphore.cpp
    utilities/concurrent/Mutex.cpp
    utilities/input/Input.cpp
)

set (GL_SOURCE_FILES
    utilities/GLUtils.cpp
    utilities/Collision.cpp
    utilities/Noise.cpp
    utilities/collections/HandleManager.cpp

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

set (DX_SOURCE_FILES
	utilities/BitManipulation.cpp

    dx/DXUtils.cpp
    dx/DXTexture.cpp
    dx/ShaderConstant.cpp
    dx/DXShader.cpp
    dx/DXModel.cpp
    dx/DXPrimitives.cpp
    dx/DXRenderer.cpp
)

set (WINDOWS_SOURCE_FILES
    utilities/input/WindowsInput.cpp
    windows/WindowsWindow.cpp
)

if (OPENGL)
list (APPEND SOURCES ${GL_SOURCE_FILES})
endif ()
if (DIRECTX)
list (APPEND SOURCES ${DX_SOURCE_FILES})
endif ()

if (WIN32)
list (APPEND SOURCES ${WINDOWS_SOURCE_FILES})
endif ()

add_executable (Meteor ${SOURCES})

# ----- DEFINES -----
if (MINGW)
add_definitions (-std=c++11)
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

list (APPEND DEFINITIONS ${DEBUG_DEFINITIONS})

set_target_properties (Meteor PROPERTIES COMPILE_DEFINITIONS "${DEFINITIONS}")

# ----- LINKER OPTIONS -----
if (MINGW)
set (CMAKE_EXE_LINKER_FLAGS
    "${CMAKE_EXE_LINKER_FLAGS} -municode -mwindows")
elseif (MSVC)
set (CMAKE_EXE_LINKER_FLAGS
    "${CMAKE_EXE_LINKER_FLAGS} /subsystem:windows")
endif ()

# ----- LIBRARIES -----
set (LIBRARIES fmodex)
set (GL_LIBRARIES glew32 opengl32 glu32)
set (DX_LIBRARIES d3dx11 dxgi)
set (WINDOWS_LIBRARIES xinput)

if (OPENGL)
list (APPEND LIBRARIES ${GL_LIBRARIES})
endif ()
if (DIRECTX)
list (APPEND LIBRARIES ${DX_LIBRARIES})
endif ()

if (WIN32)
list (APPEND LIBRARIES ${WINDOWS_LIBRARIES})
endif ()

target_link_libraries (Meteor ${LIBRARIES})