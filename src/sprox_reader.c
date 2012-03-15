/*
 * wzMRTD - An electronic passport reader library
 * Copyright (c) 2007, Johann Dantant - www.wzpass.net
 *
 * Please read LICENSE.txt for license and copyright info.
 */

#include "wzmrtd_i.h"
#ifndef DISABLE_SPROX

#pragma comment(linker, "/ignore:4199")    // no imports found
#pragma message("automatic link to springprox.lib")
#pragma comment(lib, "../../lib/win32_i386/springprox.lib")
#pragma message("springprox.dll will be loaded just in time")
#pragma comment(linker, "/delayload:springprox.dll")

#include <assert.h>

static BOOL SPROX_IccConnectA(MRTD_CTX_ST * ctx);
static BOOL SPROX_IccConnectB(MRTD_CTX_ST * ctx);



BOOL SPROX_IccConnect(MRTD_CTX_ST * ctx, const char *reader_name)
{
  if (SPROX_IccConnectEx(ctx, reader_name, 0x00, 2, 0, 0)) // TODO : allow 424kbit/s
    return TRUE;

  if (SPROX_IccConnectEx(ctx, reader_name, 0x00, 1, 0, 0)) // TODO : allow 424kbit/s
    return TRUE;

  return FALSE;
}

static void TA1DsiDri(BYTE ta1, BYTE * dsi, BYTE * dri)
{
  assert(dsi != NULL);
  assert(dri != NULL);

  if ((ta1 & 0x40) && (*dsi >= 3))
  {
    *dsi = 3;
  } else
  if ((ta1 & 0x20) && (*dsi >= 2))
  {
    *dsi = 2;
  } else
  if ((ta1 & 0x10) && (*dsi >= 1))
  {
    *dsi = 3;
  } else
    *dsi = 0;

  if ((ta1 & 0x04) && (*dri >= 3))
  {
    *dri = 3;
  } else
  if ((ta1 & 0x02) && (*dri >= 2))
  {
    *dri = 2;
  } else
  if ((ta1 & 0x01) && (*dsi >= 1))
  {
    *dri = 3;
  } else
    *dri = 0;

  if (ta1 & 0x80)
  {
    if (*dsi > *dri)
      *dsi = *dri;
    if (*dri > *dsi)
      *dri = *dsi;
  }
}

BOOL SPROX_IccConnectEx(MRTD_CTX_ST * ctx, const char *reader_name, BYTE cid, BYTE type, BYTE max_dsi, BYTE max_dri)
{
  SWORD rc;

  if (ctx == NULL)
    return FALSE;

  ctx->pcsc_reader = FALSE;

  if (!ctx->sprox_reader)
  {
    if (LoadLibrary("springprox.dll") == NULL)
    {
      MrtdSetLastError(ctx, MRTD_E_READER_NOT_AVAIL);
      return FALSE;
    }

    rc = SPROX_ReaderOpen(reader_name);
    if (rc != MI_OK)
    {
      MrtdSetLastError(ctx, MRTD_E_READER_NOT_FOUND);
      return FALSE;
    }
    ctx->sprox_reader = TRUE;
    MrtdVerbose("Connected to SpringProx reader");
  }

  ctx->reader.sprox.cid = cid;
  ctx->reader.sprox.dsi = max_dsi;
  ctx->reader.sprox.dri = max_dri;

  if (type == 1)
    return SPROX_IccConnectA(ctx);
  if (type == 2)
    return SPROX_IccConnectB(ctx);

  return FALSE;
}

BOOL SPROX_IccDisconnect(MRTD_CTX_ST * ctx)
{
  SWORD rc;

  if (ctx == NULL)
    return FALSE;

  if (ctx->reader.sprox.type)
  {
    rc = SPROX_Tcl_Deselect(ctx->reader.sprox.cid);
    ctx->reader.sprox.type = 0;
  }

  if (ctx->sprox_reader)
  {
    SPROX_ReaderClose();
    ctx->sprox_reader = FALSE;
  }

  return TRUE;
}


