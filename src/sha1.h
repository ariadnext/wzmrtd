#ifndef __SHA1_H__
#define __SHA1_H__

/*
 * SHA1
 * ----
 */
typedef struct
{
  DWORD state[5];              /* Current state */
  DWORD count[2];              /* Number of bits, modulo 2^64 (LSB first) */
  BYTE buffer[64];             /* Input buffer */
} SHA1_CTX_ST;

void SHA1_Init(SHA1_CTX_ST * sha1_ctx);
void SHA1_Update(SHA1_CTX_ST * sha1_ctx, const BYTE * input, DWORD size);
void SHA1_Final(SHA1_CTX_ST * sha1_ctx, BYTE digest[20]);

BOOL SHA1_SelfTest(void);

#endif
