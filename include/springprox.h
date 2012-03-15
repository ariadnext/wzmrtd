/**h* Pro-Active/SpringProxAPI
 *
 * NAME
 *   SpringProxAPI -- Pro-Active unified API for SpringProx, CSB and K531 contactless readers
 *
 * COPYRIGHT
 *   (c) 2000-2006 Pro-Active SAS - www.pro-active.fr
 *
 * AUTHOR
 *   Johann.D / Pro-Active
 *
 * HISTORY  
 *   JDA 15/02/2003 : [1.00]
 *                    creation from CSB3 tree
 *   JDA 16/05/2003 : [1.01]
 *                    added first set of SPROX_ functions
 *   JDA 01/07/2003 : [1.02 0]
 *                    added helpers for VB or Delphi
 *   JDA 30/09/2003 : [1.02 1]
 *                    added some utility functions
 *   JDA 13/10/2003 : [1.10]
 *                    added basic T=CL function set
 *                    added support for USB communication
 *   JDA 12/12/2003 : [1.11]
 *                    improved T=CL function set
 *   JDA 03/02/2004 : [1.12]
 *                    added SPROX_MifStSelectIdle and SPROX_MifStHalt
 *                    added SPROX_DES_... and SPROX_3DES_... (LGPL licence)
 *                    optimization for Desfire frame chaining
 *   JDA 20/11/2003 : [1.13]
 *                    bug-fix release
 *   JDA 25/11/2003 : [1.14]
 *                    bug-fix release
 *   JDA 25/01/2004 : [1.15]
 *                    added basic support for ISO 14443-B
 *   JDA 22/01/2004 : [1.16]
 *                    speed-improvement release
 *   JDA 12/02/2004 : [1.17]
 *                    reader-side changes only
 *   JDA 19/02/2004 : [1.18]
 *                    added support of 115200bps communication
 *   JDA 08/04/2004 : [1.19 0]
 *                    added support of ASCII protocol, and the GetDeviceSettings/SetDeviceSettings
 *                    functions
 *   JDA 05/05/2004 : [1.19 1]
 *                    added SPROX_MifStReadTag768 et SPROX_MifStWriteTag768
 *   JDA 17/06/2004 : [1.20]
 *                    support of multiple readers thanks to SPROX_CreateInstance, SPROX_SelectInstance,
 *                    and SPROX_DestroyInstance
 *   JDA 01/12/2004 : [1.21 0]
 *                    added the "const" modifier on most input buffers to pass strict pre-compiler checks
 *                    without a warning
 *                    fixed SPROX_TclA_ActivateAny bug
 *   JDA 14/01/2005 : [1.21 1]
 *                    added SPROX_GetFeatures and SPROX_Card_... functions
 *   JDA 25/05/2005 : [version 1.22]
 *                    added SPROX_ReaderSelectAddress
 *   JDA 21/06/2005 : [version 1.30]
 *                    added SPROX_MSO_Exchange and others MSO functions
 *   JDA 05/07/2005 : [version 1.31]
 *                    added SPROX_ReaderAttachSerial, moved multiple reader support to springprox_ex.h
 *   JDA 08/09/2005 : [version 1.32]
 *                    added SPROX_TclB_... functions
 *   JDA 08/09/2005 : [version 1.33]
 *                    reader-side changes only
 *   JDA 02/10/2005 : [version 1.34 0]
 *                    added SPROX_Card_PowerUp_Auto function
 *   JDA 14/10/2005 : [version 1.34 1]
 *                    added documentation for SPROX_Card_... functions
 *   LTX 12/04/2006 : [version 1.36 3]
 *                    added entry points for GetConsts and SetConsts
 *   JDA 24/05/2006 : [version 1.36 5]
 *                    extended USB support
 *                    every text-related function now has three version : xxx using TCHAR, xxxA and xxxW
 *                    using char (ASCII) and wchar_t (UNICODE) respectively.
 *   JDA 22/06/2006 : [version 1.40]
 *                    major rewriting :
 *                    - whole "multi-reader" stuff moved to a fully reentrant DLL (springprox_ex.dll)
 *                    - documentation moved to source code
 *                    - crypt part (DES, 3-DES and MD5) added
 *
 * PORTABILITY
 *   Win32 (Windows NT, 2000, XP)
 *   WinCE (Windows CE 3, Windows CE 4, Pocket PC 2000, Pocket PC 2002, Pocketc PC 2003)
 *   Linux (experimental)
 *
 **/

/*

  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
  ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
  TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
  PARTICULAR PURPOSE.
  
*/

#ifndef SPRINGPROX_H
#define SPRINGPROX_H

