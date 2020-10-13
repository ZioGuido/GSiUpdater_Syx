///////////////////////////////////////////////////////////////////////////
// GSi Updater - Version 1.0 - July 2013
// A utility by Guido Scognamiglio used to update GSi and Crumar Firmwares
// Uses wxWidgets and RtMidi for cross platform (Win and Mac) compatibility
//
// www.GenuineSoundware.com
// 
///////////////////////////////////////////////////////////////////////////

#pragma once

#include <stdio.h>
#if WIN32
#include "windows.h"
#include <tchar.h>
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#endif

/*********** for fstream (ifstream, ofstream) ********/
#include <iostream>
#include <sstream>  // Required for stringstreams
#include <fstream>
using namespace std;
