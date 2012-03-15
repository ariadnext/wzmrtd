/*
 * SHA-1 implementation
 * --------------------
 *
 * This is a derivative work from SHA-1 in C by Steve Reid <steve@edmweb.com>
 * 100% Public Domain
 */

#include "config.h"
#include "sha1.h"

#include <stdio.h>
#include <string.h>

/* SHA1 internal functions */
/* ----------------------- */
static void SHA1_Transform(DWORD state[5], const BYTE buffer[64]);

/* SHA1 initialization */
/* -------------------- */
void SHA1_Init(SHA1_CTX_ST *ctx)
{
  /* Current size is 0 */
  ctx->count[0] = ctx->count[1] = 0;
  /* Initial values for state */
  ctx->state[0] = 0x67452301;
  ctx->state[1] = 0xEFCDAB89;
  ctx->state[2] = 0x98BADCFE;
  ctx->state[3] = 0x10325476;
  ctx->state[4] = 0xC3D2E1F0;
}

/* SHA1 block update operation */
/* --------------------------- */
void SHA1_Update(SHA1_CTX_ST *ctx, const BYTE *input, DWORD inputLen)
{
  DWORD i, j;

  j = (ctx->count[0] >> 3) & 63;

  if ((ctx->count[0] += inputLen << 3) < (inputLen << 3))
    ctx->count[1]++;
  ctx->count[1] += (inputLen >> 29);

  if ((j + inputLen) > 63)
  {
    memcpy(&ctx->buffer[j], input, (i = 64-j));
    SHA1_Transform(ctx->state, ctx->buffer);
    for ( ; i+63 < inputLen; i+=64)
      SHA1_Transform(ctx->state, &input[i]);
    j = 0;
  } else
    i = 0;
  memcpy(&ctx->buffer[j], &input[i], inputLen - i);
}

/* SHA1 finalization */
/* ------------------ */
void SHA1_Final(SHA1_CTX_ST *ctx, BYTE digest[20])
{
  DWORD i;
  BYTE finalcount[8];

  for (i = 0; i < 8; i++)
  {
    finalcount[i] = (BYTE) ((ctx->count[(i >= 4 ? 0 : 1)] >> ((3-(i & 3)) * 8) ) & 255);  /* Endian independent */
  }

  SHA1_Update(ctx, "\200", 1);
  while ((ctx->count[0] & 504) != 448)
  {
    SHA1_Update(ctx, "\0", 1);
  }
  SHA1_Update(ctx, finalcount, 8);  /* Implies a final SHA1_Transform() */
  for (i = 0; i < 20; i++)
  {
    digest[i] = (BYTE) ((ctx->state[i>>2] >> ((3-(i & 3)) * 8) ) & 255);
  }
}

/* ROTATE_LEFT rotates x left n bits */
#define ROTATE_LEFT(x, n) (((x) << (n)) | ((x) >> (32-(n))))


#ifdef LITTLE_ENDIAN
  #define F0(i) (b512.l[i] = (ROTATE_LEFT(b512.l[i],24)&0xFF00FF00)|(ROTATE_LEFT(b512.l[i],8)&0x00FF00FF))
#else
  #define F0(i) b512.l[i]
#endif
#define F(i) (b512.l[i&15] = ROTATE_LEFT(b512.l[(i+13)&15]^b512.l[(i+8)&15]^b512.l[(i+2)&15]^b512.l[i&15],1))

/* (R0+R1), R2, R3, R4 are the different operations used in SHA1 */
#define R0(v,w,x,y,z,i) z +=((w&(x^y))^y)+F0(i)+0x5A827999+ROTATE_LEFT(v,5);w=ROTATE_LEFT(w,30);
#define R1(v,w,x,y,z,i) z +=((w&(x^y))^y)+F(i)+0x5A827999+ROTATE_LEFT(v,5);w=ROTATE_LEFT(w,30);
#define R2(v,w,x,y,z,i) z +=(w^x^y)+F(i)+0x6ED9EBA1+ROTATE_LEFT(v,5);w=ROTATE_LEFT(w,30);
#define R3(v,w,x,y,z,i) z +=(((w|x)&y)|(w&x))+F(i)+0x8F1BBCDC+ROTATE_LEFT(v,5);w=ROTATE_LEFT(w,30);
#define R4(v,w,x,y,z,i) z +=(w^x^y)+F(i)+0xCA62C1D6+ROTATE_LEFT(v,5);w=ROTATE_LEFT(w,30);

