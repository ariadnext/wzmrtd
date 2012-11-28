/*
 * wzMRTD - An electronic passport reader library
 * Copyright (c) 2007, Johann Dantant - www.wzpass.net
 *
 * Please read LICENSE.txt for license and copyright info.
 */

#include "wzmrtd_i.h"

static BOOL BacComputeMac (MRTD_CTX_ST * ctx, BYTE mac[8], const BYTE data[], WORD size);
static BOOL BacDerivateKeys (BYTE k_enc[16], BYTE k_mac[16], const BYTE k_seed[16]);
static BOOL BacComputeKSeed (BYTE k_seed[16], const BYTE mrz[MRZ_INFO_SIZE]);
static void BacSetKeyParityBits (BYTE key[16]);
static WORD BacEncryptBuffer (TDES_CTX_ST * tdes_ctx, BYTE data[], WORD size, WORD max_size, BYTE iv[8], BOOL force_padding);
static WORD BacDecryptBuffer (TDES_CTX_ST * tdes_ctx, BYTE data[], WORD size, BYTE iv[8]);
static WORD BacAddPadding (BYTE data[], WORD size, WORD max_size, BOOL force);
static WORD BacRemovePadding (BYTE data[], WORD size);
static void BacBuild_DO8E (MRTD_CTX_ST * ctx, BYTE dst[10], const BYTE data[], WORD size);
static void BacBuild_DO87 (MRTD_CTX_ST * ctx, BYTE dst[11], const BYTE data[], WORD size);
static void BacIncrementSSC (BYTE ssc[8]);

static const BYTE SELECT_FILE_CMD[] =
  { 0x0C, 0xA4, 0x02, 0x0C, 0x80, 0x00, 0x00, 0x00 };
static const BYTE READ_FILE_CMD[] =
  { 0x0C, 0xB0, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00 };
static const BYTE BUILD97[] = { 0x97, 0x01, 0x00 };

/*
 * MrtdBacInitialize
 * ---------------------
 * Perform mutual authentication and establish secure session using MRZ data
 */
