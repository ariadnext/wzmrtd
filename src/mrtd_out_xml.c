/*
 * wzMRTD - An electronic passport reader library
 * Copyright (c) 2007, Johann Dantant - www.wzpass.net
 *
 * Please read LICENSE.txt for license and copyright info.
 */

#include "wzmrtd_i.h"

LONG MrtdBioCbeffFacToXml(FILE * fp, BYTE data[], WORD size, const char *images_file_prefix, BOOL overwrite);
LONG MrtdBioCbeffFmrToXml(FILE * fp, BYTE data[], WORD size, const char *images_file_prefix, BOOL overwrite);
LONG MrtdBioCbeffFirToXml(FILE * fp, BYTE data[], WORD size, const char *images_file_prefix, BOOL overwrite);

void fprint_base64(FILE * fp, const BYTE * data, WORD size, WORD wrap);
const char *basename_p(const char *filename);

static LONG MrtdEFCOMToXml(FILE * fp, BYTE data[], WORD size)
{
  WORD offset = 0;
  WORD tag, i;
  WORD length = 0;
  BYTE * value;
  BYTE * root;

  assert (fp != NULL);
  assert (data != NULL);

  /* Check root TLV */
  if (!TLVLoop(data, &offset, &tag, &length, &value))
  {
    MrtdVerbose("MrtdParseEFCOM : TLV parse error");
    return MRTD_E_BAD_HEADER;
  }

  if (offset > size)
  {
    MrtdVerbose("MrtdParseEFCOM : specified length is invalid");
    return MRTD_E_BAD_HEADER;
  }

  if (tag != 0x60)
  {
    MrtdVerbose("MrtdParseEFCOM : wrong identifier");
    return MRTD_E_BAD_TAG;
  }

  /* Find TLVs under the root */
  root = value;
  size = length;
  offset = 0;

  while ((offset < size) && TLVLoop(root, &offset, &tag, &length, &value))
  {
    switch (tag)
    {
      case LDS_TAG_LDS_VERSION:
        fprintf(fp, "\t\t<lds_version>");
        fwrite(value, 1, length, fp);
        fprintf(fp, "</lds_version>\n");
        break;

      case LDS_TAG_UNICODE_VERSION:
        fprintf(fp, "\t\t<unicode_version>");
        fwrite(value, 1, length, fp);
        fprintf(fp, "</unicode_version>\n");
        break;

      case LDS_TAG_TAG_LIST:   /* List of DG */
        fprintf(fp, "\t\t<presence_map>");
        for (i = 0; i < length; i++)
        {
          fprintf(fp, "%02X", value[i]);
        }
        fprintf(fp, "</presence_map>\n");
        break;
      default:
        MrtdVerbose("MrtdParseEFCOM : Tag=%04X Length=%d", tag, length);
        break;
    }
  }

  return MRTD_SUCCESS;
}

static void MrtdMrzDecode(char buffer[], char empty_char, const BYTE data[], WORD size)
{
  size_t i;

  assert(buffer != NULL);
  assert(data != NULL);

  memcpy(buffer, data, size);
  buffer[size] = '\0';


  for (i = 0; i < strlen(buffer); i++)
    if (buffer[i] == '<')
      buffer[i] = empty_char;
}

