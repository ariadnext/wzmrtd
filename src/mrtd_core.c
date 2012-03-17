/*
 * wzMRTD - An electronic passport reader library
 * Copyright (c) 2007, Johann Dantant - www.wzpass.net
 *
 * Please read LICENSE.txt for license and copyright info.
 */

#include "wzmrtd_i.h"

/* We try to read files by bunches of 256 bytes... */
#define MAX_READ_SIZE     256
/* ... but some chips with BAC can't accept it... */
#define MAX_READ_SIZE_BAC 128

  
/*
 * MrtdSelectFile
 * --------------
 */ 
BOOL MrtdSelectFile(MRTD_CTX_ST * ctx, BYTE dg) 
{
  BYTE buffer[258];
  WORD recv_len;
  WORD sw;
  
  if (ctx == NULL)
    return FALSE;
  
  if (ctx->Bac.enabled)    
  {
    return MrtdBacSelectDG(ctx, dg);
  }
  
  /* Build the select file APDU */ 
  buffer[0] = 0x00;
  buffer[1] = 0xA4; 
  buffer[2] = 0x02;  
  buffer[3] = 0x0C;  
  buffer[4] = 0x02;
  
  if (dg == 0)    
  {
    /* File identifier for EF.COM is 011E */ 
    buffer[5] = 0x01;
    buffer[6] = 0x1E; 
  } else
  if (dg <= 16)    
  {
    /* File identifier for DGxx is 01xx */ 
    buffer[5] = 0x01;
    buffer[6] = dg;
  } else    
  {
    MrtdVerbose("MrtdSelectDG : Not implemented DG%d", dg);
    MrtdSetLastError(ctx, MRTD_E_INVALID_PARAM);
    return FALSE;
  }
  
  /* Send the APDU to the card */ 
  recv_len = sizeof(buffer); 
  if (!MrtdIccTransmit(ctx, buffer, 7, buffer, &recv_len))    
  {
    MrtdVerbose("MrtdSelectDG(%d) : IccTransmit failed", dg);
    return FALSE;
  }
  
  sw = MrtdIccStatusWord(ctx, buffer, recv_len);
  
  /* Check that we have 9000 at the end of the answer */ 
  if (sw != 0x9000)    
  {
    /* Perhaps it wants BAC ? */ 
    if (sw == 0x6982)      
    {
      MrtdVerbose("MrtdSelectDG(%d) : File needs BAC", dg);
      if (!ctx->Bac.enabled)      
      {
        if (!MrtdBacInitialize(ctx))         
        {
          MrtdVerbose("MrtdSelectDG(%d) : BAC failed", dg);
          return FALSE;
        }
      }
      
      return MrtdBacSelectDG(ctx, dg);
    }

    if (sw == 0x6A82)
    {
      MrtdVerbose("MrtdSelectDG(%d) : File not found", dg);
      MrtdSetLastError(ctx, MRTD_E_NO_FILE);
    } else
    {
      MrtdVerbose("MrtdSelectDG(%d) : Bad status word %04X", dg, sw);
      MrtdSetLastError(ctx, MRTD_E_CARD_ANSWER);
    }
    return FALSE;
  }
  
  return TRUE;
}


/*
 * MrtdReadFileEx
 * --------------
 * Read the content of currently selected file, starting at offset, up to len bytes
 * (len must be <= 256)
 * Returns the actual length that has been read
 */ 