/* SHA-1 512-bit block hash. This is the core of the algorithm. */
static void SHA1_Transform(DWORD state[5], const BYTE buffer[64])
{
  DWORD a, b, c, d, e;
  typedef union
  {
    BYTE  c[64];
    DWORD l[16];
  } b512_st;
  b512_st b512;
  memcpy(&b512, buffer, 64);

  /* Copy context->state[] to working vars */
  a = state[0];
  b = state[1];
  c = state[2];
  d = state[3];
  e = state[4];

  /* 4 rounds of 20 operations each. Loop unrolled. */
  R0(a,b,c,d,e, 0); R0(e,a,b,c,d, 1); R0(d,e,a,b,c, 2); R0(c,d,e,a,b, 3);
  R0(b,c,d,e,a, 4); R0(a,b,c,d,e, 5); R0(e,a,b,c,d, 6); R0(d,e,a,b,c, 7);
  R0(c,d,e,a,b, 8); R0(b,c,d,e,a, 9); R0(a,b,c,d,e,10); R0(e,a,b,c,d,11);
  R0(d,e,a,b,c,12); R0(c,d,e,a,b,13); R0(b,c,d,e,a,14); R0(a,b,c,d,e,15);
  R1(e,a,b,c,d,16); R1(d,e,a,b,c,17); R1(c,d,e,a,b,18); R1(b,c,d,e,a,19);
  R2(a,b,c,d,e,20); R2(e,a,b,c,d,21); R2(d,e,a,b,c,22); R2(c,d,e,a,b,23);
  R2(b,c,d,e,a,24); R2(a,b,c,d,e,25); R2(e,a,b,c,d,26); R2(d,e,a,b,c,27);
  R2(c,d,e,a,b,28); R2(b,c,d,e,a,29); R2(a,b,c,d,e,30); R2(e,a,b,c,d,31);
  R2(d,e,a,b,c,32); R2(c,d,e,a,b,33); R2(b,c,d,e,a,34); R2(a,b,c,d,e,35);
  R2(e,a,b,c,d,36); R2(d,e,a,b,c,37); R2(c,d,e,a,b,38); R2(b,c,d,e,a,39);
  R3(a,b,c,d,e,40); R3(e,a,b,c,d,41); R3(d,e,a,b,c,42); R3(c,d,e,a,b,43);
  R3(b,c,d,e,a,44); R3(a,b,c,d,e,45); R3(e,a,b,c,d,46); R3(d,e,a,b,c,47);
  R3(c,d,e,a,b,48); R3(b,c,d,e,a,49); R3(a,b,c,d,e,50); R3(e,a,b,c,d,51);
  R3(d,e,a,b,c,52); R3(c,d,e,a,b,53); R3(b,c,d,e,a,54); R3(a,b,c,d,e,55);
  R3(e,a,b,c,d,56); R3(d,e,a,b,c,57); R3(c,d,e,a,b,58); R3(b,c,d,e,a,59);
  R4(a,b,c,d,e,60); R4(e,a,b,c,d,61); R4(d,e,a,b,c,62); R4(c,d,e,a,b,63);
  R4(b,c,d,e,a,64); R4(a,b,c,d,e,65); R4(e,a,b,c,d,66); R4(d,e,a,b,c,67);
  R4(c,d,e,a,b,68); R4(b,c,d,e,a,69); R4(a,b,c,d,e,70); R4(e,a,b,c,d,71);
  R4(d,e,a,b,c,72); R4(c,d,e,a,b,73); R4(b,c,d,e,a,74); R4(a,b,c,d,e,75);
  R4(e,a,b,c,d,76); R4(d,e,a,b,c,77); R4(c,d,e,a,b,78); R4(b,c,d,e,a,79);

  /* Add the working vars back into context.state[] */
  state[0] += a;
  state[1] += b;
  state[2] += c;
  state[3] += d;
  state[4] += e;
}