#ifdef UNDER_CE
  /* WinCE is only a subset of Win32 */
  #ifndef WIN32
    #define WIN32
  #endif
  #define WINCE
#endif

#ifdef _WIN32
  #ifndef WIN32
    #define WIN32
  #endif
#endif

#ifdef WIN32

  /* Win32 code */
  /* ---------- */

  #include <windows.h>
  #include <tchar.h>

  typedef signed short SWORD;
  typedef signed long  SDWORD;

  /* We are to link to the DLL */
  #ifndef SPRINGPROX_LIB
    #define SPRINGPROX_LIB __declspec( dllimport )
  #endif

  #ifdef UNDER_CE
    /* Under Windows CE we use the stdcall calling convention */
    #define SPRINGPROX_API __stdcall
  #else  
    #ifndef FORCE_STDCALL
      /* Under Desktop Windows we use the cdecl calling convention ... */
      #define SPRINGPROX_API __cdecl
    #else
      /* ... unless FORCE_STDCALL has been defined */
      #define SPRINGPROX_API __stdcall
    #endif
  #endif

#else

  /* Not Win32 */
  /* --------- */
  
  /* Linkage directive : not specified, use compiler default */
  #define SPRINGPROX_API
  
  /* Calling convention : not specified, use compiler default */
  #define SPRINGPROX_LIB
  
  #include <stdint.h>

	typedef uint8_t  BOOL;
  typedef uint8_t  BYTE;
  typedef int8_t   SBYTE;
  typedef uint16_t WORD;
  typedef int16_t  SWORD;
  typedef uint32_t DWORD;
  typedef int32_t  SDWORD;
  
  #ifndef TRUE
    #define TRUE 1
  #endif
  #ifndef FALSE
    #define FALSE 0
  #endif
  
  #ifdef HAVE_TCHAR_H
    #include <tchar.h>
  #else
    #ifdef UNICODE
    	typedef uint16_t TCHAR;
    #else
    	typedef char TCHAR;
    #endif
  #endif
  
#endif


