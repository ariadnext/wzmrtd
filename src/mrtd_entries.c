/*
 * wzMRTD - An electronic passport reader library
 * Copyright (c) 2007, Johann Dantant - www.wzpass.net
 *
 * Please read LICENSE.txt for license and copyright info.
 */

#include "wzmrtd_i.h"

/*
 * MrtdCardConnect
 * ---------------
 */
WZMRTD_LIB BOOL WZMRTD_LINK MrtdCardConnect(MRTD_CTX_ST * ctx, const char *reader_name, const char *reader_option)
{
  if (ctx == NULL) return FALSE;

  MrtdStatus("Connecting to card");

  if (reader_name == NULL)
  {
    MrtdSetLastError(ctx, MRTD_E_INVALID_PARAM);
    return FALSE;
  }

#ifndef DISABLE_SPROX
  if (!strnicmp(reader_name, "SpringProx", 10)
   || !stricmp (reader_name, "SPX")
   || !stricmp (reader_name, "CSB"))
  {
    /* SpringProx reader */
    MrtdVerbose("SpringProx or CSB reader");
    return SPROX_IccConnect(ctx, reader_option);
  } else
#endif
  {
    /* PC/SC reader */
    MrtdVerbose("PC/SC reader '%s'", reader_name);
    return PCSC_IccConnect (ctx, reader_name);
  }
}

/*
 * MrtdCardDisconnect
 * ------------------
 */
WZMRTD_LIB BOOL WZMRTD_LINK MrtdCardDisconnect(MRTD_CTX_ST * ctx)
{
  if (ctx == NULL) return FALSE;

  MrtdStatus("Disconnecting...");

  if (ctx->pcsc_reader)
    return PCSC_IccDisconnect(ctx);

#ifndef DISABLE_SPROX
  if (ctx->sprox_reader)
    return SPROX_IccDisconnect(ctx);
#endif

  MrtdSetLastError(ctx, MRTD_E_READER_NOT_AVAIL);
  return FALSE;
}

/*
 * MrtdReadPassport
 * ----------------
 */
WZMRTD_LIB BOOL WZMRTD_LINK MrtdReadPassport(MRTD_CTX_ST * ctx, const char *mrz_string)
{
  return MrtdReadPassportEx(ctx, 0xFFFFFFFF, mrz_string);
}

/*
 * MrtdAllocCtx
 * ------------
 */
WZMRTD_LIB MRTD_CTX_ST * WZMRTD_LINK MrtdAllocCtx(void) 
{
  return calloc(1, sizeof(MRTD_CTX_ST));
}

/*
 * MrtdFreeCtx
 * -----------
 */
WZMRTD_LIB void WZMRTD_LINK MrtdFreeCtx(MRTD_CTX_ST *ctx) 
{
  BYTE i;  
  if (ctx == NULL)
    return;
  for (i = 0; i <= MRTD_DG_COUNT; i++)    
  {
    if (ctx->DgData[i] != NULL)     
     free(ctx->DgData[i]);
  }  
  free(ctx);
}

/*
 * EnumReadersW
 * ------------
 * Builds the list of all available readers, using the callback
 */
WZMRTD_LIB DWORD WZMRTD_LINK MrtdEnumReaders(EnumReadersProc Callback)
{
  DWORD c = 0;

#ifdef WIN32
  if (LoadLibrary("winscard.dll") != NULL)
#endif
  {
    /* Step 1 : PC/SC readers */
    /* ---------------------- */

    SCARDCONTEXT hc;
    LONG rc;
    DWORD sz;
    char *p;
    char *ls = NULL;

    /* Init the PC/SC stuff */
    rc = SCardEstablishContext (SCARD_SCOPE_USER, NULL, NULL, &hc);
    if (rc == SCARD_S_SUCCESS)
    {
      /* Retrieve list of readers */
      sz = SCARD_AUTOALLOCATE;

      rc = SCardListReaders (hc, NULL, (char *) &ls, &sz);
      if ((rc == SCARD_S_SUCCESS) && (ls != NULL))
      {
        /* Loop inside the list */
        p = ls;
        while (strlen(p))
        {
          /* New reader found */
          if (Callback != NULL)
            Callback(p, c);
          c++;

          /* Jump to next reader */
          p += strlen(p) + 1;
        }
      }

      if (ls != NULL)
        SCardFreeMemory (hc, ls);
    }

    /* De-init */
    SCardReleaseContext(hc);
  }

#ifndef DISABLE_SPROX
  if (LoadLibrary("springprox.dll") != NULL)
  {
    /* Step 2 : SpringProx reader */
    /* -------------------------- */

    if (Callback != NULL)
      Callback("SpringProx or CSB contactless reader", c);
    c++;
  }
#endif

  return c;
}



WZMRTD_LIB const char * WZMRTD_LINK MrtdVersion(void)
{
  return WZMRTD_VERSION;
}

WZMRTD_LIB BOOL WZMRTD_LINK MrtdGetDataGroup(MRTD_CTX_ST * mrtd_ctx, DWORD dgIndex, BYTE **out_dg, DWORD *dgLen)
{
    if (dgIndex > MRTD_DG_COUNT || !dgLen) {
        MrtdSetLastError(mrtd_ctx, MRTD_E_INVALID_PARAM);
        return FALSE;
    }
    else {
        *out_dg = mrtd_ctx->DgData[dgIndex];
        if (*out_dg) {
            *dgLen = mrtd_ctx->DgSize[dgIndex];
        }
        else {
            *dgLen = 0;
        }
        return TRUE;
    }
}

