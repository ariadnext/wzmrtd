#ifndef __DES_H__
#define __DES_H__

typedef struct
{
  DWORD encrypt_subkeys[32];
  DWORD decrypt_subkeys[32];  
} DES_CTX_ST;

void DES_Init(DES_CTX_ST *des_ctx, const BYTE key_data[8]);
void DES_Encrypt(DES_CTX_ST *des_ctx, BYTE data[8]);
void DES_Decrypt(DES_CTX_ST *des_ctx, BYTE data[8]);

BOOL DES_SelfTest(void);

typedef struct
{
  DES_CTX_ST key1_ctx;
  DES_CTX_ST key2_ctx;
  DES_CTX_ST key3_ctx;
} TDES_CTX_ST;

void TDES_Init(TDES_CTX_ST *tdes_ctx, const BYTE key1_data[8], const BYTE key2_data[8], const BYTE key3_data[8]);
void TDES_Encrypt(TDES_CTX_ST *tdes_ctx, BYTE data[8]);
void TDES_Decrypt(TDES_CTX_ST *tdes_ctx, BYTE data[8]);

#endif
