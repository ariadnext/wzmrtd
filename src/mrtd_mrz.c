/*
 * wzMRTD - An electronic passport reader library
 * Copyright (c) 2007, Johann Dantant - www.wzpass.net
 *
 * Please read LICENSE.txt for license and copyright info.
 */

#include "wzmrtd_i.h"

const char * skipLine(const char *mrz_string);

BOOL MrtdAssignMrz(MRTD_CTX_ST * mrtd_ctx, const char *mrz_string)
{
  const char *p;

  if (mrtd_ctx == NULL) return FALSE;

  mrtd_ctx->Mrz.provided = FALSE;

  if (mrz_string == NULL)
  {
    MrtdSetLastError(mrtd_ctx, MRTD_E_INVALID_PARAM);
    return FALSE;
  }

  p = mrz_string;
  if ( mrz_string[0] == 'I' ) {
    /* Assume 3 lines ID document */
    if (strlen(p) < 15)
    {
      MrtdSetLastError(mrtd_ctx, MRTD_E_BAD_MRZ);
      return FALSE;
    }

    memcpy(&mrtd_ctx->Mrz.content[0], &p[5], 10);

    /* Skip one line */
    p = skipLine(p);
    if (strlen(p) < 15)
    {
      MrtdSetLastError(mrtd_ctx, MRTD_E_BAD_MRZ);
      return FALSE;
    }
    memcpy(&mrtd_ctx->Mrz.content[10], &p[0], 7);
    memcpy(&mrtd_ctx->Mrz.content[17], &p[8], 7);
  }
  else {
    /* Assume 2 lines passport */
    p = skipLine(p);
    if (strlen(p) < 28)
    {
      MrtdSetLastError(mrtd_ctx, MRTD_E_BAD_MRZ);
      return FALSE;
    }

    memcpy(&mrtd_ctx->Mrz.content[0],  &p[0], 10);
    memcpy(&mrtd_ctx->Mrz.content[10], &p[13], 7);
    memcpy(&mrtd_ctx->Mrz.content[17], &p[21], 7);
  }

  mrtd_ctx->Mrz.provided = TRUE;

  return TRUE;
}

const char * skipLine(const char *p)
{
    if (strchr(p, '\r') != NULL) { p = strchr(p, '\r'); p++; }
    if (strchr(p, '\n') != NULL) { p = strchr(p, '\n'); p++; }
    return p;
}
