/*
 * wzMRTD - An electronic passport reader library
 * Copyright (c) 2007, Johann Dantant - www.wzpass.net
 *
 * Please read LICENSE.txt for license and copyright info.
 */

#include "wzmrtd_i.h"

WORD TLVTotalSize(BYTE buffer[])
{
  DWORD l, o = 0;
  if (buffer == NULL)
    return 0;
  if ((buffer[o] == 0x00) || (buffer[o] == 0xFF))
    return 0;

  /* Skip the tag */
  if ((buffer[o] & 0x1F) != 0x1F)
  {
    /* Short tag */
    o += 1;
  } else
  {
    /* Long tag */
    o += 2;
  }

  /* Read the length */
  if (buffer[o] & 0x80)
  {
    /* Multi-byte lenght */
    switch (buffer[o++] & 0x7F)
    {
      case 0x01:
        l = buffer[o++];
        break;
      case 0x02:
        l = buffer[o++];
        l <<= 8;
        l += buffer[o++];
        break;
      default:
        return FALSE;
    }
  } else
  {
    /* Length on a single byte */
    l = buffer[o++];
  }

  /* Add offset to compute total length */
  l += o;
  if (l > 65535)
    return 0;                  /* Overflow */
  return (WORD) l;
}

BOOL TLVLoop(BYTE buffer[], WORD * offset, WORD * tag, WORD * length, BYTE * value[])
{
  WORD t;
  DWORD o, l;

  if (buffer == NULL)
    return FALSE;

  if (offset != NULL)
    o = *offset;
  else
    o = 0;

  if ((buffer[o] == 0x00) || (buffer[o] == 0xFF))
    return FALSE;

  /* Read the tag */
  if ((buffer[o] & 0x1F) != 0x1F)
  {
    /* Short tag */
    t = buffer[o++];
  } else
  {
    /* Long tag */
    t = buffer[o++];
    t <<= 8;
    t |= buffer[o++];
  }

  if (tag != NULL)
    *tag = t;

  /* Read the length */
  if (buffer[o] & 0x80)
  {
    /* Multi-byte lenght */
    switch (buffer[o++] & 0x7F)
    {
      case 0x01:
        l = buffer[o++];
        break;
      case 0x02:
        l = buffer[o++];
        l <<= 8;
        l += buffer[o++];
        break;
      default:
        return FALSE;
    }
  } else
  {
    /* Length on a single byte */
    l = buffer[o++];
  }

  if (l > 65535)
    return FALSE;              /* Overflow */

  if (length != NULL)
    *length = (WORD) l;

  /* Get a pointer on data */
  if (value != NULL)
    *value = &buffer[o];

  /* Jump to the end of data */
  o += l;

  if (offset != NULL)
    *offset = (WORD) o;

  return TRUE;
}

DWORD TLVGetDWORD(BYTE buffer[], WORD length)
{
  DWORD rc = 0;
  WORD i;

  if (buffer == NULL)
    return rc;

  if ((length == 0) || (length > 4))
    return rc;

  for (i = 0; i < length; i++)
  {
    rc <<= 8;
    rc |= buffer[i];
  }

  return rc;
}