BOOL MrtdBacInitialize (MRTD_CTX_ST * ctx)
{
  struct
  {
    BYTE rnd_icc[8];
    BYTE rnd_ifd[8];
    BYTE k_ifd[16];
    BYTE k_seed[16];
    BYTE k_enc[16];
    BYTE k_mac[16];
  }
  sensitive;

  BYTE buffer[64];
  BYTE mac[8];
  WORD i, size;

  if (ctx == NULL)
    return FALSE;

  if (ctx->Bac.enabled)
    return TRUE;

  if (!ctx->Mrz.provided)
  {
    MrtdSetLastError(ctx, MRTD_E_BAC_NEEDS_MRZ);
    MrtdStatus("Authentication is required, please fill-in MRZ");
    return FALSE;
  }

  MrtdStatus("Performing Basic Authentication...");

  /* Part 1 : compute keys from MRZ */
  /* ------------------------------ */

  /* Compute Kseed */
  if (!BacComputeKSeed (sensitive.k_seed, ctx->Mrz.content))
  {
    MrtdVerbose("MrtdBacInitialize : Failed to compute Kseed from MRZ");
    MrtdSetLastError(ctx, MRTD_E_INTERNAL_ERROR);
    return FALSE;
  }

  /* Compute Kenc and Kmac */
  if (!BacDerivateKeys (sensitive.k_enc, sensitive.k_mac, sensitive.k_seed))
  {
    MrtdVerbose("MrtdBacInitialize : Failed to compute Kenc and Kmac from Kseed");
    MrtdSetLastError(ctx, MRTD_E_INTERNAL_ERROR);
    return FALSE;
  }

  /* Initialise cipher context with Kenc */
  TDES_Init (&ctx->Bac.enc_ctx, &sensitive.k_enc[0], &sensitive.k_enc[8], &sensitive.k_enc[0]);

  /* Initialise MAC context with Kmac */
  DES_Init (&ctx->Bac.mac_ctx[0], &sensitive.k_mac[0]);
  DES_Init (&ctx->Bac.mac_ctx[1], &sensitive.k_mac[8]);

  /* Part 2 : build our challenge with random data */
  /* --------------------------------------------- */

  /* Generate the 8 and 16 bytes random number */
  RAND_GetBytes (sensitive.rnd_ifd, 8);
  RAND_GetBytes (sensitive.k_ifd, 16);

  /* Part 3 : get card's challenge */
  /* ----------------------------- */

  if (!MrtdIccGetChallenge (ctx, sensitive.rnd_icc))
  {
    MrtdVerbose("MrtdBacInitialize : IccGetChallenge failed");
    return FALSE;
  }

  /* Part 4 : build the authentication cryptogram */
  /* -------------------------------------------- */

  /* Concatenate the datagram */
  memcpy (&buffer[0], sensitive.rnd_ifd, 8);
  memcpy (&buffer[8], sensitive.rnd_icc, 8);
  memcpy (&buffer[16], sensitive.k_ifd, 16);

  /* Cipher the datagram */
  memset (ctx->Bac.ssc, 0x00, 8); /* This is the 00 IV */

  size = BacEncryptBuffer (&ctx->Bac.enc_ctx, buffer, 32, sizeof (buffer), ctx->Bac.ssc, FALSE);

  /* Returned size shoul'd be... */
  assert (size == 32);

  /* Append the MAC */
  memset (ctx->Bac.ssc, 0x00, 8); /* This is the 00 IV */
  BacComputeMac (ctx, mac, buffer, 32);
  memcpy (&buffer[32], mac, 8);

  /* Part 5 : send the cryptogram to the card, and get its response */
  /* -------------------------------------------------------------- */

  if (!MrtdIccMutualAuthenticate (ctx, buffer, buffer))
  {
    MrtdVerbose("MrtdBacInitialize : IccMutualAuthenticate failed");
    MrtdStatus("Authentication failed, please verify MRZ");
    if ( ctx->Mrz.checksumsok && !ctx->Mrz.fixedUpMrz ) {
      MrtdSetLastError(ctx, MRTD_E_BAC_FAILED);
    }
    else {
      MrtdSetLastError(ctx, MRTD_E_BAC_INVALID_MRZ);
    }
    return FALSE;
  }

  /* Part 6 : check that card's response is genuine */
  /* ---------------------------------------------- */

  /* Check the MAC */
  memset (ctx->Bac.ssc, 0x00, 8); /* This is the 00 IV */
  BacComputeMac (ctx, mac, buffer, 32);

  if (memcmp (mac, &buffer[32], 8))
  {
    MrtdVerbose("MrtdBacInitialize : Wrong MAC in card's answer");
    MrtdSetLastError(ctx, MRTD_E_AUTH_INVALID);
    return FALSE;
  }

  /* Decrypt response data encryption (only 32 first bytes) */
  size = BacDecryptBuffer (&ctx->Bac.enc_ctx, buffer, 32, ctx->Bac.ssc);

  /* Check RND ICC */
  if (memcmp (sensitive.rnd_icc, &buffer[0], 8))
  {
    MrtdVerbose ("MrtdBacInitialize : Wrong Rnd.Icc in card's answer");
    MrtdSetLastError(ctx, MRTD_E_AUTH_INVALID);
    return FALSE;
  }

  /* Check RND IFD */
  if (memcmp (sensitive.rnd_ifd, &buffer[8], 8))
  {
    MrtdVerbose ("MrtdBacInitialize : Wrong Rnd.Ifd in card's answer");
    MrtdSetLastError(ctx, MRTD_E_AUTH_INVALID);
    return FALSE;
  }

  /* Part 7 : compute session key */
  /* ---------------------------- */

  /* Kseed = K_ifd XOR K_icc */
  for (i = 0; i < 16; i++)
    sensitive.k_seed[i] = sensitive.k_ifd[i] ^ buffer[i + 16];

  /* Compute session keys Kenc and Kmac */
  if (!BacDerivateKeys (sensitive.k_enc, sensitive.k_mac, sensitive.k_seed))
  {
    MrtdVerbose ("MrtdBacInitialize : Failed to compute session keys");
    MrtdSetLastError(ctx, MRTD_E_INTERNAL_ERROR);
    return FALSE;
  }

  /* Establish new context with session keys */
  TDES_Init (&ctx->Bac.enc_ctx, &sensitive.k_enc[0], &sensitive.k_enc[8], &sensitive.k_enc[0]);
  DES_Init (&ctx->Bac.mac_ctx[0], &sensitive.k_mac[0]);
  DES_Init (&ctx->Bac.mac_ctx[1], &sensitive.k_mac[8]);

  /* Initialize sequence counter */
  for (i = 0; i < 4; i++)
  {
    ctx->Bac.ssc[i] = sensitive.rnd_icc[4 + i];
    ctx->Bac.ssc[i + 4] = sensitive.rnd_ifd[4 + i];
  }

  /* Zeroize sensitive data */
  memset (&sensitive, 0xCC, sizeof (sensitive));
  memset (&buffer, 0xCC, sizeof (buffer));

  /* Done */
  MrtdStatus("Reading passport (secure communication)...");
  ctx->Bac.enabled = TRUE;
  return TRUE;
}