#ifdef __cplusplus
/* SpringProx API is pure C */
extern  "C"
{
#endif

/* Library identification */
/* ---------------------- */
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_GetLibrary(TCHAR library[], WORD len);
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_GetLibraryW(wchar_t library[], WORD len);
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_GetLibraryA(char library[], WORD len);

/* Return code to error message translation */
/* ---------------------------------------- */
SPRINGPROX_LIB const TCHAR   *SPRINGPROX_API SPROX_GetErrorMessage(SWORD status);
SPRINGPROX_LIB const wchar_t *SPRINGPROX_API SPROX_GetErrorMessageW(SWORD status);
SPRINGPROX_LIB const char    *SPRINGPROX_API SPROX_GetErrorMessageA(SWORD status);

/* Reader access functions */
/* ----------------------- */

/* Open the reader (this library is not reentrant, we work with one reader at a time   */
/* Set device to the communication port name ("COM1", "USB", "/dev/ttyS2" ...) or NULL */
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_ReaderOpen(const TCHAR device[]);
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_ReaderOpenW(const wchar_t device[]);
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_ReaderOpenA(const char device[]);

/* Close the reader */
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_ReaderClose(void);

/* Suspend the reader, without destroying the handle to access it */
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_ReaderDeactivate(void);

/* Resume the reader */
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_ReaderActivate(void);

/* Discover the reader on a previously opened communication port */
#ifdef WIN32
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_ReaderAttachSerial(HANDLE hComm);
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_ReaderAttachUSB(HANDLE hComm);
#endif

/* Enumerate the compliant devices found on USB */
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_EnumUSBDevices(DWORD idx, TCHAR device[64], TCHAR description[64]);
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_EnumUSBDevicesW(DWORD idx, wchar_t device[64], wchar_t description[64]);
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_EnumUSBDevicesA(DWORD idx, char device[64], char description[64]);

/* Select the address (RS-485 bus mode only) */
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_ReaderSelectAddress(BYTE address);

/* Reader information functions */
/* ---------------------------- */

/* Retrieve name of the communication port */
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_ReaderGetDevice(TCHAR device[], WORD len);
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_ReaderGetDeviceW(wchar_t device[], WORD len);
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_ReaderGetDeviceA(char device[], WORD len);

/* Retrieve reader's firmware (type - version) */
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_ReaderGetFirmware(TCHAR firmware[], WORD len);
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_ReaderGetFirmwareW(wchar_t firmware[], WORD len);
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_ReaderGetFirmwareA(char firmware[], WORD len);

/* Retrieve actual features of reader's firmware */
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_ReaderGetFeatures(DWORD *features);

/* Retrieve communication settings between host and reader */
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_ReaderGetDeviceSettings(DWORD *settings);
/* Select communication settings between host and reader */
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_ReaderSetDeviceSettings(DWORD settings);

/* Bitmasks for SPROX_ReaderGetDeviceSettings and SPROX_ReaderSetDeviceSettings */
#define SPROX_SETTINGS_PROTOCOL_MASK       0x00000003
#define SPROX_SETTINGS_PROTOCOL_OSI        0x00000000
#define SPROX_SETTINGS_PROTOCOL_ASCII      0x00000001
#define SPROX_SETTINGS_PROTOCOL_BIN        0x00000002
#define SPROX_SETTINGS_PROTOCOL_BUS        0x00000003
#define SPROX_SETTINGS_HARDWARE_CTRL       0x00000004
#define SPROX_SETTINGS_BAUDRATE_MASK       0x00000008
#define SPROX_SETTINGS_BAUDRATE_38400      0x00000000
#define SPROX_SETTINGS_BAUDRATE_115200     0x00000008
#define SPROX_SETTINGS_CHANNEL_MASK        0x00000060
#define SPROX_SETTINGS_CHANNEL_RS232       0x00000000
#define SPROX_SETTINGS_CHANNEL_RS485       0x00000020
#define SPROX_SETTINGS_CHANNEL_USB         0x00000040
#define SPROX_SETTINGS_CHANNEL_TCP         0x00000060
#define SPROX_SETTINGS_FORCE_CHANNEL_RS485 0x00000020
#define SPROX_SETTINGS_FORCE_BAUDRATE      0x00000010
#define SPROX_SETTINGS_FORCE_PROTOCOL      0x00000004

/* Read configuration constants of the reader */
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_ReaderGetConsts(DWORD * consts);
/* Write configuration constants of the reader */
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_ReaderSetConsts(DWORD consts);

/* Miscellaneous reader functions */
/* ------------------------------ */

/* Reset the reader */
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_ReaderReset(void);

/* Drive the LEDs */
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_ControlLed(BYTE led_r, BYTE led_g);
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_ControlLedY(BYTE led_r, BYTE led_g, BYTE led_y);

/* Drive or read the USER pin */
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_ControlUserIO(BOOL is_output, BOOL out_value, BOOL *in_value);

/* Drive the RF field */
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_ControlRF(BOOL mode);

/* Configure the reader for ISO/IEC 14443-A or 14443-B operation */
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_SetConfig(BYTE mode);
#define CFG_MODE_ISO_14443_A 0x01
#define CFG_MODE_ISO_14443_B 0x02

/* Retrieve RF chipset's information (Philips RC type and serial number) */
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_ReaderGetRc500Id(BYTE rc500_type[5], BYTE rc500_snr[4]);

/* Send a raw control command to the reader */
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_ControlEx(const BYTE send_buffer[], WORD send_bytelen, BYTE recv_buffer[], WORD *recv_bytelen);

/* Send a raw command to the reader */
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_Function(BYTE cmd, const BYTE send_buffer[], WORD send_bytelen, BYTE recv_buffer[], WORD *recv_bytelen);

/* Test communication with the reader */
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_Echo(WORD len);

/* ISO/IEC 14443-A functions */
/* ------------------------- */

/* Layer 3 */
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_A_Request(BYTE req_code, BYTE atq[2]);
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_A_RequestAny(BYTE atq[2]);
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_A_RequestIdle(BYTE atq[2]);
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_A_SelectAny(BYTE atq[2], BYTE snr[12], BYTE *snrlen, BYTE sak[1]);
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_A_SelectIdle(BYTE atq[2], BYTE snr[12], BYTE *snrlen, BYTE sak[1]);
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_A_SelectAgain(const BYTE snr[12], BYTE snrlen, BYTE sak[1]);
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_A_Halt(void);
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_A_Exchange(const BYTE send_buffer[], WORD send_bytelen, BYTE recv_buffer[], WORD *recv_bytelen, BYTE append_crc, WORD timeout);
SPRINGPROX_LIB WORD SPRINGPROX_API  SPROX_ComputeIso14443ACrc(BYTE crc[2], const BYTE buffer[], WORD size);

/* Layer 4 (T=CL) */
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_TclA_ActivateAny(BYTE atq[2], BYTE snr[12], BYTE *snrlen, BYTE sak[1]);
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_TclA_ActivateIdle(BYTE atq[2], BYTE snr[12], BYTE *snrlen, BYTE sak[1]);
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_TclA_ActivateAgain(const BYTE snr[12], BYTE snrlen);
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_TclA_Halt(void);
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_TclA_Deselect(BYTE cid);
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_TclA_GetAts(BYTE cid, BYTE ats[32], BYTE *atslen);
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_TclA_Pps(BYTE cid, BYTE dsi, BYTE dri);
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_TclA_Exchange(BYTE fsci, BYTE cid, BYTE nad, const BYTE send_buffer[], WORD send_len, BYTE recv_buffer[], WORD *recv_len);

/* ISO/IEC 14443-B functions */
/* ------------------------- */

/* Layer 3 */
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_B_SelectAny(BYTE afi, BYTE *atq);
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_B_SelectIdle(BYTE afi, BYTE *atq);
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_B_AnticollAny(BYTE slots, BYTE afi, BYTE atq[11]);
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_B_AnticollIdle(BYTE slots, BYTE afi, BYTE atq[11]);
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_B_AnticollSlot(BYTE slot, BYTE atq[11]);
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_B_Exchange(const BYTE send_buffer[], WORD send_bytelen, BYTE recv_buffer[], WORD *recv_bytelen, BYTE append_crc, WORD timeout);
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_B_Halt(const BYTE pupi[4]);
SPRINGPROX_LIB WORD SPRINGPROX_API  SPROX_ComputeIso14443BCrc(BYTE crc[2], const BYTE buffer[], WORD size);

/* Layer 4 (T=CL) */
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_TclB_ActivateAny(BYTE afi, BYTE atq[11]);
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_TclB_ActivateIdle(BYTE afi, BYTE atq[11]);
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_TclB_Attrib(const BYTE atq[11], BYTE cid);
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_TclB_AttribEx(const BYTE atq[11], BYTE cid, BYTE dsi, BYTE dri);
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_TclB_Deselect(BYTE cid);
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_TclB_Halt(const BYTE pupi[4]);
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_TclB_Exchange(BYTE fsci, BYTE cid, BYTE nad, const BYTE send_buffer[], WORD send_len, BYTE recv_buffer[], WORD *recv_len);

/* ISO/IEC 14443 type independant functions */
/* ------------------------------------------ */

SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_Tcl_Exchange(BYTE cid, const BYTE send_buffer[], WORD send_len, BYTE recv_buffer[], WORD *recv_len);
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_Tcl_Deselect(BYTE cid);


/* Legacy Mifare functions */
/* ----------------------- */

/* Mifare is a registered trademark of Philips.                            */
/* Please refer to Philips documentation for any explanation on this part. */

SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_MifStSelectAny(BYTE snr[12], BYTE atq[2], BYTE sak[1]);
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_MifStSelectIdle(BYTE snr[12], BYTE atq[2], BYTE sak[1]);
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_MifStSelectAgain(const BYTE snr[4]);
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_MifStHalt(void);

/* Mifare read, without authentication (Mifare UltraLight cards) */
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_MifRead(const BYTE snr[4], BYTE addr, BYTE data[16]);

/* Mifare write, without authentication (Mifare UltraLight cards) */
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_MifWrite(const BYTE snr[4], BYTE addr, const BYTE data[16]);
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_MifWrite4(const BYTE snr[4], BYTE addr, const BYTE data[4]);

/* Mifare standard authenticate and read */
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_MifStReadBlock(const BYTE snr[4], BYTE bloc, BYTE data[16], const BYTE key[6]);
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_MifStReadSector(const BYTE snr[4], BYTE sect, BYTE data[240], const BYTE key[6]);
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_MifStReadTag768(const BYTE snr[4], WORD *sectors, BYTE data[768]);

/* Mifare standard authenticate and write */
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_MifStWriteBlock(const BYTE snr[4], BYTE bloc, const BYTE data[16], const BYTE key[6]);
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_MifStWriteSector(const BYTE snr[4], BYTE sect, const BYTE data[240], const BYTE key[6]);
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_MifStWriteTag768(const BYTE snr[4], WORD *sectors, const BYTE data[768]);

/* Mifare standard counter manipulation */
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_MifStReadCounter(const BYTE snr[4], BYTE bloc, SDWORD * value, const BYTE key[6]);
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_MifStWriteCounter(const BYTE snr[4], BYTE bloc, SDWORD value, const BYTE key[6]);
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_MifStDecrementCounter(const BYTE snr[4], BYTE bloc, DWORD value, const BYTE key[6]);

/* Mifare standard sector trailers formatting */
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_MifStUpdateAccessBlock(const BYTE snr[4], BYTE sect, const BYTE old_key[6], const BYTE new_key_A[6], const BYTE new_key_B[6], BYTE ac0, BYTE ac1, BYTE ac2, BYTE ac3);
/* Valid access conditions for ac0, ac1 and ac2 */
#define ACC_BLOCK_VALUE     0x04
#define ACC_BLOCK_COUNT     0x06
/* Valid access conditions for ac3 */
#define ACC_AUTH_NORMAL     0x03
#define ACC_AUTH_TRANSPORT  0x01

/* Perform Mifare standard authentication */
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_MifStAuthKey(BYTE auth_mode, const BYTE snr[4], const BYTE key[6], BYTE block);
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_MifStAuthE2(BYTE auth_mode, const BYTE snr[4], BYTE key_sector, BYTE block);
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_MifStAuthRam(BYTE auth_mode, BYTE key_sector, BYTE block);

/* Load a Mifare standard key into reader's memory (RAM or EEPROM) */
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_MifLoadKey(BOOL eeprom, BYTE key_type, BYTE key_index, const BYTE key_data[6]);

/* Who is the last authentication key successfully used ? */
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_MifLastAuthKey(BYTE * info);
/* Related defines */
#define MIF_RAM_KEY     0x80
#define MIF_E2_KEY      0x40
#define MIF_CODED_KEY   0xC0
#define MIF_KEY_A       0x00
#define MIF_KEY_B       0x20

/* Smartcard related functions (readers with embedded GemPlus GemCore smartcard reader) */
/* ------------------------------------------------------------------------------------ */

/* Power up the card in a slot */
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_Card_PowerUp(BYTE slot, BYTE config, BYTE atr[32], WORD *atr_len);
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_Card_PowerUp_Auto(BYTE slot, BYTE atr[32], WORD *atr_len);

/* Power down the card in a slot */
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_Card_PowerDown(BYTE slot);

/* Exchange with the card in a slot */
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_Card_Exchange(BYTE slot, const BYTE send_buffer[], WORD send_len, BYTE recv_buffer[], WORD *recv_len);

/* Get status of a slot */
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_Card_Status(BYTE slot, BYTE *stat, BYTE *type, BYTE config[4]);

/* Configure / retrieve actual configuration of a slot */
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_Card_SetConfig(BYTE slot, BYTE mode, BYTE type);
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_Card_GetConfig(BYTE slot, BYTE *mode, BYTE *type);

/* Retrieve smartcard coupler's firmware (type - version) */
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_Card_GetFirmware(TCHAR firmware[], WORD len);
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_Card_GetFirmwareW(wchar_t firmware[], WORD len);
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_Card_GetFirmwareA(char firmware[], WORD len);

/* Transparent exchange between host and smartcard coupler */
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_Card_Control(const BYTE send_buffer[], WORD send_len, BYTE recv_buffer[], WORD *recv_len);

/* Fingerprint related functions (reader with embedded Sagem MorphoSmart CBM module) */
/* --------------------------------------------------------------------------------- */

/* Power up the MorphoSmart */
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_MSO_Open(TCHAR *mso_product, TCHAR *mso_firmware, TCHAR *mso_sensor);
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_MSO_OpenW(wchar_t *mso_product, wchar_t *mso_firmware, wchar_t *mso_sensor);
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_MSO_OpenA(char *mso_product, char *mso_firmware, char *mso_sensor);

/* Power down the MorphoSmart */
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_MSO_Close(void);

/* Transparent exchange between host and MorphoSmart */
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_MSO_Exchange(const BYTE send_buffer[], WORD send_len, BYTE recv_buffer[], WORD *recv_len, DWORD timeout, BYTE *async);

/* DES and Triple-DES cryptography */
/* ------------------------------- */
typedef struct
{
  BYTE opaque[768];
} SPROX_DES_3DES_CTX_ST;

SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_3DES_Init(SPROX_DES_3DES_CTX_ST *context, const BYTE key1[8], const BYTE key2[8], const BYTE key3[8]);
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_3DES_Encrypt(SPROX_DES_3DES_CTX_ST *context, BYTE outbuf[8], const BYTE inbuf[8]);
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_3DES_Decrypt(SPROX_DES_3DES_CTX_ST *context, BYTE outbuf[8], const BYTE inbuf[8]);
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_DES_Init(SPROX_DES_3DES_CTX_ST *context, const BYTE key[8]);
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_DES_Encrypt(SPROX_DES_3DES_CTX_ST *context, BYTE outbuf[8], const BYTE inbuf[8]);
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_DES_Decrypt(SPROX_DES_3DES_CTX_ST *context, BYTE outbuf[8], const BYTE inbuf[8]);

/* MD5 hash */
/* -------- */
typedef struct
{
  BYTE opaque[88];
} SPROX_MD5_CTX_ST;

SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_MD5_Init(SPROX_MD5_CTX_ST *context);
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_MD5_Update(SPROX_MD5_CTX_ST *context, const BYTE *inbuf, DWORD size);
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_MD5_Final(SPROX_MD5_CTX_ST *context, BYTE digest[16]);

/* Miscelleanous utilities, helpers for VB or Delphi users */
/* ------------------------------------------------------- */

SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_Malloc(BYTE **buffer, WORD size);
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_Free(BYTE * buffer);
SPRINGPROX_LIB WORD  SPRINGPROX_API SPROX_StrLen(const TCHAR *buffer);
SPRINGPROX_LIB WORD  SPRINGPROX_API SPROX_StrLenW(const wchar_t *buffer);
SPRINGPROX_LIB WORD  SPRINGPROX_API SPROX_StrLenA(const char *buffer);
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_ArrayToString(TCHAR *string, const BYTE *buffer, WORD size);
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_ArrayToStringW(wchar_t *string, const BYTE *buffer, WORD size);
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_ArrayToStringA(char *string, const BYTE *buffer, WORD size);
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_StringToArray(BYTE *buffer, const TCHAR *string, WORD size);
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_StringToArrayW(BYTE *buffer, const wchar_t *string, WORD size);
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_StringToArrayA(BYTE *buffer, const char *string, WORD size);

/* Philips basic function set */
/* -------------------------- */

/* Those functions are copied from Philips "Pegoda" low-level API */
/* For better performance, DO NOT USE those low level calls.      */ 

/* Modes for 'Mf500InterfaceOpen' */
#define USB         0x00000031
#define RS232       0x00000040
SPRINGPROX_LIB SWORD SPRINGPROX_API Mf500InterfaceOpen(DWORD mode, DWORD unused);
SPRINGPROX_LIB SWORD SPRINGPROX_API Mf500InterfaceClose(void);

SPRINGPROX_LIB SWORD SPRINGPROX_API Mf500PcdConfig(void);
SPRINGPROX_LIB SWORD SPRINGPROX_API Mf500PcdSetDefaultAttrib(void);
SPRINGPROX_LIB SWORD SPRINGPROX_API Mf500PcdSetAttrib(BYTE DSI, BYTE DRI);
SPRINGPROX_LIB SWORD SPRINGPROX_API Mf500PcdGetAttrib(BYTE *FSCImax, BYTE *FSDImax, BYTE *DSsupp, BYTE *DRsupp, BYTE *DREQDS);

SPRINGPROX_LIB SWORD SPRINGPROX_API Mf500PiccRequest(BYTE req_code, BYTE *atq);
SPRINGPROX_LIB SWORD SPRINGPROX_API Mf500PiccCommonRequest(BYTE req_code, BYTE *atq);
SPRINGPROX_LIB SWORD SPRINGPROX_API Mf500PiccAnticoll(BYTE bcnt, BYTE *snr);
SPRINGPROX_LIB SWORD SPRINGPROX_API Mf500PiccCascAnticoll(BYTE select_code, BYTE bcnt, BYTE *snr);
SPRINGPROX_LIB SWORD SPRINGPROX_API Mf500PiccSelect(const BYTE *snr, BYTE *sak);
SPRINGPROX_LIB SWORD SPRINGPROX_API Mf500PiccCascSelect(BYTE sel_code, const BYTE *snr, BYTE *sak);
SPRINGPROX_LIB SWORD SPRINGPROX_API Mf500PiccActivateWakeup(BYTE br, BYTE *atq, BYTE *sak, const BYTE *uid, BYTE uid_len);
SPRINGPROX_LIB SWORD SPRINGPROX_API Mf500PiccActivateIdle(BYTE br, BYTE *atq, BYTE *sak, BYTE *uid, BYTE *uid_len);
SPRINGPROX_LIB SWORD SPRINGPROX_API Mf500PiccAuth(BYTE auth_mode, BYTE key_sector, BYTE block);
SPRINGPROX_LIB SWORD SPRINGPROX_API Mf500PiccAuthRam(BYTE auth_mode, BYTE key_sector, BYTE block);
SPRINGPROX_LIB SWORD SPRINGPROX_API Mf500PiccAuthE2(BYTE auth_mode, BYTE *snr, BYTE key_sector, BYTE block);
SPRINGPROX_LIB SWORD SPRINGPROX_API Mf500PiccAuthKey(BYTE auth_mode, const BYTE *snr, BYTE *keys, BYTE block);
SPRINGPROX_LIB SWORD SPRINGPROX_API Mf500PiccRead(BYTE addr, BYTE *data);
SPRINGPROX_LIB SWORD SPRINGPROX_API Mf500PiccCommonRead(BYTE cmd, BYTE addr, BYTE datalen, BYTE *data);
SPRINGPROX_LIB SWORD SPRINGPROX_API Mf500PiccWrite(BYTE addr, BYTE *data);
SPRINGPROX_LIB SWORD SPRINGPROX_API Mf500PiccWrite4(BYTE addr, BYTE *data);
SPRINGPROX_LIB SWORD SPRINGPROX_API Mf500PiccCommonWrite(BYTE cmd, BYTE addr, BYTE datalen, BYTE *data);
SPRINGPROX_LIB SWORD SPRINGPROX_API Mf500PiccValue(BYTE dd_mode, BYTE addr, BYTE *value, BYTE trans_addr);
SPRINGPROX_LIB SWORD SPRINGPROX_API Mf500PiccValueDebit(BYTE dd_mode, BYTE addr, BYTE *value);
SPRINGPROX_LIB SWORD SPRINGPROX_API Mf500PiccHalt(void);
SPRINGPROX_LIB SWORD SPRINGPROX_API Mf500PiccExchangeBlock(BYTE *send_data, WORD send_bytelen, BYTE *rec_data, WORD *rec_bytelen, BYTE append_crc, DWORD timeout);

SPRINGPROX_LIB SWORD SPRINGPROX_API Mf500HostCodeKey(BYTE *uncoded, BYTE *coded);
SPRINGPROX_LIB SWORD SPRINGPROX_API Mf500PcdLoadKeyE2(BYTE key_type, BYTE sector, BYTE *uncoded_keys);
SPRINGPROX_LIB SWORD SPRINGPROX_API Mf500PcdLoadKeyRam(BYTE key_type, BYTE sector, BYTE *uncoded_keys);

SPRINGPROX_LIB SWORD SPRINGPROX_API Mf500PiccExchangeBlock_A(BYTE *send_data, WORD send_bytelen, BYTE *rec_data, WORD *rec_bytelen, BYTE append_crc, WORD timeout);
SPRINGPROX_LIB SWORD SPRINGPROX_API Mf500PiccExchangeBlock_B(BYTE *send_data, WORD send_bytelen, BYTE *rec_data, WORD *rec_bytelen, BYTE append_crc, WORD timeout);

SPRINGPROX_LIB SWORD SPRINGPROX_API PcdSetTmo(DWORD tmoLength);
SPRINGPROX_LIB SWORD SPRINGPROX_API PcdGetSnr(BYTE *snr);
SPRINGPROX_LIB SWORD SPRINGPROX_API PcdReadE2(WORD startaddr, BYTE length, BYTE *data);
SPRINGPROX_LIB SWORD SPRINGPROX_API PcdWriteE2(WORD startaddr, BYTE length, BYTE *data);
SPRINGPROX_LIB SWORD SPRINGPROX_API PcdReset(void);

SPRINGPROX_LIB SWORD SPRINGPROX_API PcdSetIdleMode(void);
SPRINGPROX_LIB SWORD SPRINGPROX_API PcdClearIdleMode(void);

SPRINGPROX_LIB SWORD SPRINGPROX_API ExchangeByteStream(BYTE cmd, BYTE *send_data, WORD send_bytelen, BYTE *rec_data, WORD *rec_bytelen);
SPRINGPROX_LIB SWORD SPRINGPROX_API ReadRIC(BYTE reg, BYTE *value);
SPRINGPROX_LIB SWORD SPRINGPROX_API WriteRIC(BYTE reg, BYTE value);
SPRINGPROX_LIB SWORD SPRINGPROX_API SetRCBitMask(BYTE reg, BYTE mask);
SPRINGPROX_LIB SWORD SPRINGPROX_API ClearRCBitMask(BYTE reg, BYTE mask);

#ifdef __cplusplus
}
#endif

