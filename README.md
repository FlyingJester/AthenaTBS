Athena
======

A Turn-Based Strategy Game
--------------------------

[![Build Status](https://semaphoreci.com/api/v1/projects/f229b496-f9d5-4fb3-9f9f-ab03b0cd2eb8/566636/badge.svg)](https://semaphoreci.com/flyingjester/athenatbs)

Athena is a turn-based strategy game. It is written in ANSI C, and is known to compile with a variety of compilers.

Building Athena
---------------

You will need the folowwing:

 * An ANSI C compiler. See "Known Working Compilers"
 * Python 2.6 or 2.7 (Python 3 will not work)
 * SCons
 * libogg 1.3.0 (or greater)
 * libopus 1.1 (or greater)
 * libpng 1.6 (or greater)

You should be able to build by running the following:

`scons`

Known Working Compilers
-----------------------

Athena has been compiled successfully with the following compilers:

 * GCC 4.8
 * GCC 5.2
 * Cygwin
 * Clang 3.6
 * Open Watcom (final Sybase release)
 * Solaris Studio 11.3
 * Microsoft Visual Studio 2010

Note that not every commit or even every release is tested with these compilers. Each commit is, however, at least tested to not break the build using GCC 4.8.

The Windows build has been optimized for MinGW or Cygwin, and has additionally been tested with Open Watcom.

Platform-Specific Information
-----------------------------

****OS X****

There is no app bundle yet. You must run the athena binary from the terminal, and you must run it from the proper directory. Or alternatively, you could help create an app bundle for Athena :)

The Cocoa backend is known to have serious issues with graphics. Please use the SDL2 backend. Any patches to improve the Cocoa backend are welcome.

****Linux****

The X11 backend has no graphics support yet. Please use the SDL2 backend. Any patches to improve the X11 backend are welcome.

****OpenBSD****

I am very interested in creating a sndio audio backend. Any patches to add or improve a sndio audio backend are very welcome!

****Haiku****

Haiku has a dual-toolchain setup. It has both GCC 2 and GCC 4. Although it is certainly possible that Athena will compile using GCC 2, it is not recommended (you are free to try it).

To specify to build using GCC 4 on haiku, you must prepend "setarch x86" to scons, as so:

`setarch x86 scons`

There is a specific Haiku backend for video and audio, but it is known to have a few issues. On certain machines, the window is detached from the actual video output. It seems to depend on the graphics card. There are a few variables in src/platform/window/SConscript that can force Athena to be built using SDL2. This is also known to have some issues with flickering, but input works better.

****Windows****

It is known that the Creative Labs OpenAL implementation has very bad crackling in the audio with Athena. Please use OpenAL-Soft. Or alternatively, you could help with the creation of a Windows audio backend, or improving the existing OpenAL backend :)
