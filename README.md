METEOR
======

This is a simple cross-platform game framework developed in c++ by [Vavassor](http://vavassor.tumblr.com)

It is built to use either OpenGL or DirectX for rendering, and FMOD Ex for sound. Also uses XInput on windows for gamepads, and libudev/evdev on Linux. It has been tested on Windows under Visual Studio and MinGW-w64 and Linux Mint under gcc.

It will be subject to change as I intend to continue to develop and fork games off from it.

// Making/Building
--------------

CMake can be used to build the project using the CMakeLists.txt file in the root directory. Note that options are only provided for the compilers tested above.

If you wish to build manually, the requirements are basically as follows:

-   exclude source subdirectories that conflict with the platform
    compiled under, like the directx /dx directory should be excluded
    under linux builds because DirectX is a microsoft-owned windows-only
    thing.
-   compile with _UNICODE and UNICODE flags and use the flags 
    GRAPHICS_OPENGL or GRAPHICS_DIRECTX for compiling the code related
    to those api's.
-   Dependencies, headers and static libraries (.lib or .a) needed for all:
      * DirectX SDK for any windows build that wishes to render using it
      * FMOD Ex for Sound.h and Sound.cpp
      * XInput for controller input on Windows
	  * libudev for controller connection notifications in Linux
-   also the .dll or .so dynamically-linked library for FMOD Ex is needed
    either in the directory of the .exe or listed in the run configuration

// Licensing
--------------

All code dependent on specific libraries is subject to their licensing restrictions, which are located in Content/docs/. All OTHER code is released under _public domain_, specifically meaning I place NO restrictions on its use at all, whatsoever. Take it and do a thing with it. It will be cool I promise. No need to credit me
