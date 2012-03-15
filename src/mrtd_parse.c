/*
 * wzMRTD - An electronic passport reader library
 * Copyright (c) 2007, Johann Dantant - www.wzpass.net
 *
 * Please read LICENSE.txt for license and copyright info.
 */

#include "wzmrtd_i.h"

/*
 * MrtdParseEFCOM
 * --------------
 */
BOOL MrtdParseEFCOM(BYTE data[], WORD size, DWORD * dglist)
{
  WORD offset = 0;
  WORD tag, i;
  WORD length = 0;
  BYTE * value;
  BYTE * root;

  if (data == NULL)
    return FALSE;
  if (dglist == NULL)
    return FALSE;

  /* Check root TLV */
  if (!TLVLoop(data, &offset, &tag, &length, &value))
  {
    MrtdVerbose("MrtdParseEFCOM : TLV parse error");
    return FALSE;
  }

  if (offset > size)
  {
    MrtdVerbose("MrtdParseEFCOM : specified length is invalid");
    return FALSE;
  }

  if (tag != 0x60)
  {
    MrtdVerbose("MrtdParseEFCOM : wrong identifier");
    return FALSE;
  }

  /* Find TLVs under the root */
  root = value;
  size = length;
  offset = 0;

  while ((offset < size) && TLVLoop(root, &offset, &tag, &length, &value))
  {
    if (tag != LDS_TAG_TAG_LIST)
      continue;

    /* This the list of DG */
    *dglist = 0;
    for (i = 0; i < length; i++)
    {
      switch (value[i])
      {
        case LDS_TAG_DG1:
          *dglist |= 0x00000002; /* DG1 */
          break;
        case LDS_TAG_DG2:
          *dglist |= 0x00000004; /* DG2 */
          break;
        case LDS_TAG_DG3:
          *dglist |= 0x00000008; /* DG3 */
          break;
        case LDS_TAG_DG4:
          *dglist |= 0x00000010; /* DG4 */
          break;
        case LDS_TAG_DG5:
          *dglist |= 0x00000020; /* DG5 */
          break;
        case LDS_TAG_DG6:
          *dglist |= 0x00000040; /* DG6 */
          break;
        case LDS_TAG_DG7:
          *dglist |= 0x00000080; /* DG7 */
          break;
        case LDS_TAG_DG8:
          *dglist |= 0x00000100; /* DG8 */
          break;
        case LDS_TAG_DG9:
          *dglist |= 0x00000200; /* DG9 */
          break;
        case LDS_TAG_DG10:
          *dglist |= 0x00000400; /* DG10 */
          break;
        case LDS_TAG_DG11:
          *dglist |= 0x00000800; /* DG11 */
          break;
        case LDS_TAG_DG12:
          *dglist |= 0x00001000; /* DG12 */
          break;
        case LDS_TAG_DG13:
          *dglist |= 0x00002000; /* DG13 */
          break;
        case LDS_TAG_DG14:
          *dglist |= 0x00004000; /* DG14 */
          break;
        case LDS_TAG_DG15:
          *dglist |= 0x00008000; /* DG15 */
          break;
        case LDS_TAG_DG16:
          *dglist |= 0x00010000; /* DG16 */
          break;
        case LDS_TAG_SOD:
          *dglist |= 0x00020000; /* EF.SOD */
          break;
      }
    }
  }

  return TRUE;
}
