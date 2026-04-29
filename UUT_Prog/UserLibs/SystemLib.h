/*
 * SystemLib.h
 *
 *  Created on: 18 Apr 2026
 *      Author: boris
 */

#ifndef ____SYSTEMLIB_H__
#define ____SYSTEMLIB_H__


typedef enum ResetReason_e
 {
  E_POWER_ON_RESET,
  E_BROWN_OUT_RESET,
  E_WATCHDOC_RESET,
  E_SOFTWARE_RESET,
  E_NTRS_PIN_RESET,
  // ...
  E_NUM_RESET_FLAGS
 }ResetReason_e;



void InitRTPrintf();

int	rtprintf (const char *format, ...);

void AdjustIntVectTable(void);  /* Adjusts Interrupt Vector Table MCU pointer according to code location in flash memory. */

ResetReason_e CheckResetReason();


#endif /* ____SYSTEMLIB_H__ */