static LONG MrtdDG1ToXmlEx(FILE * fp, BYTE data[], WORD size)
{
  char buffer[100 + 1];
  int i;

  assert (fp != NULL);
  assert (data != NULL);

  if (size >= sizeof(buffer))
    return MRTD_E_BAD_LENGTH;

  /* Document type */
  MrtdMrzDecode(buffer, '\0', &data[0], 2);
  fprintf(fp, "\t\t<type>%s</type>\n", buffer);

  /* Issuing state or organisation */
  MrtdMrzDecode(buffer, '\0', &data[2], 3);
  fprintf(fp, "\t\t<issuer>%s</issuer>\n", buffer);

  /* Name of holder */
  MrtdMrzDecode(buffer, ' ', &data[5], 39);
  i = strlen(buffer) - 1;
  while ((i >= 0) && (buffer[i] == ' '))
    buffer[i--] = '\0';
  fprintf(fp, "\t\t<fullName>%s</fullName>\n", buffer);

  /* Cut the name into 2 parts (surname / givenname) */
  while ((i >= 2) && ((buffer[i - 1] != ' ') || (buffer[i - 2] != ' ')))
    i--;
  if (i >= 2)
  {
    buffer[i - 2] = '\0';
    fprintf(fp, "\t\t<surName>%s</surName>\n", buffer);
    fprintf(fp, "\t\t<givenName>%s</givenName>\n", &buffer[i]);
  }

  /* Document number (with check digit first) */
  MrtdMrzDecode(buffer, '\0', &data[53], 1);
  fprintf(fp, "\t\t<number check=\"%s\">", buffer);
  MrtdMrzDecode(buffer, '\0', &data[44], 9);
  fprintf(fp, "%s</number>\n", buffer);

  /* Nationality */
  MrtdMrzDecode(buffer, '\0', &data[54], 3);
  fprintf(fp, "\t\t<nationality>%s</nationality>\n", buffer);

  /* Date of birth (with check digit first) */
  MrtdMrzDecode(buffer, '\0', &data[63], 1);
  fprintf(fp, "\t\t<dateOfBirth format=\"yymmdd\" check=\"%s\">", buffer);
  MrtdMrzDecode(buffer, '\0', &data[57], 6);
  fprintf(fp, "%s</dateOfBirth>\n", buffer);

  /* Sex */
  MrtdMrzDecode(buffer, '\0', &data[64], 1);
  fprintf(fp, "\t\t<sex>%s</sex>\n", buffer);

  /* Date of expiry (with check digit first) */
  MrtdMrzDecode(buffer, '\0', &data[71], 1);
  fprintf(fp, "\t\t<dateOfExpiry check=\"%s\" format=\"yymmdd\">", buffer);
  MrtdMrzDecode(buffer, '\0', &data[65], 6);
  fprintf(fp, "%s</dateOfExpiry>\n", buffer);

  if (size > 74)
  {
    /* Optional data provided */
    if (size == 88)
    {
      /* Optional data check digit for ID-3 only */
      MrtdMrzDecode(buffer, '\0', &data[86], 1);
      fprintf(fp, "\t\t<optionalData check=\"%s\">", buffer);
    } else
    {
      fprintf(fp, "\t\t<optionalData>");
    }
    MrtdMrzDecode(buffer, '\0', &data[72], (WORD) (size - 74));
    fprintf(fp, "%s</optionalData>\n", buffer);
  }

  /* Compositif check digit */
  MrtdMrzDecode(buffer, '\0', &data[size - 1], 1);
  fprintf(fp, "\t\t<checkDigit>%s</checkDigit>\n", buffer);

  return MRTD_SUCCESS;
}

static LONG MrtdDG1ToFile(FILE * fp, BYTE data[], WORD size, BOOL xml_output)
{
  WORD offset = 0;
  WORD tag;
  WORD length = 0;
  BYTE * value;
  BYTE * root;

  assert (fp != NULL);
  assert (data != NULL);

  /* Check root TLV */
  if (!TLVLoop(data, &offset, &tag, &length, &root))
  {
    MrtdVerbose("MrtdParseDG1 : TLV parse error");
    return MRTD_E_BAD_HEADER;
  }

  if (offset > size)
  {
    MrtdVerbose("MrtdParseDG1 : specified length is invalid");
    return MRTD_E_BAD_HEADER;
  }

  if (tag != LDS_TAG_DG1)
  {
    MrtdVerbose("MrtdParseDG1 : wrong identifier");
    return MRTD_E_BAD_TAG;
  }

  /* Find TLVs under the root */
  offset = 0;
  size = length;

  while ((offset < size) && TLVLoop(root, &offset, &tag, &length, &value))
  {
    switch (tag)
    {
      case 0x5F1F:            /* Write MRZ data into file */
        if (xml_output)
        {
          /* Write data as XML elements */
          LONG rc = MrtdDG1ToXmlEx(fp, value, length);
          if (rc != MRTD_SUCCESS)
          {
            MrtdVerbose("MrtdParseDG1 : XML export failed");
            return rc;
          }
        } else
        {
          /* Raw data only */
          fwrite(value, length, 1, fp);
        }
        break;
      default:
        MrtdVerbose("MrtdDG1ToFile : Tag=%04X Length=%d", tag, length);
        break;
    }
  }

  return MRTD_SUCCESS;
}

