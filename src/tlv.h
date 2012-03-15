#ifndef __TLV_H__
#define __TLV_H__

WORD TLVTotalSize(BYTE buffer[]);
BOOL TLVLoop(BYTE buffer[], WORD * offset, WORD * tag, WORD * length, BYTE * value[]);
DWORD TLVGetDWORD(BYTE buffer[], WORD length);

#endif

