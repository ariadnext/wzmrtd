/*
 * wzMRTD - An electronic passport reader library
 * Copyright (c) 2007, Johann Dantant - www.wzpass.net
 *
 * Please read LICENSE.txt for license and copyright info.
 */

#include "wzmrtd_i.h"

WZMRTD_LIB LONG WZMRTD_LINK MrtdGetLastError(MRTD_CTX_ST * mrtd_ctx)
{
  if (mrtd_ctx == NULL) return MRTD_E_INVALID_HANDLE;
  return mrtd_ctx->LastError;
}

void MrtdSetLastError(MRTD_CTX_ST * mrtd_ctx, LONG value)
{
  assert (mrtd_ctx != NULL);
  mrtd_ctx->LastError = value;
}

WZMRTD_LIB const char * WZMRTD_LINK MrtdTranslateError(LONG code)
{
  switch (code)
  {
    case MRTD_SUCCESS:
      return "No error";
    case MRTD_CANCELLED:
      return "Cancelled by user";
    case MRTD_E_READER_NOT_AVAIL:
      return "Reader support not available or not implemented";
    case MRTD_E_READER_NOT_FOUND:
      return "Reader not found";
    case MRTD_E_READER_COMM:
      return "Reader communication error";
    case MRTD_E_NO_CARD:
      return "Card not found";
    case MRTD_E_CARD_LOST:
      return "Card has been removed";
    case MRTD_E_CARD_COMM:
      return "Card communication error";
    case MRTD_E_CARD_ANSWER:
      return "Incoherent card answer";
    case MRTD_E_CARD_STATUS:
      return "Error signaled by the card";
    case MRTD_E_NO_LDS:
      return "Passport application not found on the card";
    case MRTD_E_NO_FILE:
      return "Mandatory file not found on the card";
    case MRTD_E_BAD_MRZ:
      return "Provided MRZ is invalid";
    case MRTD_E_BAC_NEEDS_MRZ:
      return "Basic authentication is needed, but MRZ not provided";
    case MRTD_E_BAC_FAILED:
      return "Basic authentication failed, please check content of MRZ";
    case MRTD_E_AUTH_INVALID:
      return "Secure communication error, is card genuine ?";
    case MRTD_E_BAD_HEADER:
      return "Invalid format in passport's data file";
    case MRTD_E_BAD_LENGTH:
      return "Invalid length in passport's data file";
    case MRTD_E_BAD_TAG:
      return "Invalid identifier in passport's data file";
    case MRTD_E_BAD_CONTENT:
      return "Incorrect data in passport's data file";
    case MRTD_E_FILE_EXISTS:
      return "File already exists, overwrite not permitted";
    case MRTD_E_FILE_OPEN_WRITE:
      return "Failed to open output file";
    case MRTD_E_FILE_OPEN_READ:
      return "Failed to open input file";
    case MRTD_E_FILE_WRITE:
      return "Failed to write into output file";
    case MRTD_E_FILE_READ:
      return "Failed to read from output file";
    case MRTD_E_INVALID_HANDLE:
      return "Invalid handle";
    case MRTD_E_INVALID_PARAM:
      return "Invalid parameter";
    case MRTD_E_INTERNAL_ERROR:
      return "Internal error";
    case MRTD_E_FUNCTION_NOT_AVAIL:
      return "Function not yet implemented";
    case MRTD_E_OUT_OF_MEMORY:
      return "Out of memory";
  }

  return "Unknown error code";
}

static CallbackProc *MrtdCallback = NULL;

WZMRTD_LIB void WZMRTD_LINK MrtdSetCallback(CallbackProc Callback)
{
  MrtdCallback = Callback;
}

BOOL MrtdStatus(const char *fmt,...) 
{
  char buffer[256];
  va_list arg_ptr;

  if (MrtdCallback == NULL) return TRUE;

  va_start(arg_ptr, fmt);
  vsprintf(buffer, fmt, arg_ptr);
  va_end(arg_ptr);

  return MrtdCallback(buffer, NULL, 0xFFFFFFFF, 0xFFFFFFFF);
}

BOOL MrtdVerbose(const char *fmt,...) 
{
  char buffer[256];
  va_list arg_ptr;

  if (MrtdCallback == NULL) return TRUE;

  va_start(arg_ptr, fmt);
  vsprintf(buffer, fmt, arg_ptr);
  va_end(arg_ptr);

  return MrtdCallback(NULL, buffer, 0xFFFFFFFF, 0xFFFFFFFF);
}

BOOL MrtdProgress(DWORD pos, DWORD max) 
{
  if (MrtdCallback == NULL) return TRUE;

  return MrtdCallback(NULL, NULL, pos, max);
}
