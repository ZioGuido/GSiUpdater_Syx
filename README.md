# GSiUpdater_Syx
Utility to send sysex dumps to Midi machines

This is the <SysEx Only> version of the GSi Updater utility originally written in 2013 for the GSi Burn.
The compiled version can be downloaded from this link:
  https://www.genuinesoundware.com/?a=dl&b=206&r=VHSOIDPSKGESUAGO
  

*************************************************
* GSi Updater - by Guido Scognamiglio - 2013    *
* www.GenuineSoundware.com - zioguido@gmail.com *
*                                               *
* Written in C++ using wxWidgets and RtMIDI for *
* multiplatform compatibility (Win / OSX)       *
*************************************************

- How to compile:

1. Download wxWidgets 2.8.x from here: http://www.wxwidgets.org/downloads/
   For Windows, it's recommended to download the package "wxMSW".
   For OSX, you have to download the package "wxMac".
   
2. Download RtMidi from here: http://www.music.mcgill.ca/~gary/rtmidi/
   and copy the files in the same folder of the project 
   
3. To make the banner image, you can use wxFormBuilder, download it here:
   http://sourceforge.net/projects/wxformbuilder/


- Compiling in Windows

You need Microsoft Visual Studio C++ (I have used the Express 2008 edition).

Compile the libraries by following the instructions you see here: 
http://wiki.wxwidgets.org/Compiling_WxWidgets_with_MSVC_(2)

Note: with MSVC Express 2008 or newer you don't need to install the Platform SDK any more.

It's important to set the environment variable %WXWIN% to point to the correct path
where the wxWidgets root is located. This variable will be automatically set if you
use the EXE installer instead of the manual install.


- Compiling in OSX

Xcode 3.0 is preferred, newer version should equally work.

Compile the libraries by following the instructions found in the file "install-mac.txt".