/*
 * MrtdBacReadFileEx
 * -----------------
 * Random access read from file (BAC secured version)
 */
BOOL MrtdBacReadFileEx (MRTD_CTX_ST * ctx, BYTE data[], WORD offset, WORD * length)
{
  BYTE build8E[10];
  BYTE buffer[256 + 16];
  BYTE iv[8] = { 0 };
  BYTE resp_mac[8];
  WORD resp_len;
  WORD want_len;
  WORD recv_len;
  WORD sw;

  if (ctx == NULL)
    return FALSE;

  if (!ctx->Bac.enabled)
    return FALSE;

  if (data == NULL)
    return FALSE;

  if (length == NULL)
    return FALSE;

  want_len = *length;

  if (want_len > 256)
    return FALSE;

  /* Step 1 : build the protected APDU */
  /* --------------------------------- */
  memcpy (buffer, READ_FILE_CMD, 8);

  /* Offset (P1 / P2) */
  buffer[2] = (BYTE) (offset / 0x0100);
  buffer[3] = (BYTE) (offset % 0x0100);
  memcpy (&buffer[8], BUILD97, 3);

  /* Length (Le) */
  if (want_len == 256)
    buffer[10] = 0x00;
  else
    buffer[10] = (BYTE) want_len;

  BacBuild_DO8E (ctx, build8E, buffer, 11);

  memcpy (buffer, READ_FILE_CMD, 4);

  buffer[2] = (BYTE) (offset / 0x0100);
  buffer[3] = (BYTE) (offset % 0x0100);
  buffer[4] = 0x0D;
  memcpy (&buffer[5], BUILD97, 3);

  /* Length (Le) */
  if (want_len == 256)
    buffer[7] = 0x00;
  else
    buffer[7] = (BYTE) want_len;

  memcpy (&buffer[8], build8E, 10);
  buffer[18] = 0x00;

  /* Step 2 : do the exchange */
  /* ------------------------ */
  resp_len = sizeof (buffer);

  if (!MrtdIccTransmit (ctx, buffer, 19, buffer, &resp_len))
  {
    MrtdVerbose ("MrtdBacReadFile : Read binary failed");
    return FALSE;
  }


  /* Step 3 : decode and check the response  */
  /* --------------------------------------- */
  sw = MrtdIccStatusWord (ctx, buffer, resp_len);
  if (sw != 0x9000)
  {
    MrtdVerbose ("MrtdBacReadFile : Bad status word %04X", sw);
    if (sw != 0x6982)
        MrtdSetLastError(ctx, MRTD_E_CARD_STATUS);
    else
        /* DG is EAC or PACE protected */
        MrtdSetLastError(ctx, MRTD_E_SECURITY_STATUS);  
    
    return FALSE;
  }

  /* How many bytes for length do we have ?
     If we have 1....... then we have ....... bytes */
  if ((buffer[1] >> 7) == 0x01)
    offset = buffer[1] - 0x80;
  else
    offset = 0;

  /* Calculate the length */
  /* Compute RAPDU CC of DO '87' and DO '99' */
  BacComputeMac (ctx, resp_mac, buffer, (WORD) (buffer[1 + offset] + 6 + offset));

  /* RAPDU CC is behind Do '99' */
  if (memcmp (resp_mac, buffer + (resp_len - 10 >= 0 ? resp_len - 10 : resp_len), 8))
  {
    MrtdVerbose("MrtdBacReadFile : Wrong MAC in card's answer");
    MrtdSetLastError(ctx, MRTD_E_AUTH_INVALID);
    return FALSE;
  }

  /* Decrypt data of DO '87' to obtain the length */
  recv_len = BacDecryptBuffer (&ctx->Bac.enc_ctx, &buffer[3 + offset], (WORD) (buffer[1 + offset] - 1), iv);
  if (recv_len < want_len)
  {
//  Verbose(VRB_DEBUG, "Too short (%d < %d)\n", recv_len, want_len);
  } else
  if (recv_len > want_len)
  {
    MrtdVerbose ("MrtdBacReadFile : Read binary is too long (%d > %d)\n", recv_len, want_len);
    MrtdSetLastError(ctx, MRTD_E_CARD_ANSWER);
    return FALSE;
  }

  /* If file_size = 0 then we have just read the header (4 bytes) so we have to determine the length */
  memcpy (data, buffer + 3 + offset, recv_len);
  *length = recv_len;
  return TRUE;
}

