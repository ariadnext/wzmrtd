/*
 * wzMRTD - An electronic passport reader library
 * Copyright (c) 2007, Johann Dantant - www.wzpass.net
 *
 * Please read LICENSE.txt for license and copyright info.
 */

#include "wzmrtd_i.h"

void fprint_base64(FILE * fp, const BYTE * data, WORD size, WORD wrap);
const char *basename_p(const char *filename);

/*
 * CBEFF Finger format
 * -------------------
 */
LONG MrtdBioCbeffFmrToXml(FILE * fp, BYTE data[], WORD size, const char *images_file_prefix, BOOL overwrite)
{
  assert (fp != NULL);
  assert (data != NULL);

  // TODO
  fprintf(fp, "\t<!-- CBEFF FMR not yet implemented -->\n");

  return MRTD_SUCCESS;
}

/*
 * CBEFF Iris format
 * -----------------
 */
LONG MrtdBioCbeffFirToXml(FILE * fp, BYTE data[], WORD size, const char *images_file_prefix, BOOL overwrite)
{
  assert (fp != NULL);
  assert (data != NULL);

  // TODO
  fprintf(fp, "\t<!-- CBEFF FIR not yet implemented -->\n");

  return MRTD_SUCCESS;
}

/*
 * CBEFF Face format
 * -----------------
 */
