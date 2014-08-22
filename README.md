METEOR
======

This is a simple cross-platform game framework developed in c++ 
by Vavassor (http://vavassor.tumblr.com)

It is built to use either OpenGL or DirectX for rendering, 
and FMOD Ex for sound. Also uses XInput on windows for gamepads,
but no gamepad support on linux, yet. It has been tested on Windows
under Visual Studio and MinGW-w64 and Linux Mint under gcc.

It will be subject to change as I intend to continue to
develop and fork games off from it.

// Making/Building
--------------

Make instructions for specific compilers I currently can't
provide, but generally as follows:

-   the project directory should be in the include path.
-   exclude source subdirectories that conflict with the platform
    compiled under, like the directx /dx directory should be excluded
    under linux builds because DirectX is a microsoft-owned windows-only
    thing.
-   compile with _UNICODE and UNICODE flags and use the flags 
    GRAPHICS_OPENGL or GRAPHICS_DIRECTX for compiling the code related
    to those api's.
-   Dependencies, headers and static libraries (.lib or .a) needed for all:
      * GLEW openGL Extension Wrangler ( http://glew.sourceforge.net/ )
      * DirectX SDK for any windows build that wishes to render using it
      * FMOD Ex for Sound.h and Sound.cpp
      * XInput for controller input on Windows
-   also the .dll or .so dynamically-linked library for FMOD Ex is needed
    either in the directory of the .exe or listed in the run configuration

// Licensing
--------------

All code dependent on specific libraries is subject to their licensing
restrictions, which are located in Content/docs/. All OTHER code is
released under _public domain_, specifically meaning I place NO restrictions
on its use at all, whatsoever. Take it and do a thing with it. It will
be cool I promise. No need to credit me
