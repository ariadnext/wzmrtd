#ifndef __WZMRTD_I_H__
#define __WZMRTD_I_H__

#ifdef WZMRTD_DLL_EXPORTS
  /* We are building wzmrtd.dll, let's export our public methods */
  #define WZMRTD_LIB __declspec(dllexport)
#else
  /* Must be a self-standing exe or a static library, no need to export */
  #define WZMRTD_LIB
#endif

/* In /inc directory */ 
#include "config.h" 
#include "ldstags.h"
#include "wzmrtd.h" 

/* In /src directory */ 
#include "tlv.h"
#include "des.h"
#include "sha1.h"
#include "random.h"  

/* C standard library */ 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>

#ifndef WIN32
// for mkdir
#include <sys/stat.h>
#endif

/* From main.c */ 
/* ----------- */ 
#define MRZ_INFO_SIZE 24 

struct _MRTD_CTX_ST
{
  LONG LastError;

  BOOL pcsc_reader;
  BOOL sprox_reader;
  
  union 
  {
    struct 
    {
      SCARDCONTEXT hcontext;
      SCARDHANDLE hcard;
      BYTE atr[32];
      BYTE atrlen;
      DWORD protocol;
    } pcsc;    
    struct 
    {
      BYTE type;
      BYTE cid;
      BYTE dsi;
      BYTE dri;
      union 
      {
        struct 
        {
          BYTE atq[2];
          BYTE uid[12];
          BYTE sak[1];
          BYTE ats[32];
          BYTE uidlen;
          BYTE atslen;
        } a;
        struct 
        {
          BYTE atq[11];
        } b;
       } card;
     } sprox;
   } reader;
   
  /* Storage and size of each DG (DG1 to DG16) ; EF.COM is stored as DG0 */ 
  BYTE * DgData[MRTD_DG_COUNT + 1];  
  WORD DgSize[MRTD_DG_COUNT + 1];

  /* Basic authentication context */  
  struct 
  {
    BOOL enabled;
    BYTE ssc[8];
    TDES_CTX_ST enc_ctx;
    DES_CTX_ST mac_ctx[2];
  } Bac;

  struct
  {
    BOOL provided;
    BYTE content[MRZ_INFO_SIZE];
    BOOL checksumsok;
  } Mrz;

};

/* From mrtd_errors.c */ 
/* ----------------- */ 

void MrtdSetLastError(MRTD_CTX_ST * ctx, LONG code);

/* From mrtd_core.c */ 
/* ---------------- */ 
  
BOOL MrtdReadFileEx(MRTD_CTX_ST * ctx, BYTE data[], WORD offset, WORD * length);
BOOL MrtdReadFileSize(MRTD_CTX_ST * ctx, WORD * filesize);
BOOL MrtdReadFile(MRTD_CTX_ST * ctx, BYTE filedata[], WORD filesize);
BOOL MrtdSelectDG(MRTD_CTX_ST * ctx, BYTE dg);
BOOL MrtdSelectApplet(MRTD_CTX_ST * ctx);
BOOL MrtdReadPassportEx(MRTD_CTX_ST * ctx, DWORD want_dgs, const char *mrz_string);

/* From mrtd_bac.c */ 
/* --------------- */ 
  
BOOL MrtdBacInitialize(MRTD_CTX_ST * ctx);
BOOL MrtdBacReadFileEx(MRTD_CTX_ST * ctx, BYTE data[], WORD offset, WORD * length);
BOOL MrtdBacSelectDG(MRTD_CTX_ST * ctx, BYTE dg);

/* From mrtd_mrz.c */ 
/* --------------- */ 
 
BOOL MrtdAssignMrz(MRTD_CTX_ST * mrtd_ctx, const char *mrz_string);

/* From mrtd_icc.c */ 
/* --------------- */ 
  
BOOL MrtdIccTransmit(MRTD_CTX_ST * ctx, const BYTE send[], WORD send_len, BYTE recv[], WORD * recv_len);
BOOL MrtdIccGetChallenge(MRTD_CTX_ST * ctx, BYTE rnd_icc[8]);
BOOL MrtdIccMutualAuthenticate(MRTD_CTX_ST * ctx, BYTE send_buffer[40], BYTE recv_buffer[40]);
WORD MrtdIccStatusWord(MRTD_CTX_ST * ctx, BYTE recv_buffer[], WORD recv_len);

/* From mrtd_parse.c */ 
/* ----------------- */ 
BOOL MrtdParseEFCOM(BYTE data[], WORD size, DWORD * dglist);

/* From mrtd_out_xml.c */ 
/* ------------------- */ 
LONG MrtdDG1ToTxt(FILE * fp, BYTE data[], WORD size);
LONG MrtdDG1ToXml(FILE * fp, BYTE data[], WORD size);

/* From mrtd_out_raw.c */ 
/* ------------------- */ 

LONG MrtdWriteFile(const char *file_name, const BYTE data[], WORD size, BOOL overwrite);

/* From sprox_reader.c */ 
/* ------------------- */ 
#ifndef DISABLE_SPROX
BOOL SPROX_IccConnect(MRTD_CTX_ST * ctx, const char *reader_name);
BOOL SPROX_IccConnectEx(MRTD_CTX_ST * ctx, const char *reader_name, BYTE cid, BYTE type, BYTE max_dsi, BYTE max_dri);
BOOL SPROX_IccDisconnect(MRTD_CTX_ST * ctx);
BOOL SPROX_IccTransmit(MRTD_CTX_ST * ctx, const BYTE send[], WORD send_sz, BYTE recv[], WORD * recv_sz);
#endif

/* From pcsc_reader.c */ 
/* ------------------ */ 
BOOL PCSC_IccConnect(MRTD_CTX_ST * ctx, const char *reader_name);
BOOL PCSC_IccDisconnect(MRTD_CTX_ST * ctx);
BOOL PCSC_IccTransmit(MRTD_CTX_ST * ctx, const BYTE send[], WORD send_sz, BYTE recv[], WORD * recv_sz);

/* From mrtd_errors.c */
/* ------------------ */

BOOL MrtdProgress(DWORD pos, DWORD max);
BOOL MrtdStatus(const char *fmt, ...);
BOOL MrtdVerbose(const char *fmt, ...);

#define VRB_ALWAYS  0
#define VRB_ERROR   1
#define VRB_WARNING 2
#define VRB_TRACE   3
#define VRB_DEBUG   4
  
#endif