BOOL MrtdReadFileEx(MRTD_CTX_ST * ctx, BYTE * data, WORD offset, WORD * length) 
{
  BYTE buffer[258];
  WORD recv_len;
  WORD sw;
 
  if (ctx == NULL)
    return FALSE;
  
  if (data == NULL)
    return FALSE;
  if (length == NULL)
    return FALSE;
  
  /* Read binary APDU */ 
  buffer[0] = 0x00; 
  buffer[1] = 0xB0;
  buffer[2] = (BYTE) (offset / 0x0100);
  buffer[3] = (BYTE) (offset % 0x0100);
  
  if (*length <= 255)    
  {
    buffer[4] = (BYTE) * length;
  } else
  if (*length == 256)    
  {
    buffer[4] = 0x00;
  } else
    return FALSE;
 
  /* Send the APDU to the card */ 
  recv_len = sizeof(buffer); 
  if (!MrtdIccTransmit(ctx, buffer, 5, buffer, &recv_len))    
  {
    MrtdVerbose("MrtdReadFileEx : IccTransmit failed");
    return FALSE;
  }
 
  sw = MrtdIccStatusWord(ctx, buffer, recv_len);  

  /* Check that we have 9000 at the end of the answer */ 
  if (sw != 0x9000) 
  {
    MrtdVerbose("MrtdReadFileEx(%d,%d) failed, SW=%04X", offset, *length);
    return FALSE;
  }

  /* Check returned length */
  if (recv_len - 2 > *length)
  {
    MrtdVerbose("MrtdReadFileEx(%d,%d) failed, response too long (%d > %d)", offset, *length, recv_len - 2, *length);
    MrtdSetLastError(ctx, MRTD_E_CARD_ANSWER);
    return FALSE;
  }

  /* Actual length is... */ 
  *length = recv_len - 2;
  
  /* Copy the data */ 
  if (data != NULL)
    memcpy(data, buffer, *length);
   
  /* Done */ 
  return TRUE;
}



BOOL MrtdReadFileSize(MRTD_CTX_ST * ctx, WORD * filesize) 
{
  BYTE header[4];
  WORD size;
  
  if (filesize == NULL)
    return FALSE;
  
  size = 4;
  
  if (ctx->Bac.enabled)    
  {
    if (!MrtdBacReadFileEx(ctx, header, 0, &size))     
      return FALSE;
  } else    
  {
    if (!MrtdReadFileEx(ctx, header, 0, &size))      
      return FALSE;
  }
  
  if (size != 4)    
    return FALSE;
  
  *filesize = TLVTotalSize(header);
  return TRUE;
}

BOOL MrtdReadFile(MRTD_CTX_ST * ctx, BYTE * filedata, WORD filesize) 
{
  WORD buffer_size;
  WORD remaining = filesize;
  WORD offset = 0;
  WORD size;
  
  MrtdProgress(0, filesize);
  
  if (ctx->Bac.enabled)
    buffer_size = MAX_READ_SIZE_BAC;
  else
    buffer_size = MAX_READ_SIZE;
  
  while (remaining)    
  {
    if (remaining > buffer_size)     
      size = buffer_size;
    else
      size = remaining;    

    if (ctx->Bac.enabled)
    {
      if (!MrtdBacReadFileEx(ctx, &filedata[offset], offset, &size))        
      {
        MrtdVerbose("MrtdBacReadFile : error at offset %d/%d\n", offset, filesize);
        return FALSE;
      }
    } else      
    {
      if (!MrtdReadFileEx(ctx, &filedata[offset], offset, &size))        
      {
        MrtdVerbose("MrtdReadFile : error at offset %d/%d\n", offset, filesize);
        return FALSE;
      }
    }    
    
    if (size > remaining)
    {
      MrtdVerbose("MrtdReadFile : invalid return size %d>%d at %d/%d\n", size, remaining, offset, filesize);
      return FALSE;
    }

    remaining -= size;   
    offset    += size;

    if (!MrtdProgress(offset, filesize))
    {
      MrtdSetLastError(ctx, MRTD_CANCELLED);
      return FALSE;
    }
  }
  
  return TRUE;
}

BOOL MrtdSelectApplet(MRTD_CTX_ST * ctx) 
{
  BYTE pbSendBuffer[] = { 0x00, 0xA4, 0x04, 0x0C, 0x07, 0xA0, 0x00, 0x00, 0x02, 0x47, 0x10, 0x01 };
  BYTE pbRecvBuffer[258];
  WORD wRecvLength = sizeof(pbRecvBuffer);
  WORD sw;  

  /* Send the APDU to the card */ 
  if (!MrtdIccTransmit (ctx, pbSendBuffer, sizeof(pbSendBuffer), pbRecvBuffer, &wRecvLength))    
  {
    MrtdVerbose("MrtdSelectApplet : IccTransmit failed");
    return FALSE;  
  }
  
  sw = MrtdIccStatusWord(ctx, pbRecvBuffer, wRecvLength);  

  /* Check that we have 9000 at the end of the answer */ 
  if (sw != 0x9000)   
  {
    if (sw == 0x6A82)
    {
      MrtdVerbose("MrtdSelectApplet : LDS not found");
      MrtdSetLastError(ctx, MRTD_E_NO_LDS);
    } else
    {
      MrtdVerbose("MrtdSelectApplet : Bad status word %04X", sw);
      MrtdSetLastError(ctx, MRTD_E_CARD_ANSWER);
    }
    return FALSE;
  }
  return TRUE;
}


