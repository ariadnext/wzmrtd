/*
 * wzMRTD - An electronic passport reader library
 * Copyright (c) 2007, Johann Dantant - www.wzpass.net
 *
 * Please read LICENSE.txt for license and copyright info.
 */

#include "wzmrtd_i.h"
  
BOOL MrtdIccTransmit(MRTD_CTX_ST * ctx, const BYTE send[], WORD send_len, BYTE recv[], WORD * recv_len) 
{
  if (ctx->pcsc_reader)
    return PCSC_IccTransmit(ctx, send, send_len, recv, recv_len);
#ifndef DISABLE_SPROX
  if (ctx->sprox_reader)
    return SPROX_IccTransmit(ctx, send, send_len, recv, recv_len);
#endif
  return FALSE;
}



WORD MrtdIccStatusWord(MRTD_CTX_ST * ctx, BYTE recv_buffer[], WORD recv_len) 
{
  if (recv_len < 2)
  {
    MrtdVerbose("MrtdIccStatusWord : Received buffer is too short");
    return 0xFFFF;
  }
  return (recv_buffer[recv_len - 2] * 0x0100 + recv_buffer[recv_len - 1]);
}




/*
 * IccGetChallenge
 * ----------------
 * Get the card challenge for mutual authentication
 */ 
BOOL MrtdIccGetChallenge(MRTD_CTX_ST * ctx, BYTE rnd_icc[8]) 
{
  BYTE send_buffer[] = { 0x00, 0x84, 0x00, 0x00, 0x08};
  BYTE recv_buffer[8 + 2];
  WORD recv_len = sizeof(recv_buffer);
  WORD sw;
  
  if (!MrtdIccTransmit(ctx, send_buffer, sizeof(send_buffer), recv_buffer, &recv_len))
  {
    MrtdVerbose("MrtdIccTransmit error");
    return FALSE;
  }

  sw = MrtdIccStatusWord(ctx, recv_buffer, recv_len);
  
  if (sw != 0x9000)
  {
    MrtdVerbose("MrtdIccGetChallenge : Wrong status word %04X", sw);
    MrtdSetLastError(ctx, MRTD_E_CARD_STATUS);
    return FALSE;
  }
  
  memcpy(rnd_icc, recv_buffer, 8);
  return TRUE;
}

/*
 * IccMutualAuthenticate
 * ---------------------
 */ 
BOOL MrtdIccMutualAuthenticate(MRTD_CTX_ST * ctx, BYTE send_buffer[40], BYTE recv_buffer[40]) 
{
  BYTE io_buffer[46];
  WORD resp_len;
  WORD sw;
  
  if (send_buffer == NULL)
    return FALSE;
  if (recv_buffer == NULL)
    return FALSE;
  
  io_buffer[0] = 0x00;        /* CLA */
  io_buffer[1] = 0x82;        /* INS */
  io_buffer[2] = 0x00;        /* P1  */
  io_buffer[3] = 0x00;        /* P2  */
  io_buffer[4] = 0x28;        /* Lc  */
  
  memcpy(&io_buffer[5], send_buffer, 40);
  
  io_buffer[45] = 0x28;       /* Le  */
  
  resp_len = 42;
  if (!MrtdIccTransmit(ctx, io_buffer, 46, io_buffer, &resp_len))
  {
    MrtdVerbose("MrtdIccTransmit error");
    return FALSE;
  }
  
  if (resp_len != 42)    
  {
    MrtdVerbose("MrtdIccGetChallenge : Card returned %d bytes instead of %d", resp_len, 42);
    MrtdSetLastError(ctx, MRTD_E_CARD_ANSWER);
    return FALSE;
  }
  
  sw = MrtdIccStatusWord(ctx, io_buffer, resp_len);
  if (sw != 0x9000) 
  {
    MrtdVerbose("MrtdIccGetChallenge : Wrong status word %04X", sw);
    MrtdSetLastError(ctx, MRTD_E_CARD_STATUS);
    return FALSE;
  }
  
  memcpy(recv_buffer, io_buffer, 40);
  return TRUE;
}