#ifdef _DEBUG

/*
 *****************************************************************************
 *
 *                                 VALIDATION
 *
 *****************************************************************************
 */

#include <stdio.h>

static BYTE c2b(char c)
{
  if (c >= '0' && c <= '9')
    return (BYTE) (c - '0');
  if (c >= 'a' && c <= 'f')
    return (BYTE)(c - 'a' + 10);
  if (c >= 'A' && c <= 'F')
    return (BYTE)(c - 'A' + 10);
  return 0;
}

static DWORD s2b(BYTE dst[], const char *src)
{
  DWORD i;
  DWORD l = strlen(src) / 2;

  for (i=0; i<l; i++)
    dst[i] = (BYTE) ((c2b(src[2*i])<<4) + c2b(src[2*i+1]));

  return l;
}

BOOL SHA1_Test(char *text, char *hash)
{
  SHA1_CTX_ST context;
  BYTE digest[20], expect[20];
  DWORD i, len = strlen(text);

  SHA1_Init(&context);
  SHA1_Update(&context, (BYTE *) text, len);
  SHA1_Final(&context, digest);

  s2b(expect, hash);
  if (memcmp(expect, digest, 20))
  {
    printf("SHA1 SelfTest failed\n");
    printf("Vector   : \"%s\"\n", text);
    printf("Hash     : ");
    for (i=0; i<20; i++)
      printf("%02X", digest[i]);
    printf("Expected : %s\n", hash);
    return FALSE;
  }
  return TRUE;
}

BOOL SHA1_AMillionTest(char *hash, int len)
{
  SHA1_CTX_ST context;
  BYTE digest[20], expect[20];
  DWORD i;
  char text[1000];

  if (len > 1000) return FALSE;
  if (1000000 % len) return FALSE;

  memset(text, 'a', len);

  SHA1_Init(&context);
  for (i=0; i<(DWORD)(1000000/len); i++)
    SHA1_Update(&context, (BYTE *) text, len);
  SHA1_Final(&context, digest);

  s2b(expect, hash);
  if (memcmp(expect, digest, 20))
  {
    printf("SHA1 SelfTest failed\n");
    printf("Vector   : 1 million of 'a' (paquets of %d)\n", len);
    printf("Hash     : ");
    for (i=0; i<20; i++)
      printf("%02X", digest[i]);
    printf("Expected : %s\n", hash);
    return FALSE;
  }
  return TRUE;
}

/*
Test Vectors (from FIPS PUB 180-1)
"abc"
  A9993E36 4706816A BA3E2571 7850C26C 9CD0D89D
"abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq"
  84983E44 1C3BD26E BAAE4AA1 F95129E5 E54670F1
A million repetitions of "a"
  34AA973C D4C4DAA4 F61EEB2B DBAD2731 6534016F
*/

BOOL SHA1_SelfTest(void)
{
  if (!SHA1_Test("abc", "a9993e364706816aba3e25717850c26c9cd0d89d")) return FALSE;
  if (!SHA1_Test("abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq", "84983E441C3BD26EBAAE4AA1F95129E5E54670F1")) return FALSE;
  if (!SHA1_AMillionTest("34aa973cd4c4daa4f61eeb2bdbad27316534016f", 1)) return FALSE;
  if (!SHA1_AMillionTest("34aa973cd4c4daa4f61eeb2bdbad27316534016f", 2)) return FALSE;
  if (!SHA1_AMillionTest("34aa973cd4c4daa4f61eeb2bdbad27316534016f", 10)) return FALSE;
  if (!SHA1_AMillionTest("34aa973cd4c4daa4f61eeb2bdbad27316534016f", 100)) return FALSE;
  if (!SHA1_AMillionTest("34aa973cd4c4daa4f61eeb2bdbad27316534016f", 250)) return FALSE;
  if (!SHA1_AMillionTest("34aa973cd4c4daa4f61eeb2bdbad27316534016f", 500)) return FALSE;
  if (!SHA1_AMillionTest("34aa973cd4c4daa4f61eeb2bdbad27316534016f", 1000)) return FALSE;
  return TRUE;
}

#endif
