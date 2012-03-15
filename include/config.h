#ifndef __CONFIG_H__
#define __CONFIG_H__

#ifdef WIN32
  #include <windows.h>
  #include <direct.h>
#else
  #define TRUE 1
  #define FALSE 0

  #define MAX_PATH PATH_MAX
#endif
#ifndef DISABLE_PCSC
  #include <winscard.h>
#endif
#ifndef DISABLE_SPROX
  #include <springprox.h>
#endif

#define LITTLE_ENDIAN /* should be defined if so */

#include <stdio.h>
#include <assert.h>
#include <time.h>
#include <limits.h>

#endif