/*
 * BacSelectFile
 * -------------
 */
BOOL MrtdBacSelectDG (MRTD_CTX_ST * ctx, BYTE dg)
{
  BYTE buffer[256 + 16];
  BYTE build87[11];
  BYTE build8E[10];
  WORD resp_len;
  BYTE file_id[] = {
    0x01, 0xFF
  };
  BYTE rapdu_cc[8];
  WORD sw;

  /* Construct the DO '87' of File ID */
  if (dg == 0) {
    file_id[1] = 0x1E;
  } else
  if (dg < MRTD_DG_COUNT)    
  {
    /* File identifier for DGxx is 01xx */ 
    file_id[1] = dg;
  } else    
  if (dg == MRTD_DG_COUNT)    
  {
    /* File identifier for EF.SOD is 011D */ 
    file_id[1] = 0x1D;
  } else    
  {
    MrtdVerbose("MrtdSelectDG : Not implemented DG%d", dg);
    MrtdSetLastError(ctx, MRTD_E_INVALID_PARAM);
    return FALSE;
  }

  BacBuild_DO87 (ctx, build87, file_id, 2);
  memcpy (&buffer[0], SELECT_FILE_CMD, 8);
  memcpy (&buffer[8], build87, 11);
  BacBuild_DO8E (ctx, build8E, buffer, 19);

  /* Now we can construct the real Protected APDU */
  memcpy (&buffer[0], SELECT_FILE_CMD, 4);  /* we do not copy padding this time */
  buffer[4] = 0x15;
  memcpy (&buffer[5], build87, 11);
  memcpy (&buffer[16], build8E, 10);
  buffer[26] = 0x00;

  resp_len = sizeof (buffer);
  if (!MrtdIccTransmit (ctx, buffer, 27, buffer, &resp_len))
  {
    MrtdVerbose ("MrtdBacSelectDG(%d) : Select failed", dg);
    return FALSE;
  }

  sw = MrtdIccStatusWord (ctx, buffer, resp_len);

  if (sw != 0x9000)
  {
    if (sw == 0x6A82)
    {
      MrtdVerbose ("MrtdBacSelectDG(%d) : File not found", dg);
      MrtdSetLastError(ctx, MRTD_E_NO_FILE);
    } else
    {
      MrtdVerbose ("MrtdBacSelectDG(%d) : Bad status word %04X", dg, sw);
      MrtdSetLastError(ctx, MRTD_E_CARD_ANSWER);
    }
    return FALSE;
  }

  /* Verify RAPDU CC from Do '99' which is the fourth first bytes */
  /* /!\ Warning !! /!\
     If we do not want do desynchronyse the SSC we must perform the MAC even if there is an error.
     Typically we can have File Not Found (6A 82) and we want to see the other files */
  BacComputeMac (ctx, rapdu_cc, buffer, 4);

  /* RAPDU CC is behinid Do '99' */
  if (memcmp (rapdu_cc, &buffer[6], 8))
  {
    MrtdVerbose ("MrtdBacSelectDG(%d) : Wrong MAC in card's answer", dg);
    MrtdSetLastError(ctx, MRTD_E_AUTH_INVALID);
    return FALSE;
  }

  return TRUE;
}