/*
 * MrtdDG1ToTxt
 * ------------
 */
LONG MrtdDG1ToTxt(FILE * fp, BYTE data[], WORD size)
{
  return MrtdDG1ToFile(fp, data, size, FALSE);
}

/*
 * MrtdDG1ToXml
 * ------------
 */
LONG MrtdDG1ToXml(FILE * fp, BYTE data[], WORD size)
{
  return MrtdDG1ToFile(fp, data, size, TRUE);
}


/*
 * MrtdBioHeaderToXml
 * ------------------
 */
static LONG MrtdBioHeaderToXml(FILE * fp, BYTE data[], WORD size)
{
  WORD offset = 0;
  WORD tag;
  WORD length = 0;
  BYTE * value;
  DWORD dw;

  assert (fp != NULL);
  assert (data != NULL);

  offset = 0;
  while ((offset < size) && TLVLoop(data, &offset, &tag, &length, &value))
  {
    dw = TLVGetDWORD(value, length);
    switch (tag)
    {
      case 0x80:              /* Version */
        fprintf(fp, "\t\t\t\t<version>%04lX</version>\n", dw);
        break;
      case 0x81:              /* Type */
        fprintf(fp, "\t\t\t\t<type>%06lX</type>\n", dw);
        switch (dw)
        {
          case 0x000002:
            fprintf(fp, "\t\t\t\t<typeExplain>facial</typeExplain>\n");
            break;
          case 0x000008:
            fprintf(fp, "\t\t\t\t<typeExplain>finger</typeExplain>\n");
            break;
          case 0x000010:
            fprintf(fp, "\t\t\t\t<typeExplain>iris</typeExplain>\n");
            break;
        }
        break;
      case 0x82:              /* Version */
        fprintf(fp, "\t\t\t\t<feature>%08lX</feature>\n", TLVGetDWORD(value, length));
        break;
      case 0x83:
        break;                 /* To be done */
      case 0x84:
        break;                 /* To be done */
      case 0x85:
        break;                 /* To be done */
      case 0x86:             /* Creator PID */
        fprintf(fp, "\t\t\t\t<productIdentifier>%08lX</productIdentifier>\n", dw);
        break;
      case 0x87:              /* Format owner */
        fprintf(fp, "\t\t\t\t<formatOwner>%04lX</formatOwner>\n", dw);
        break;
      case 0x88:              /* Format type */
        fprintf(fp, "\t\t\t\t<formatType>%04lX</formatType>\n", dw);
        break;
      default:
        MrtdVerbose("MrtdBioHeaderToXml : Tag=%04X Length=%d", tag, length);
        break;
    }
  }

  return MRTD_SUCCESS;
}



/*
 * MrtdBioTemplateToXml
 * --------------------
 */
static LONG MrtdBioTemplateToXml(FILE * fp, BYTE data[], WORD size, const char *images_file_prefix, BOOL overwrite)
{
  WORD offset = 0;
  WORD tag;
  WORD length = 0;
  BYTE * value;
  LONG rc;

  assert (fp != NULL);
  assert (data != NULL);

  offset = 0;
  while ((offset < size) && TLVLoop(data, &offset, &tag, &length, &value))
  {
    switch (tag)
    {
      case LDS_TAG_BIO_DATA: /* Plain biometric data block */
        if (!strcmp((const char*)value, "FAC"))
        {
          /* CBEFF facial image */
          rc = MrtdBioCbeffFacToXml(fp, value, length, images_file_prefix, overwrite);
          if (rc != MRTD_SUCCESS)
            return rc;
        } else
        if (!strcmp((const char*)value, "FIR"))
        {
          /* CBEFF finger image */
          rc = MrtdBioCbeffFirToXml(fp, value, length, images_file_prefix, overwrite);
          if (rc != MRTD_SUCCESS)
            return rc;
        } else
        if (!strcmp((const char*)value, "FMR"))
        {
          /* CBEFF finger minutiae */
          rc = MrtdBioCbeffFmrToXml(fp, value, length, images_file_prefix, overwrite);
          if (rc != MRTD_SUCCESS)
            return rc;
        } else
        {
          MrtdVerbose("Skipping unsupported biometric data block");
        }
        break;

      case LDS_TAG_BIO_DATA_CIPHER: /* Enciphered biometric data block, not implemented here */
        MrtdVerbose("Skipping ciphered biometric data block");
        break;

      default:
        if (((tag & 0xF0) == 0xA0) && (tag != 0xA0))
        {
          /* Biometric header */
          fprintf(fp, "\t\t\t<bioHeader>\n");
          rc = MrtdBioHeaderToXml(fp, value, length);
          if (rc != MRTD_SUCCESS)
            return rc;
          fprintf(fp, "\t\t\t</bioHeader>\n");
        } else
        {
          MrtdVerbose("MrtdBioTemplateToXml : Tag=%04X Length=%d", tag, length);
        }
        break;
    }
  }

  return MRTD_SUCCESS;
}

