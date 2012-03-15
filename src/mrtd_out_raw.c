/*
 * wzMRTD - An electronic passport reader library
 * Copyright (c) 2007, Johann Dantant - www.wzpass.net
 *
 * Please read LICENSE.txt for license and copyright info.
 */

#include "wzmrtd_i.h"

WZMRTD_LIB BOOL WZMRTD_LINK MrtdSaveToFiles(MRTD_CTX_ST * mrtd_ctx, const char *out_directory)
{
  return MrtdSaveToFilesEx(mrtd_ctx, out_directory, NULL, TRUE, TRUE, TRUE);
}

static void prepare_filename(char *filename, const char *out_directory, const char *file_name_prefix)
{
  filename[0] = '\0';

  if (out_directory != NULL)
    strcat(filename, out_directory);

  if (strlen(filename) > 0)
    if ((filename[strlen(filename) - 1] != '/')
     && (filename[strlen(filename) - 1] != '\\')
     && (filename[strlen(filename) - 1] != ':'))
    strcat(filename, "/");

  if (file_name_prefix != NULL)
    strcat(filename, file_name_prefix);
}

WZMRTD_LIB BOOL WZMRTD_LINK MrtdSaveToFilesEx(MRTD_CTX_ST * ctx, const char *out_directory, const char *file_name_prefix, BOOL sep_text, BOOL sep_images, BOOL overwrite)
{
  BYTE dg;
  LONG rc;
  char filename[MAX_PATH];

  if (ctx == NULL)
    return FALSE;

  if (out_directory != NULL)
#ifdef WIN32
    mkdir(out_directory);
#else
     mkdir(out_directory, S_IRWXU);
#endif

  for (dg = 0; dg <= 16; dg++)
  {
    if ((ctx->DgData[dg] == NULL) || (ctx->DgSize[dg] == 0))
      continue;

    prepare_filename(filename, out_directory, file_name_prefix);
    if (dg == 0)
      strcat(filename, "ef_com");
    else
      sprintf(filename + strlen(filename), "dg%d", dg);
    strcat(filename, ".raw");

    /* Write the file */
    rc = MrtdWriteFile(filename, ctx->DgData[dg], ctx->DgSize[dg], overwrite);
    if (rc != MRTD_SUCCESS)
    {
      MrtdVerbose("Failed to write DG%d as '%s'", dg, filename);
      MrtdSetLastError(ctx, rc);
      return FALSE;
    }
  }

  return TRUE;
}

LONG MrtdWriteFile(const char *filename, const BYTE data[], WORD size, BOOL overwrite)
{
  FILE *fp;
  size_t done;

  if (data == NULL) return MRTD_E_INVALID_PARAM;
  if (filename == NULL) return MRTD_E_INVALID_PARAM;

  /* Create output file (overwrite it if allowed) */
  if (!overwrite)
  {
    fp = fopen(filename, "rb");
    if (fp != NULL)
    {
      /* File already exists */
      fclose(fp);
      return MRTD_E_FILE_EXISTS;
    }
  }
  fp = fopen(filename, "wb");
  if (fp == NULL)
  {
    return MRTD_E_FILE_OPEN_WRITE;
  }

  while (size)
  {
    done = fwrite(data, 1, size, fp);
    if (done <= 0)
    {
      fclose(fp);
      return MRTD_E_FILE_WRITE;
    }
    size -= done;
    data += done;
  }


  fclose(fp);
  return MRTD_SUCCESS;
}