/****************************************************************************
 *
 * KEYS COMPUTATION
 *
 ****************************************************************************/

/*
 * SetKeyParityBits
 * ----------------
 * Set the parity bits of the key
 */
static void BacSetKeyParityBits (BYTE key[16])
{
  int count;      /* Bits count */
  int i, j;
  
  assert(key != NULL);

  /* For each byte in the key */
  for (i = 0; i < 16; i++)
  {
    count = 0;

    /* Count bits */
    for (j = 1; j < 0xff; j *= 2)
      count += ((j & key[i]) > 0);

    /* If bits count is even, change it to odd number */
    if (!(count % 2))
      key[i] = key[i] ^ 0x01;
  }
}

/*
 * BacDerivateKeys
 * ---------------
 * Compute the Kenc and Kmac keys with the Kseed
 */
static BOOL BacDerivateKeys (BYTE k_enc[16], BYTE k_mac[16], const BYTE k_seed[16])
{
  SHA1_CTX_ST sha1_ctx;
  BYTE digest[20];
  const BYTE c_enc[4] = { 0x00, 0x00, 0x00, 0x01 };
  const BYTE c_mac[4] = { 0x00, 0x00, 0x00, 0x02 };

  if ((k_enc == NULL) || (k_mac == NULL) || (k_seed == NULL))
    return FALSE;

  /* Compute Kenc */
  memcpy (&digest[0], k_seed, 16);
  memcpy (&digest[16], c_enc, 4);

  SHA1_Init (&sha1_ctx);
  SHA1_Update (&sha1_ctx, digest, 20);
  SHA1_Final (&sha1_ctx, digest);

  memcpy (k_enc, digest, 16);

  /* Compute Kmac */
  memcpy (&digest[0], k_seed, 16);
  memcpy (&digest[16], c_mac, 4);

  SHA1_Init (&sha1_ctx);
  SHA1_Update (&sha1_ctx, digest, 20);
  SHA1_Final (&sha1_ctx, digest);

  memcpy (k_mac, digest, 16);
  BacSetKeyParityBits (k_enc);
  BacSetKeyParityBits (k_mac);
  return TRUE;

}

/*
 * BacComputeKSeed
 * ---------------
 * Compute Kseed from the MRZ data
 */
static BOOL BacComputeKSeed (BYTE k_seed[16], const BYTE mrz[MRZ_INFO_SIZE])
{
  SHA1_CTX_ST sha1_ctx;
  BYTE digest[20];

  if ((k_seed == NULL) || (mrz == NULL))
    return FALSE;

  SHA1_Init (&sha1_ctx);
  SHA1_Update (&sha1_ctx, mrz, MRZ_INFO_SIZE);
  SHA1_Final (&sha1_ctx, digest);

  memcpy (k_seed, digest, 16);
  return TRUE;
}

/****************************************************************************
 *
 * PADDING AND CIPHERING
 *
 ****************************************************************************/

/* CBC mode on one block with zero IV according to ISO 11568-2 */
void DES_CBC_Encrypt(DES_CTX_ST *des_ctx, BYTE data[8], BYTE iv[8])
{
	/* Perform the XOR */
	int i;
	for (i = 0; i < 8; i++)
		data[i] = data[i] ^ iv[i];
	/* Cipher */
	DES_Encrypt(des_ctx, data);
	/* Copy the new cipher block as vector for the next step */
	memcpy(iv, data, 8);
}