/* Error codes */
/* ----------- */

#define MI_OK                           (0)

#define MI_NOTAGERR                     (-1)
#define MI_CRCERR                       (-2)
#define MI_EMPTY                        (-3)
#define MI_AUTHERR                      (-4)
#define MI_PARITYERR                    (-5)
#define MI_CODEERR                      (-6)

#define MI_SERNRERR                     (-8)
#define MI_KEYERR                       (-9)
#define MI_NOTAUTHERR                   (-10)
#define MI_BITCOUNTERR                  (-11)
#define MI_BYTECOUNTERR                 (-12)
#define MI_IDLE                         (-13)
#define MI_TRANSERR                     (-14)
#define MI_WRITEERR                     (-15)
#define MI_INCRERR                      (-16)
#define MI_DECRERR                      (-17)
#define MI_READERR                      (-18)
#define MI_OVFLERR                      (-19)
#define MI_POLLING                      (-20)
#define MI_FRAMINGERR                   (-21)
#define MI_ACCESSERR                    (-22)
#define MI_UNKNOWN_COMMAND              (-23)
#define MI_COLLERR                      (-24)
#define MI_RESETERR                     (-25)
#define MI_INITERR                      (-25)
#define MI_INTERFACEERR                 (-26)
#define MI_ACCESSTIMEOUT                (-27)
#define MI_NOBITWISEANTICOLL            (-28)
#define MI_QUIT                         (-30)
#define MI_CODINGERR                    (-31)
#define MI_SENDBYTENR                   (-51)
#define MI_CASCLEVEX                    (-52)
#define MI_SENDBUF_OVERFLOW             (-53)
#define MI_BAUDRATE_NOT_SUPPORTED       (-54)
#define MI_SAME_BAUDRATE_REQUIRED       (-55)

