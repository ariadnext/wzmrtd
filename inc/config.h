#ifndef __CONFIG_H__
#define __CONFIG_H__

#ifdef WIN32
  #include <windows.h>
  #include <direct.h>

  #define LITTLE_ENDIAN /* should be defined if so */

  #ifndef WZMRTD_LIB
    /* We are to link to the DLL */
    #define WZMRTD_LIB __declspec( dllimport )
  #endif

  /* We use the stdcall convention since it is easier for .NET callbacks */
  #define WZMRTD_LINK __stdcall
#else
  #ifndef TRUE
    #define TRUE 1
  #endif
  
  #ifndef FALSE
    #define FALSE 0
  #endif

  #define MAX_PATH PATH_MAX

  #define WZMRTD_LIB

  #define WZMRTD_LINK
#endif

#ifndef DISABLE_PCSC
  #include <winscard.h>
#endif
#ifndef DISABLE_SPROX
  #include <springprox.h>
#endif

#include <stdio.h>
#include <assert.h>
#include <time.h>
#include <limits.h>

#endif