LONG MrtdBioCbeffFacToXml(FILE * fp, BYTE data[], WORD size, const char *images_file_prefix, BOOL overwrite)
{  
  DWORD length;
  WORD offset, i;
  WORD idx, number, features;
  BYTE img_type = 0;
  
  assert (fp != NULL);
  assert (data != NULL);
  
  /* Size */ 
  offset = 8;
  length = 0;
  for (i = 0; i < 4; i++)    
  {
    length <<= 8;
    length += data[offset++];
  }
  
  if (size < length)
    return MRTD_E_BAD_LENGTH; /* Wrong size */
  
  /* Number of images */ 
  number = 0;
  for (i = 0; i < 2; i++)    
  {
    number <<= 8;
    number += data[offset++];
  }
  
  /* Loop for each image */ 
  /* ------------------- */ 
  for (idx = 0; idx < number; idx++)   
  {
    if (offset >= size)
      break;                   /* Be carefull... */
    
    fprintf(fp, "\t\t\t<bioDataHeader>\n");
    
    /* Facial information block */ 
    /* ------------------------ */ 
      
    length = 0;
    for (i = 0; i < 4; i++)      
    {
      length <<= 8;
      length += data[offset++];
    }
      
    /* Number of feature points */ 
    features = 0;   
    for (i = 0; i < 2; i++)      
    {
      features <<= 8;
      features += data[offset++];
    }    

    /* Gender */ 
    fprintf(fp, "\t\t\t\t<gender>");    
    switch (data[offset++])      
    {
      case 0x00:       
        fprintf(fp, "unspecified");
        break;
      case 0x01:
        fprintf(fp, "male");
        break;
      case 0x02:
        fprintf(fp, "female");
        break;
      case 0x03:
        fprintf(fp, "unknown");
        break;
    }    
    fprintf(fp, "</gender>\n");
    

    /* Eye color */ 
    fprintf(fp, "\t\t\t\t<eyeColor>");   
    switch (data[offset++])      
    {
      case 0x00:
        fprintf(fp, "unspecified");
        break;
      case 0x01:
        fprintf(fp, "blue");
        break;
      case 0x02:
        fprintf(fp, "brown");
        break;
      case 0x03:
        fprintf(fp, "green");
        break;
      case 0x12:
        fprintf(fp, "hazel");
        break;
      case 0x22:
        fprintf(fp, "maroon");
        break;
      case 0x10:
        fprintf(fp, "multi");
        break;
      case 0x20:
        fprintf(fp, "pink");
        break;
      case 0xFF:
        fprintf(fp, "unknown");
        break;
    }
    fprintf(fp, "</eyeColor>\n");
    

    /* Hair color */ 
    fprintf(fp, "\t\t\t\t<hairColor>");
    switch (data[offset++])     
    {
      case 0x00:
        fprintf(fp, "unspecified");
        break;
      case 0x01:
        fprintf(fp, "bald");
        break;
      case 0x02:
        fprintf(fp, "black");
        break;
      case 0x03:
        fprintf(fp, "blonde");
        break;
      case 0x04:
        fprintf(fp, "brown");
        break;
      case 0x05:
        fprintf(fp, "gray");
        break;
      case 0x06:
        fprintf(fp, "red");
        break;
      case 0x10:
        fprintf(fp, "blue");
        break;
      case 0x20:
        fprintf(fp, "green");
        break;
      case 0x30:
        fprintf(fp, "orange");
        break;
      case 0x40:
        fprintf(fp, "pink");
        break;
      case 0x13:
        fprintf(fp, "sandy");
        break;
      case 0x14:
        fprintf(fp, "auburn");
        break;
      case 0x15:
        fprintf(fp, "white");
        break;
      case 0x16:
        fprintf(fp, "strawberry");
        break;
      case 0x26:
        fprintf(fp, "orange");
        break;
      case 0x36:
        fprintf(fp, "pink");
        break;
      case 0xFF:
        fprintf(fp, "unknown");
        break;
    }
    fprintf(fp, "</hairColor>\n");
    
    /* Feature mask */ 
    if (data[offset + 2] & 0x01)      
    {
      if (data[offset + 2] & 0x02)
        fprintf(fp, "\t\t\t\t<feature value=\"glasses\">\n");
      if (data[offset + 2] & 0x04)
        fprintf(fp, "\t\t\t\t<feature value=\"moustache\">\n");
      if (data[offset + 2] & 0x08)
        fprintf(fp, "\t\t\t\t<feature value=\"beard\">\n");
      if (data[offset + 2] & 0x10)
        fprintf(fp, "\t\t\t\t<feature value=\"teeth visible\">\n");
      if (data[offset + 2] & 0x20)
        fprintf(fp, "\t\t\t\t<feature value=\"blink\">\n");
      if (data[offset + 2] & 0x40)
        fprintf(fp, "\t\t\t\t<feature value=\"mouth open\">\n");
      if (data[offset + 2] & 0x80)
        fprintf(fp, "\t\t\t\t<feature value=\"left eye patch\">\n");
      if (data[offset + 1] & 0x01)
        fprintf(fp, "\t\t\t\t<feature value=\"right eye patch\">\n");
      if (data[offset + 2] & 0x02)
        fprintf(fp, "\t\t\t\t<feature value=\"both eye patch\">\n");
      if (data[offset + 2] & 0x04)
        fprintf(fp, "\t\t\t\t<feature value=\"dark glasses\">\n");
      if (data[offset + 2] & 0x08)
        fprintf(fp, "\t\t\t\t<feature value=\"major medical condition\">\n");
    }
    offset += 3;    

    /* Expression */ 
    if (data[offset++] == 0x00)     
    {
      fprintf(fp, "\t\t\t\t<expression>");
      switch (data[offset++])       
      {
        case 0x00:
          fprintf(fp, "unspecified");
          break;
        case 0x01:
          fprintf(fp, "neutral");
          break;
        case 0x02:
          fprintf(fp, "smile, closed jaw");
          break;
        case 0x03:
          fprintf(fp, "smile, open jaw");
          break;
        case 0x04:
          fprintf(fp, "raised eyebrows");
          break;
        case 0x05:
          fprintf(fp, "looking away");
          break;
        case 0x06:
          fprintf(fp, "squinting");
          break;
        case 0x07:
          fprintf(fp, "frowning");
          break;
      }
      fprintf(fp, "</expression>\n");
    }
    
    /* Pose angle */ 
    offset += 3; // TODO
    
    /* Pose angle uncertainty */ 
    offset += 3; // TODO
    
    /* Feature points */ 
    /* -------------- */ 
      
    /* Skip feature points */ 
    offset += 8 * features;
    
    /* Image information */ 
    /* ----------------- */ 
      
    /* !!! Facial image type renamed to "imageMode" */ 
    fprintf(fp, "\t\t\t\t<imageMode>");   
    switch (data[offset++])     
    {
      case 0x00:
        fprintf(fp, "basic");
        break;
      case 0x01:
        fprintf(fp, "full frontal");
        break;
      case 0x02:
        fprintf(fp, "token frontal");
        break;
      case 0x03:
        fprintf(fp, "other");
        break;
      default:
        fprintf(fp, "%d", data[offset - 1]);
        break;
    }
    fprintf(fp, "</imageMode>\n");
    
    /* !!! Image data type renamed to "imageType" */ 
    fprintf(fp, "\t\t\t\t<imageType>");   
    switch (data[offset++])      
    {
      case 0x00:
        fprintf(fp, "jpeg");
        img_type = 1;
        break;
      case 0x01:
        fprintf(fp, "jpeg2000");
        img_type = 2;
        break;
      default:
        fprintf(fp, "%d", data[offset - 1]);
        break;
    }
    fprintf(fp, "</imageType>\n");
    
    fprintf(fp, "\t\t\t\t<imageWidth>%d</imageWidth>\n", (data[offset + 0] << 8) + data[offset + 1]);
    offset += 2;
    
    fprintf(fp, "\t\t\t\t<imageHeight>%d</imageHeight>\n", (data[offset + 0] << 8) + data[offset + 1]);
    offset += 2;
    
    fprintf(fp, "\t\t\t\t<imageColorSpace>%d</imageColorSpace>\n", data[offset++]);   
    fprintf(fp, "\t\t\t\t<imageSourceType>%d</imageSourceType>\n", data[offset++]);
    
    fprintf(fp, "\t\t\t\t<imageDeviceType>%d</imageDeviceType>\n", (data[offset + 0] << 8) + data[offset + 1]);
    offset += 2;
    
    fprintf(fp, "\t\t\t\t<imageQuality>%d</imageQuality>\n", (data[offset + 0] << 8) + data[offset + 1]);
    offset += 2;
    
    fprintf(fp, "\t\t\t</bioDataHeader>\n");    

    /* Actual picture */
    if ((images_file_prefix != NULL) && (strlen(images_file_prefix)))
    {
      /* Picture is put in a separate file */
      char filename[MAX_PATH];
      sprintf(filename, "%s_fac_%d", images_file_prefix, idx);
      switch (img_type)
      {
        case 1  : strcat(filename, ".jpg"); break; /* JPEG */
        case 2  : strcat(filename, ".jp2"); break; /* JPEG 2000 */
        default : strcat(filename, ".raw"); break; /* ? */
      }
      fprintf(fp, "\t\t\t<image src=\"%s\" size=\"%d\"/>\n", basename_p(filename), (WORD) (length + 14 - offset));  // TODO +14 ?
      fprintf(fp, "\t\t\t<!-- exported to \"%s\" -->\n", filename);
      MrtdWriteFile(filename, &data[offset], (WORD) (length + 14 - offset), overwrite);  // TODO +14 ?
      /* Hope the image has been written. Ignore the result */
    } else
    {
      /* Base64 encoding of the picture right into the XML file */
      fprintf(fp, "\t\t\t<image encoding=\"base64\" size=\"%d\">\n", (WORD) (length + 14 - offset));  // TODO +14 ?
      fprint_base64(fp, &data[offset], (WORD) (length + 14 - offset), 64);  // TODO +14 ?
      fprintf(fp, "\t\t\t</image>\n");
    }
  }
  
  /* Done */ 
  return MRTD_SUCCESS;
}
