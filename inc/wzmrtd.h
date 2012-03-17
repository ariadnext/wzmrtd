#ifndef __WZMRTD_H__
#define __WZMRTD_H__

#include "config.h"

#define WZMRTD_EXPORT

/* The MRTD context object is abstract here */
typedef struct _MRTD_CTX_ST MRTD_CTX_ST;

/* Here's our errors for MrtdGetLastError */
#define MRTD_SUCCESS                 0

#define MRTD_E_READER_NOT_AVAIL   1000
#define MRTD_E_READER_NOT_FOUND   1001
#define MRTD_E_READER_COMM        1002
#define MRTD_E_NO_CARD            1010
#define MRTD_E_CARD_LOST          1011
#define MRTD_E_CARD_COMM          1012
#define MRTD_E_CARD_ANSWER        1013
#define MRTD_E_CARD_STATUS        1014
#define MRTD_E_SECURITY_STATUS    1015

#define MRTD_E_NO_LDS             1020
#define MRTD_E_NO_FILE            1021

#define MRTD_E_BAD_MRZ            1022

#define MRTD_E_BAC_NEEDS_MRZ      1023
#define MRTD_E_BAC_FAILED         1024

#define MRTD_E_AUTH_INVALID       1025

#define MRTD_E_BAD_HEADER         1030
#define MRTD_E_BAD_LENGTH         1031
#define MRTD_E_BAD_TAG            1032
#define MRTD_E_BAD_CONTENT        1033

#define MRTD_E_FILE_EXISTS        1040
#define MRTD_E_FILE_OPEN_WRITE    1041
#define MRTD_E_FILE_OPEN_READ     1042
#define MRTD_E_FILE_WRITE         1043
#define MRTD_E_FILE_READ          1044

#define MRTD_E_INVALID_HANDLE     1100
#define MRTD_E_INVALID_PARAM      1101
#define MRTD_E_INTERNAL_ERROR     1102
#define MRTD_E_FUNCTION_NOT_AVAIL 1103
#define MRTD_E_OUT_OF_MEMORY      1104

#define MRTD_CANCELLED            1200

/* Max number of possible data groups */
#define MRTD_DG_COUNT 16

#ifdef __cplusplus
  extern "C" {
#endif

WZMRTD_LIB MRTD_CTX_ST * WZMRTD_LINK MrtdAllocCtx(void);
WZMRTD_LIB void WZMRTD_LINK MrtdFreeCtx(MRTD_CTX_ST * mrtd_ctx);

typedef void (WZMRTD_LINK EnumReadersProc) (const char *reader, DWORD index);
WZMRTD_LIB DWORD WZMRTD_LINK MrtdEnumReaders(EnumReadersProc Callback);

typedef BOOL (WZMRTD_LINK CallbackProc) (const char *info, const char *trace, DWORD pos, DWORD max);
WZMRTD_LIB void WZMRTD_LINK MrtdSetCallback(CallbackProc Callback);

WZMRTD_LIB BOOL WZMRTD_LINK MrtdCardConnect(MRTD_CTX_ST * ctx, const char *reader_name, const char *reader_option);
WZMRTD_LIB BOOL WZMRTD_LINK MrtdCardDisconnect(MRTD_CTX_ST * ctx);

WZMRTD_LIB BOOL WZMRTD_LINK MrtdReadPassport(MRTD_CTX_ST * ctx, const char *mrz_string);

WZMRTD_LIB BOOL WZMRTD_LINK MrtdSaveToXML(MRTD_CTX_ST * mrtd_ctx, const char *xml_file_name);
WZMRTD_LIB BOOL WZMRTD_LINK MrtdSaveToXMLEx(MRTD_CTX_ST * mrtd_ctx, const char *xml_file_name, BOOL sep_images, BOOL add_unproc_raw, BOOL add_full_raw, BOOL overwrite);
WZMRTD_LIB BOOL WZMRTD_LINK MrtdSaveToFiles(MRTD_CTX_ST * mrtd_ctx, const char *out_directory);
WZMRTD_LIB BOOL WZMRTD_LINK MrtdSaveToFilesEx(MRTD_CTX_ST * mrtd_ctx, const char *out_directory, const char *file_name_prefix, BOOL sep_text, BOOL sep_images, BOOL overwrite);
WZMRTD_LIB BOOL WZMRTD_LINK MrtdGetDataGroup(MRTD_CTX_ST * mrtd_ctx, DWORD dgIndex, BYTE **out_dg, DWORD *dgLen);

WZMRTD_LIB LONG WZMRTD_LINK MrtdGetLastError(MRTD_CTX_ST * mrtd_ctx);
WZMRTD_LIB const char * WZMRTD_LINK MrtdTranslateError(LONG code);

WZMRTD_LIB const char * WZMRTD_LINK MrtdVersion(void);

#ifdef __cplusplus
  }
#endif

#endif