#define MI_WRONG_PARAMETER_VALUE        (-60)

/* MV EV700 error codes */
#define MI_BREAK                        (-99)
#define MI_NY_IMPLEMENTED               (-100)
#define MI_NO_MFRC                      (-101)
#define MI_MFRC_NOTAUTH                 (-102)
#define MI_WRONG_DES_MODE               (-103)
#define MI_HOST_AUTH_FAILED             (-104)

#define MI_WRONG_LOAD_MODE              (-106)
#define MI_WRONG_DESKEY                 (-107)
#define MI_MKLOAD_FAILED                (-108)
#define MI_FIFOERR                      (-109)
#define MI_WRONG_ADDR                   (-110)
#define MI_DESKEYLOAD_FAILED            (-111)
#define MI_RECBUF_OVERFLOW              (-112)
#define MI_WRONG_SEL_CNT                (-114)

#define MI_WRONG_TEST_MODE              (-117)
#define MI_TEST_FAILED                  (-118)
#define MI_TOC_ERROR                    (-119)
#define MI_COMM_ABORT                   (-120)
#define MI_INVALID_BASE                 (-121)
#define MI_MFRC_RESET                   (-122)
#define MI_WRONG_VALUE                  (-123)
#define MI_VALERR                       (-124)
#define MI_WRONG_LENGTH                 (-125)