BOOL MrtdReadPassportEx(MRTD_CTX_ST * ctx, DWORD want_dgs, const char *mrz_string) 
{
  DWORD total = 0;
  DWORD have_dgs;
  BYTE dg;
  clock_t t0, t1;
  
  if (ctx == NULL)
    return FALSE;

  if ((mrz_string != NULL) && strlen(mrz_string))
  {
    if (!MrtdAssignMrz(ctx, mrz_string))
      return FALSE;
  }
  
  t0 = clock();

  MrtdStatus("Selecting ICAO/MRTD LDS applet");

  if (!MrtdSelectApplet(ctx))    
    return FALSE;
  
  for (dg = 0; dg <= MRTD_DG_COUNT; dg++)     
  {
    BYTE * filedata;
    WORD filesize = 0;

    if (dg == 0)
    {
      /* This is EF.COM (pseudo DG 0). It is mandatory. DGList will be updated after it */
      MrtdStatus("Reading EF.COM");

    } else
    {    
      if (!(have_dgs & (1 << dg))) continue; /* DG is absent from EF.COM list */
      if (!(want_dgs & (1 << dg))) continue; /* User doesn't want this DG */

      if (ctx->Bac.enabled)
        MrtdStatus("Reading DG%d (secure mode)...", dg);
      else
        MrtdStatus("Reading DG%d...", dg);
    }    
   
    if (!MrtdSelectFile(ctx, dg))      
    {
      MrtdVerbose("Error selecting DG %d", dg);
      return FALSE;
    }
    
    MrtdVerbose("Querying file size", dg);
    
    /* First we read the header to retrieve the size */ 
    if (!MrtdReadFileSize(ctx, &filesize))     
    {
      /* Skip if EAC protected */
      if ( ctx->LastError == MRTD_E_SECURITY_STATUS )
      {
        /* Getting quicked out just screwed BAC */
        ctx->Bac.enabled = FALSE;
        continue;
      }
      MrtdVerbose("Failed to read header of DG%d", dg);
      return FALSE;
    }
    
    /* Allocate a buffer to hold the data */ 
    filedata = malloc(filesize);
    if (filedata == NULL)      
    {
      MrtdVerbose("Out of memory");
      MrtdSetLastError(ctx, MRTD_E_OUT_OF_MEMORY);
      return FALSE;
    }

    MrtdVerbose("Receiving file content", dg);
    
    /* Read the whole file */ 
    if (!MrtdReadFile(ctx, filedata, filesize))     
    {
      free(filedata);
      MrtdVerbose("Failed to read content of DG%d", dg);
      return FALSE;
    }
    
    total += filesize;

    /* Store pointer and data in context */ 
    ctx->DgSize[dg] = filesize;   
    ctx->DgData[dg] = filedata;    

    /* If file is EF.COM, parse it now to update the list */ 
    if (dg == 0)     
    {
      MrtdVerbose("Parsing EF.COM");

      if (!MrtdParseEFCOM(filedata, filesize, &have_dgs))        
      {
        MrtdVerbose("Content of EF.COM is invalid");
        MrtdSetLastError(ctx, MRTD_E_BAD_CONTENT);
        return FALSE;
      }
    }
  }
  
  t1 = clock();
  MrtdStatus("Done");
  MrtdVerbose("%ld bytes read in %ldms", total, ((t1 - t0) * CLOCKS_PER_SEC) / 1000);
  return TRUE;
}