/*
 * MrtdBioDgToXml
 * --------------
 * Common to DG2, DG3 & DG4
 */
static LONG MrtdBioDgToXml(FILE * fp, BYTE data[], WORD size, const char *images_file_prefix, BOOL overwrite)
{
  WORD offset = 0;
  WORD tag;
  WORD length = 0;
  BYTE * value;
  BYTE * root, *first;
  LONG rc;

  assert (fp != NULL);
  assert (data != NULL);

  /* Check root TLV */
  if (!TLVLoop(data, &offset, &tag, &length, &root))
  {
    return MRTD_E_BAD_HEADER;
  }

  if (offset > size)
  {
    return MRTD_E_BAD_HEADER;
  }

  if ((tag != LDS_TAG_DG2) && (tag != LDS_TAG_DG3) && (tag != LDS_TAG_DG4))
  {
    return MRTD_E_BAD_CONTENT;
  }

  /* Find first child under the root, must be 0x7F61 */
  offset = 0;

  if (!TLVLoop(root, &offset, &tag, &length, &first))
  {
    return MRTD_E_BAD_CONTENT;
  }

  if (tag != 0x7F61)
  {
    MrtdVerbose("Mandatory tag 0x7F61 not found as first child of bio DG");
    return MRTD_E_BAD_CONTENT;
  }

  /* Now parse first child */
  offset = 0;
  size = length;
  while ((offset < size) && TLVLoop(first, &offset, &tag, &length, &value))
  {
    switch (tag)
    {
      case LDS_TAG_COUNT: /* Number of instances (not handled here) */
        fprintf(fp, "\t\t<numberOfInstances>%ld</numberOfInstances>\n", TLVGetDWORD(value, length));
        break;
      case LDS_TAG_BIO_INFO_TPL: /* Biometric info template */
        fprintf(fp, "\t\t<bioTemplate>\n");
        rc = MrtdBioTemplateToXml(fp, value, length, images_file_prefix, overwrite);
        if (rc != MRTD_SUCCESS)
          return rc;
        fprintf(fp, "\t\t</bioTemplate>\n");
        break;
      default:
        MrtdVerbose("MrtdBioDataToXml : Tag=%04X Length=%d", tag, length);
      break;
    }
  }

  return MRTD_SUCCESS;
}

/*
 * MrtdImageDgToXml
 * ----------------
 * Common to DG5, DG7
 */
