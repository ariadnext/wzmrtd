/*
 * wzMRTD - An electronic passport reader library
 * Copyright (c) 2007, Johann Dantant - www.wzpass.net
 *
 * Please read LICENSE.txt for license and copyright info.
 */

#include "wzmrtd_i.h"

const char * skipLine(const char *mrz_string);
void addMrzFragment(MRTD_CTX_ST * mrtd_ctx, int offset, const char *fragment, int frag_length);

unsigned int ansiCharValue(char c)
{
    if ( isdigit(c) ) return c - '0';
    if ( isupper(c) ) return c - 'A' + 10;
    return 0;
}

BOOL MrtdAssignMrz(MRTD_CTX_ST * mrtd_ctx, const char *mrz_string)
{
  const char *p;

  if (mrtd_ctx == NULL) return FALSE;

  mrtd_ctx->Mrz.provided = FALSE;
  mrtd_ctx->Mrz.checksumsok = TRUE;

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

    addMrzFragment(mrtd_ctx, 0, &p[5], 10);

    /* Skip one line */
    p = skipLine(p);
    if (strlen(p) < 15)
    {
      MrtdSetLastError(mrtd_ctx, MRTD_E_BAD_MRZ);
      return FALSE;
    }
    addMrzFragment(mrtd_ctx, 10, &p[0], 7);
    addMrzFragment(mrtd_ctx, 17, &p[8], 7);
  }
  else {
    /* Assume 2 lines passport */
    p = skipLine(p);
    if (strlen(p) < 28)
    {
      MrtdSetLastError(mrtd_ctx, MRTD_E_BAD_MRZ);
      return FALSE;
    }

    addMrzFragment(mrtd_ctx, 0,  &p[0], 10);
    addMrzFragment(mrtd_ctx, 10, &p[13], 7);
    addMrzFragment(mrtd_ctx, 17, &p[21], 7);
  }

  mrtd_ctx->Mrz.provided = TRUE;

  return TRUE;
}

void addMrzFragment(MRTD_CTX_ST * mrtd_ctx, int offset, const char *fragment, int frag_length)
{
    unsigned int val = 0;
    int i = 0;

    /* recompute checksums */
    for ( i = 0; i < frag_length - 1; ++i ) {
        switch ( i % 3 ) {
        case 0:
            val += 7 * ansiCharValue(fragment[i]);
            break;
        case 1:
            val += 3 * ansiCharValue(fragment[i]);
            break;
        case 2:
            val += ansiCharValue(fragment[i]);
            break;
        }
    }

    memcpy(&mrtd_ctx->Mrz.content[offset], fragment, frag_length);

    mrtd_ctx->Mrz.checksumsok &= ( ( val % 10 ) == ansiCharValue(fragment[frag_length - 1]));
}

const char * skipLine(const char *p)
{
    if (strchr(p, '\r') != NULL) { p = strchr(p, '\r'); p++; }
    if (strchr(p, '\n') != NULL) { p = strchr(p, '\n'); p++; }
    return p;
}