/* CBC mode on one block with zero IV according to ISO 11568-2 */
void TDES_CBC_Encrypt(TDES_CTX_ST *tdes_ctx, BYTE data[8], BYTE iv[8])
{
	/* Perform the XOR */
	int i;
	for (i = 0; i < 8; i++)
		data[i] = data[i] ^ iv[i];
	/* Cipher */
	TDES_Encrypt(tdes_ctx, data);
	/* Copy the new cipher block as vector for the next step */
	memcpy(iv, data, 8);
}

/* CBC mode on one bloc with zero IV according to ISO 11568-2 */
void DES_CBC_Decrypt(DES_CTX_ST *des_ctx, BYTE data[8], BYTE iv[8])
{
	int i;
	/* Decipher */
	DES_Decrypt(des_ctx, data);
	/* Perform the XOR */
	for (i = 0; i < 8; i++)
		data[i] = data[i] ^ iv[i];
}

/* CBC mode on one bloc with zero IV according to ISO 11568-2 */
void TDES_CBC_Decrypt(TDES_CTX_ST *tdes_ctx, BYTE data[8], BYTE iv[8])
{
	int i;
	/* Decipher */
	TDES_Decrypt(tdes_ctx, data);
	/* Perform the XOR */
	for (i = 0; i < 8; i++)
		data[i] = data[i] ^ iv[i];
}

/*
 * BacAddPadding
 * -------------
 * Perform ISO 9797-1 method 2 padding
 * This method detect if padding is necessary or not.
 * It can be forced when mandatory
 */
static WORD BacAddPadding (BYTE data[], WORD size, WORD max_size, BOOL force)
{
  WORD padded_size = size;

  if (data == NULL)
    return 0;

  if (max_size < size)
    return 0;

  /* Append 00 at the end of the buffer */
  memset (&data[size], 0x00, max_size - size);

  /* We must padd only if force is specified or if size is not multiple of 8 */
  if ((size % 8) || force)
  {
    padded_size /= 8;
    padded_size += 1;
    padded_size *= 8;
  }

  /* Set first bit of padding to 1 */
  if (padded_size != size)
    data[size] = 0x80;

  return padded_size;
}

/*
 * BacRemovePadding
 * ----------------
 * Detect padding method2 of data and return the actual size
 */
static WORD BacRemovePadding (BYTE data[], WORD size)
{
  WORD remind = size;

  if (data == NULL)
    return 0;

  if (size % 8)
    return 0;

  /* Detect padding */
  if ((data[size - 1] == 0x00) || (data[size - 1] == 0x80))
  {
    while ((size > 0) && (data[size - 1] == 0x00))
      size--;

    /* We found a 0x80 before the data is empty so we have to remove it */
    if (data[size - 1] == 0x80)
      size--;
    else
      size = remind;
  }
  return size;
}

/*
 * BacEncryptBuffer
 * ----------------
 * CBC mode with zero IV according to ISO 11568-2 and padding according to ISO 9797-1 method 2
 * Padding can be forced if mandatory
 */
static WORD BacEncryptBuffer (TDES_CTX_ST * tdes_ctx, BYTE data[], WORD size, WORD max_size, BYTE iv[8], BOOL force_padding)
{
  WORD padded_size;
  WORD i;

  padded_size = BacAddPadding (data, size, max_size, force_padding);
  assert ((padded_size % 8) == 0);

  /* Now we can perform the encryption */
  for (i = 0; i < padded_size; i += 8)
    TDES_CBC_Encrypt (tdes_ctx, &data[i], iv);

  return padded_size;
}

/*
 * BacDecryptBuffer
 * ----------------
 * CBC mode with zero IV according to ISO 11568-2 and padding according to ISO 9797-1 method 2
 * IV is the actual IV (00000000 in ISO 11568-2)
 * The right size of data (without the padding) is returned
 */