static LONG MrtdImageDgToXml(FILE * fp, BYTE data[], WORD size, const char *images_file_prefix, BOOL overwrite)
{
  WORD offset = 0;
  WORD tag;
  WORD length = 0;
  BYTE idx = 0;
  BYTE * value;
  BYTE * root;

  assert (fp != NULL);
  assert (data != NULL);

  /* Check root TLV */
  if (!TLVLoop(data, &offset, &tag, &length, &root))
  {
    return MRTD_E_BAD_HEADER;
  }

  if (offset > size)
  {
    return MRTD_E_BAD_HEADER;
  }

  if ((tag != LDS_TAG_DG5) && (tag != LDS_TAG_DG7))
  {
    return MRTD_E_BAD_CONTENT;
  }

  /* Now parse children */
  offset = 0;
  size = length;
  while ((offset < size) && TLVLoop(root, &offset, &tag, &length, &value))
  {
    switch (tag)
    {
      case LDS_TAG_COUNT: /* Number of instances (not handled here) */
        fprintf(fp, "\t\t<numberOfImages>%ld</numberOfImages>\n", TLVGetDWORD(value, length));
        break;
      case LDS_TAG_IMG_PORTRAIT: /* Portrait */
      case LDS_TAG_IMG_SIGNATURE: /* Signature */
        /* Actual picture */
        if ((images_file_prefix != NULL) && (strlen(images_file_prefix)))
        {
          /* Picture is put in a separate file */
          char filename[MAX_PATH];
          if (tag == LDS_TAG_IMG_PORTRAIT)
            sprintf(filename, "%s_pict_%d.jpg", images_file_prefix, idx++);
          else
            sprintf(filename, "%s_sign_%d.jpg", images_file_prefix, idx++);
          fprintf(fp, "\t\t\t<image src=\"%s\" size=\"%d\"/>\n", basename_p(filename), length);
          fprintf(fp, "\t\t\t<!-- exported to \"%s\" -->\n", filename);
          MrtdWriteFile(filename, value, length, overwrite);
          /* Hope the image has been written. Ignore the result */
        } else
        {
          /* Base64 encoding of the picture right into the XML file */
          fprintf(fp, "\t\t\t<image encoding=\"base64\" size=\"%d\">\n", length);
          fprint_base64(fp, value, length, 64);
          fprintf(fp, "\t\t\t</image>\n");
        }
        break;

      default:
        MrtdVerbose("MrtdBioDataToXml : Tag=%04X Length=%d", tag, length);
      break;
    }
  }

  return MRTD_SUCCESS;
}
/*
 * MrtdSaveToXMLEx
 * ---------------
 */
WZMRTD_LIB BOOL WZMRTD_LINK MrtdSaveToXMLEx(MRTD_CTX_ST * ctx, const char *filename, BOOL sep_images, BOOL add_unproc_raw, BOOL add_full_raw, BOOL overwrite)
{
  FILE * fp;
  LONG rc;
  BYTE dg;
  char images_file_prefix[MAX_PATH] = "";

  if (ctx == NULL)
    return FALSE;

#ifdef WZMRTD_DLL_EXPORTS
  /* The DLL can't access stdout */
  if ((filename == NULL) || !strlen(filename))
    filename = "mrtd.xml";
#endif

  if ((filename == NULL) || !strlen(filename))
  {
    fp = stdout;
  } else
  {
    /* Create output file (overwrite it if allowed) */
    if (!overwrite)
    {
      fp = fopen(filename, "r");
      if (fp != NULL)
      {
        /* File already exists */
        fclose(fp);
        MrtdSetLastError(ctx, MRTD_E_FILE_EXISTS);
        return FALSE;
      }
    }
    fp = fopen(filename, "wt");
    if (fp == NULL)
    {
      MrtdSetLastError(ctx, MRTD_E_FILE_OPEN_WRITE);
      return FALSE;
    }
  }

  /* If we are to separate the images, let's build the filename prefix */
  if ((filename != NULL) && sep_images)
  {
    int i;
    strcpy(images_file_prefix, filename);
    i = strlen(images_file_prefix);
    while ((i > 0) && images_file_prefix[i] != '.') i--;
    images_file_prefix[i] = '\0';
  }

  fprintf(fp, "<mrtd>\n");

  /* No DG0, but EF.COM instead... */
  if ((ctx->DgData[0] != NULL) && (ctx->DgSize[0]))
  {
    fprintf(fp, "\t<!-- content of EF.COM -->\n");
    fprintf(fp, "\t<ef_com>\n");
    rc = MrtdEFCOMToXml(fp, ctx->DgData[0], ctx->DgSize[0]);
    if (rc != MRTD_SUCCESS)
    {
      MrtdSetLastError(ctx, rc);
      goto failed;
    }
    fprintf(fp, "\t</ef_com>\n");
  }

  /* DG1 is the MRZ string */
  if ((ctx->DgData[1] != NULL) && (ctx->DgSize[1]))
  {
    fprintf(fp, "\t<!-- content of DG%d -->\n", 1);
    fprintf(fp, "\t<mrz>\n");
    rc = MrtdDG1ToXml(fp, ctx->DgData[1], ctx->DgSize[1]);
    if (rc != MRTD_SUCCESS)
    {
      MrtdSetLastError(ctx, rc);
      goto failed;
    }
    fprintf(fp, "\t</mrz>\n");
  }

  /* DG2, DG3, DG4 store biometric data */
  for (dg = 2; dg <= 4; dg++)
  {
    if ((ctx->DgData[dg] != NULL) && (ctx->DgSize[dg]))
    {
      fprintf(fp, "\t<!-- content of DG%d -->\n", dg);

      switch (dg)
      {
        case 2:
          fprintf(fp, "\t<facialBioData>\n");
          break;
        case 3:
          fprintf(fp, "\t<fingerBioData>\n");
          break;
        case 4:
          fprintf(fp, "\t<irisBioData>\n");
          break;
      }

      rc = MrtdBioDgToXml(fp, ctx->DgData[dg], ctx->DgSize[dg], images_file_prefix, overwrite);
      if (rc != MRTD_SUCCESS)
      {
        MrtdSetLastError(ctx, rc);
        goto failed;
      }

      switch (dg)
      {
        case 2:
          fprintf(fp, "\t</facialBioData>\n");
          break;
        case 3:
          fprintf(fp, "\t</fingerBioData>\n");
          break;
        case 4:
          fprintf(fp, "\t</irisBioData>\n");
          break;
      }
    }
  }

  /* DG5 is the displayed portrait */
  if ((ctx->DgData[5] != NULL) && (ctx->DgSize[5]))
  {
    fprintf(fp, "\t<!-- content of DG%d -->\n", 5);
    fprintf(fp, "\t<displayedPortrait>\n");

    rc = MrtdImageDgToXml(fp, ctx->DgData[5], ctx->DgSize[5], images_file_prefix, overwrite);
    if (rc != MRTD_SUCCESS)
    {
      MrtdSetLastError(ctx, rc);
      goto failed;
    }

    fprintf(fp, "\t</displayedPortrait>\n");
  }

  /* DG7 is the displayed signature or mark */
  if ((ctx->DgData[7] != NULL) && (ctx->DgSize[7]))
  {
    fprintf(fp, "\t<!-- content of DG%d -->\n", 7);
    fprintf(fp, "\t<displayedSignature>\n");

    rc = MrtdImageDgToXml(fp, ctx->DgData[7], ctx->DgSize[7], images_file_prefix, overwrite);
    if (rc != MRTD_SUCCESS)
    {
      MrtdSetLastError(ctx, rc);
      goto failed;
    }

    fprintf(fp, "\t</displayedSignature>\n");
  }


  fprintf(fp, "</mrtd>");

  if (filename != NULL)
    fclose(fp);
  return TRUE;

failed:
  if (filename != NULL)
    fclose(fp);
  return FALSE;
}

