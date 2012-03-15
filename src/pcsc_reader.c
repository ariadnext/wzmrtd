/*
 * wzMRTD - An electronic passport reader library
 * Copyright (c) 2007, Johann Dantant - www.wzpass.net
 *
 * Please read LICENSE.txt for license and copyright info.
 */

#include "wzmrtd_i.h"
#ifndef DISABLE_PCSC

#pragma comment(linker, "/ignore:4199")    // no imports found
#pragma message("automatic link to winscard.lib")
#pragma comment(lib, "winscard.lib")
#pragma message("winscard.dll will be loaded just in time")
#pragma comment(linker, "/delayload:winscard.dll")

BOOL PCSC_IccConnect (MRTD_CTX_ST * ctx, const char *pcsc_reader)
{
  LONG rc;
  char *reader_name = NULL;
  char *reader_list = NULL;
  DWORD reader_list_sz;
  int reader_sel = -1;

  if (ctx == NULL)
    return FALSE;

  if (LoadLibrary("winscard.dll") == NULL)
  {
    MrtdSetLastError(ctx, MRTD_E_READER_NOT_AVAIL);
    return FALSE;
  }

  /* Try to understand the pcsc_reader name */
  if (pcsc_reader == NULL)
  {
    reader_sel = 0;
  } else
  if ((strlen (pcsc_reader) == 2) && (pcsc_reader[0] == '#'))
  {
    reader_sel = atoi (&pcsc_reader[1]);
  }

  /* Create the PC/SC context */
  if (ctx->reader.pcsc.hcard)
  {
    SCardDisconnect (ctx->reader.pcsc.hcard, SCARD_LEAVE_CARD);
    ctx->reader.pcsc.hcard = 0;
  }

  if (ctx->reader.pcsc.hcontext)
  {
    SCardReleaseContext (ctx->reader.pcsc.hcontext);
    ctx->reader.pcsc.hcontext = 0;
  }

  rc = SCardEstablishContext (SCARD_SCOPE_USER, NULL, NULL, &ctx->reader.pcsc.hcontext);
  if (rc != SCARD_S_SUCCESS)
  {
    MrtdVerbose("PC/SC error %08lX", rc);
    MrtdSetLastError (ctx, MRTD_E_READER_NOT_FOUND);
    return FALSE;
  }

  /* Retrieve list of readers */
  reader_list_sz = SCARD_AUTOALLOCATE;

  rc = SCardListReaders (ctx->reader.pcsc.hcontext, NULL, (char *) &reader_list, &reader_list_sz);
  if (rc != SCARD_S_SUCCESS)
  {
    MrtdVerbose("SCardListReaders : PC/SC error %08lX", rc);
    MrtdSetLastError (ctx, MRTD_E_READER_NOT_FOUND);
    return FALSE;
  }

  /* Loop inside the list */
  if (reader_list != NULL)
  {
    char *p = reader_list;

    while (strlen (p))
    {
      if (reader_sel < 0)
      {
        /* We must compare the names */
        if (!strcmp (p, pcsc_reader))
        {
          /* This one is OK ! */
          reader_name = p;
          break;
        }
      } else if (reader_sel == 0)
      {
        /* We shall take this one */
        reader_name = p;
        break;
      } else
      {
        /* Decrement... */
        reader_sel--;
      }
      p += strlen (p) + 1;
    }
  }

  /* A reader has been selected ? */
  if (reader_name == NULL)
  {
    SCardFreeMemory (ctx->reader.pcsc.hcontext, reader_list);
    MrtdSetLastError (ctx, MRTD_E_READER_NOT_FOUND);
    return FALSE;
  }

  MrtdVerbose("Connected to PC/SC reader \"%s\"", reader_name);

  /* Try to connect to the card in the reader */
  rc = SCardConnect (ctx->reader.pcsc.hcontext, reader_name, SCARD_SHARE_EXCLUSIVE, SCARD_PROTOCOL_T0 | SCARD_PROTOCOL_T1, &ctx->reader.pcsc.hcard, &ctx->reader.pcsc.protocol);
  SCardFreeMemory (ctx->reader.pcsc.hcontext, reader_list);

  if (rc != SCARD_S_SUCCESS)
  {
    MrtdVerbose("SCardConnect : PC/SC error %08lX", rc);
    MrtdSetLastError (ctx, MRTD_E_NO_CARD);
    return FALSE;
  }

  if (ctx->reader.pcsc.protocol == SCARD_PROTOCOL_T0)
  {
    MrtdVerbose("Connected to a T=0 card");
  } else
  if (ctx->reader.pcsc.protocol == SCARD_PROTOCOL_T1)
  {
    MrtdVerbose("Connected to a T=1 card");
  }

  /* We are connected to the card */
  ctx->pcsc_reader = TRUE;
  ctx->sprox_reader = FALSE;
  return TRUE;
}

BOOL PCSC_IccDisconnect (MRTD_CTX_ST * ctx)
{
  LONG rc;

  if (ctx == NULL)
    return FALSE;

  if (!ctx->pcsc_reader)
    return FALSE;

  if (ctx->reader.pcsc.hcard)
  {
    rc = SCardDisconnect (ctx->reader.pcsc.hcard, SCARD_EJECT_CARD);
    if (rc != SCARD_S_SUCCESS)
    {
      MrtdVerbose("SCardDisconnect : PC/SC error %08lX", rc);
    }
    ctx->reader.pcsc.hcard = 0;
  }

  if (ctx->reader.pcsc.hcontext)
  {
    SCardReleaseContext (ctx->reader.pcsc.hcontext);
    if (rc != SCARD_S_SUCCESS)
    {
      MrtdVerbose("SCardReleaseContext : PC/SC error %08lX", rc);
    }
    ctx->reader.pcsc.hcontext = 0;
  }

  ctx->pcsc_reader = FALSE;
  return TRUE;
}

BOOL PCSC_IccTransmit (MRTD_CTX_ST * ctx, const BYTE send[], WORD send_sz, BYTE recv[], WORD * recv_sz)
{
  LONG rc;
  DWORD recv_sz_dw = 0;
  const SCARD_IO_REQUEST *pioSendPci = NULL;

  if (ctx == NULL)
    return FALSE;

  if (recv_sz != NULL)
    recv_sz_dw = *recv_sz;

  if (ctx->reader.pcsc.protocol == SCARD_PROTOCOL_T0)
  {
    pioSendPci = SCARD_PCI_T0;
  } else
  if (ctx->reader.pcsc.protocol == SCARD_PROTOCOL_T1)
  {
    pioSendPci = SCARD_PCI_T1;
  }

  rc = SCardTransmit (ctx->reader.pcsc.hcard, NULL, send, send_sz, NULL, recv, &recv_sz_dw);
  if (recv_sz != NULL)
    *recv_sz = (WORD) recv_sz_dw;

  if (rc != SCARD_S_SUCCESS)
  {
    MrtdVerbose("SCardTransmit : PC/SC error %08lX", rc);
    MrtdSetLastError (ctx, MRTD_E_CARD_LOST);
    return FALSE;
  }

  return TRUE;
}

#endif
