#include "wzmrtd_i.h"

void RAND_GetBuffer(BYTE buffer[8])
{
  DWORD i;

  for (i = 0; i < 8; i++)
    buffer[i] = rand();
}

void RAND_GetBytes(BYTE buffer[], DWORD size)
{
  DWORD i;

  for (i = 0; i < size; i++)
    buffer[i] = rand();
}

DWORD RAND_GetDWord(void)
{
  BYTE buffer[8];
  DWORD rc;

  RAND_GetBuffer(buffer);
  memcpy(&rc, buffer, sizeof(DWORD));

  return rc;
}