WZMRTD_LIB BOOL WZMRTD_LINK MrtdSaveToXML(MRTD_CTX_ST * mrtd_ctx, const char *xml_file_name)
{
  return MrtdSaveToXMLEx(mrtd_ctx, xml_file_name, TRUE, FALSE, FALSE, TRUE);
}

static const char base64[] =
  "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

void  fprint_base64(FILE * fp, const BYTE * data, WORD size, WORD wrap)
{
  WORD i, l;
  DWORD c;
  char block[4 + 1];

  assert(fp != NULL);
  assert(data != NULL);

  i = 0;
  l = 0;
  for (i = 0; i < size;)
  {
    c = data[i++];
    c <<= 8;
    if (i < size)
      c += data[i];
    i++;
    c <<= 8;
    if (i < size)
      c += data[i];
    i++;
    block[0] = base64[(c & 0x00fc0000) >> 18];
    block[1] = base64[(c & 0x0003f000) >> 12];
    block[2] = base64[(c & 0x00000fc0) >> 6];
    block[3] = base64[(c & 0x0000003f) >> 0];
    if (i > size)
      block[3] = '=';
    if (i > size + 1)
      block[2] = '=';
    block[4] = '\0';
    fprintf(fp, block);
    l += 4;
    if (wrap && (l >= wrap))
    {
      fprintf(fp, "\n");
      l = 0;
    }
  }
  if (wrap && l)
    fprintf(fp, "\n");
}

const char *basename_p(const char *filename)
{
  const char *p;

  assert(filename != NULL);

  p = filename + strlen(filename);
  while (p > filename)
  {
    if ((*p == '/') || (*p == '\\') || (*p == ':'))
      return p+1;
    p--;
  }

  return filename;
}