BOOL SPROX_IccTransmit(MRTD_CTX_ST * ctx, const BYTE send[], WORD send_sz, BYTE recv[], WORD * recv_sz)
{
  SWORD rc;

  if (ctx == NULL)
    return FALSE;

  if (!ctx->sprox_reader)
    return FALSE;

  if (!ctx->reader.sprox.type)
    return FALSE;

  rc = SPROX_Tcl_Exchange(ctx->reader.sprox.cid, send, send_sz, recv, recv_sz);
  if (rc != MI_OK)
  {
    MrtdVerbose("SpringProx error %d", rc);
    if (rc <= -240)
    {
      MrtdSetLastError(ctx, MRTD_E_READER_COMM);
    } else
    if (rc == TCL_TRANSMERR_NOTAG)
    {
      MrtdSetLastError(ctx, MRTD_E_CARD_LOST);
    } else
    {
      MrtdSetLastError(ctx, MRTD_E_CARD_COMM);
    }
    return FALSE;
  }

  return TRUE;
}

static BOOL SPROX_IccConnectA(MRTD_CTX_ST * ctx)
{
  SWORD rc;

  assert(ctx != NULL);

  ctx->reader.sprox.card.a.uidlen = sizeof(ctx->reader.sprox.card.a.uid);

  rc = SPROX_TclA_ActivateAny(ctx->reader.sprox.card.a.atq, ctx->reader.sprox.card.a.uid, &ctx->reader.sprox.card.a.uidlen, ctx->reader.sprox.card.a.sak);
  if (rc != MI_OK)
  {
    MrtdVerbose("SpringProx error %d in ActivateA", rc);
    MrtdSetLastError(ctx, MRTD_E_NO_CARD);
    return FALSE;
  }

  MrtdVerbose("SpringProx : 14443-A card found");

  ctx->reader.sprox.card.a.atslen = sizeof(ctx->reader.sprox.card.a.ats);
  rc = SPROX_TclA_GetAts(ctx->reader.sprox.cid, ctx->reader.sprox.card.a.ats, &ctx->reader.sprox.card.a.atslen);

  if (rc != MI_OK)
  {
    MrtdVerbose("SpringProx error %d in ATS", rc);
    MrtdSetLastError(ctx, MRTD_E_CARD_LOST);
    return FALSE;
  }

  if (ctx->reader.sprox.card.a.ats[0] & 0x10)
  {
    TA1DsiDri(ctx->reader.sprox.card.a.ats[1], &ctx->reader.sprox.dsi, &ctx->reader.sprox.dri);
    if (ctx->reader.sprox.dsi || ctx->reader.sprox.dri)
    {
      rc = SPROX_TclA_Pps(ctx->reader.sprox.cid, ctx->reader.sprox.dsi, ctx->reader.sprox.dri);
      if (rc != MI_OK)
      {
        MrtdVerbose("SpringProx error %d in PPS", rc);
        MrtdSetLastError(ctx, MRTD_E_CARD_LOST);
        return FALSE;
      }
    }
  } else
  {
    ctx->reader.sprox.dsi = 0;
    ctx->reader.sprox.dri = 0;
  }

  MrtdVerbose("SpringProx : 14443-A card activated");
  ctx->reader.sprox.type = 1;
  return TRUE;
}

static BOOL SPROX_IccConnectB(MRTD_CTX_ST * ctx)
{
  SWORD rc;

  assert(ctx != NULL);

  rc = SPROX_TclB_ActivateAny(0x00, ctx->reader.sprox.card.b.atq);
  if (rc != MI_OK)
  {
    MrtdVerbose("SpringProx error %d in ActivateB", rc);
    MrtdSetLastError(ctx, MRTD_E_NO_CARD);
    return FALSE;
  }

  MrtdVerbose("SpringProx : 14443-B card found");

  TA1DsiDri(ctx->reader.sprox.card.b.atq[8], &ctx->reader.sprox.dsi, &ctx->reader.sprox.dri);

  rc = SPROX_TclB_AttribEx(ctx->reader.sprox.card.b.atq, ctx->reader.sprox.cid, ctx->reader.sprox.dsi, ctx->reader.sprox.dri);

  if (rc != MI_OK)
  {
    MrtdVerbose("SpringProx error %d in ATTRIB", rc);
    MrtdSetLastError(ctx, MRTD_E_CARD_LOST);
    return FALSE;
  }

  MrtdVerbose("SpringProx : 14443-B card activated");
  ctx->reader.sprox.type = 2;
  return TRUE;
}

#endif