/* Not an error... */
#define MI_TIME_EXTENSION               (-128)

/* T=CL error codes */
#define TCL_OK                          (0)
#define TCL_NOTAGERR                    (-131)
#define TCL_CRCERR                      (-132)
#define TCL_PRITYERR                    (-133)
#define TCL_OTHERERR                    (-134)
#define TCL_SERNRERR                    (-135)
#define TCL_BITCOUNTERR                 (-136)
#define TCL_POLLING                     (-137)
#define TCL_RF_CHANNEL                  (-138)
#define TCL_MULTACT_DISABLED            (-139)
#define TCL_MULTACT_ENABLED             (-140)
#define TCL_CID_NOT_ACTIVE              (-141)
#define TCL_BITANTICOLL                 (-142)
#define TCL_UIDLEN                      (-143)
#define TCL_CIDINVALID                  (-144)
#define TCL_ATSLEN                      (-145)
#define TCL_NO_ATS_AVAILABLE            (-146)
#define TCL_ATS_ERROR                   (-147)
#define TCL_FATAL_PROTOCOL              (-148)
#define TCL_RECBUF_OVERFLOW             (-149)
#define TCL_SENDBYTENR                  (-150)
#define TCL_TRANSMERR_HALTED            (-151)
#define TCL_TRANSMERR_NOTAG             (-152)
#define TCL_BAUDRATE_NOT_SUPPORTED_PICC (-153)
#define TCL_CID_NOT_SUPPORTED           (-154)
#define TCL_NAD_NOT_SUPPORTED           (-155)
#define TCL_PROTOCOL_NOT_SUPPORTED      (-156)
#define TCL_PPS_FORMAT                  (-157)
#define TCL_ERROR                       (-158)
#define TCL_NADINVALID                  (-159)
#define TCL_OTHER_ERR                   (-161)
#define TCL_BAUDRATE_NOT_SUPPORTED_PCD  (-162)
#define TCL_CID_ACTIVE                  (-163)

/* Pro-Active specific */
#define MI_FUNCTION_NOT_AVAILABLE       (-240)
#define MI_SER_LENGTH_ERR               (-241)
#define MI_SER_CHECKSUM_ERR             (-242)
#define MI_SER_PROTO_ERR                (-243)
#define MI_SER_PROTO_NAK                (-244)
#define MI_SER_ACCESS_ERR               (-245)
#define MI_SER_TIMEOUT_ERR              (-246)
#define MI_SER_NORESP_ERR               (-247)
#define MI_LIB_CALL_ERROR               (-248)
#define MI_OUT_OF_MEMORY_ERROR          (-249)
#define MI_FILE_ACCESS_ERROR            (-250)

#define MI_READER_ACTIVE_ERROR          (-251)
#define MI_INVALID_READER_CONTEXT       (-252)

#endif