static WORD BacDecryptBuffer (TDES_CTX_ST * tdes_ctx, BYTE data[], WORD size, BYTE iv[8])
{
  signed long i;

  if (size % 8)
    return 0;     /* Oups, fatal error here ! */

  /* We have to decipher all the blocks except for the first block which will work with IV value */
  for (i = size - 8; i > 0; i -= 8)
    TDES_CBC_Decrypt (tdes_ctx, &data[i], &data[i - 8]);

  /* First block is deciphered with IV */
  TDES_CBC_Decrypt (tdes_ctx, data, iv);

  /* Removing padding */
  return BacRemovePadding (data, size);

}

/* Build the DO '8E' over data (compute the MAC first) */
static void BacBuild_DO8E (MRTD_CTX_ST * ctx, BYTE dst[10], const BYTE data[], WORD size)
{
  BYTE mac[8];

  assert (ctx != NULL);
  assert (dst != NULL);
  assert (data != NULL);

  BacComputeMac (ctx, mac, data, size);

  dst[0] = 0x8E;
  dst[1] = 0x08;
  memcpy (&dst[2], mac, 8);
}

static void BacBuild_DO87 (MRTD_CTX_ST * ctx, BYTE dst[11], const BYTE data[], WORD size)
{
  BYTE padded_data[16];
  BYTE iv[8] = { 0 };

  assert (ctx != NULL);
  assert (dst != NULL);
  assert (data != NULL);
  /* Check that we've room for buffer + padding */
  assert (((unsigned int)size + 8) <= sizeof (padded_data));

  memcpy (padded_data, data, size);

  /* Padd and cipher data (padding is mandatory here) */
  size = BacEncryptBuffer (&ctx->Bac.enc_ctx, padded_data, size, sizeof (padded_data), iv, TRUE);

  /* Check size */
  if ((size + 3) > 11)
    return;

  /* Build the DO '87' */
  dst[0] = 0x87;
  dst[1] = size + 1;
  dst[2] = 0x01;
  memcpy (&dst[3], padded_data, size);

  /* Zeroize sensitive data */
  memset (padded_data, 0xCC, sizeof (padded_data));
  memset (iv, 0xCC, sizeof (iv));
}

/*
 * BacComputeMac
 * -------------
 * Cryptographic Checksum using ISO 9797-1 MAC algorithm 3 with block cipher DES,
 * zero IV and ISO 9797-1 padding method 2
 * If SSC is not 00, DES[&k_mac[0]](SSC) will be used as IV
 */
static BOOL BacComputeMac (MRTD_CTX_ST * ctx, BYTE mac[8], const BYTE data[], WORD size)
{
  BYTE padded_data[256 + 16];
  BYTE iv[8] = { 0 };
  WORD i;

  /* If SSC is not 00000000 or NULL, prepend data with it */
  if (memcmp (ctx->Bac.ssc, iv, 8))
  {
    BacIncrementSSC (ctx->Bac.ssc);
    memcpy (&padded_data[0], ctx->Bac.ssc, 8);
    memcpy (&padded_data[8], data, size);
    size += 8;
  } else
    memcpy (&padded_data[0], data, size);

  /* Padd if necessary */
  size = BacAddPadding (padded_data, size, sizeof (padded_data), TRUE);

  /* Now we can compute the checksum */
  for (i = 0; i < size; i += 8)
    DES_CBC_Encrypt (&ctx->Bac.mac_ctx[0], &padded_data[i], iv);

  /* At the end we must Decrypt Yn with &k_mac[8] and encrypt the result with &k_mac[0] */
  memcpy (mac, &padded_data[size - 8], 8);
  DES_Decrypt (&ctx->Bac.mac_ctx[1], mac);
  DES_Encrypt (&ctx->Bac.mac_ctx[0], mac);

  /* Zeroize sensitive data */
  memset (padded_data, 0xCC, sizeof (padded_data));
  memset (iv, 0xCC, sizeof (iv));
  return TRUE;
}

/* Increment SSC by one */
static void BacIncrementSSC (BYTE ssc[8])
{
  int i = 7;
  assert(ssc != NULL);
  while ((i >= 0) && !(++(ssc[i])))
    i--;
}
