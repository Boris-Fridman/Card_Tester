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
  E_POWER_ON_RESET,    /* Power on reset*/
  E_BROWN_OUT_RESET,   /* Brown out reset (power supply undervoltage detected.) */
  E_IWATCHDOC_RESET,   /* Independent watch-dog reset (watchdog counter underrun)  */
  E_WWATCHDOC_RESET,   /* Window watch-dog reset adcwatchdog counter underrun */
  E_SOFTWARE_RESET,    /* Sortware reset: The reset was done by the command "NVIC_SystemReset()" in the software. */
  E_NRST_PIN_RESET,    /* NRTST Pin Reset: The reset was done by the hardware for example the Reset-Button connected to the NRST Pin was pressed. */
  E_LOWPOWER_RESET,    /* Low Power Reset. The reset was done when the MCU was in standby. (Rarely Usable)*/
  E_RCC_READY,         /* The LSI Oscilator is runnint and stable. Not a reset reason. */
  // ...
  E_NUM_RESET_FLAGS
 }ResetReason_e;


#ifdef USE_FREERTOS
/**
 *
 * @brief Initializes mutexes used by the function "rtprintf()".
 *
 * @code{c}
 * void InitRTPrintf();
 * @code
 */
void InitRTPrintf();

/**
 * @brief
 * The function is similar to "printf()" but it is using the internal mutex to prevent
 * print-mixing when two or more tasks try to print messages at the same time.
 * If it happens the first-running-copy of the function take the mutex and the
 * other copies of the function wait until the mutex is given back.
 *
 * @code{c}
 * int	rtprintf(const char *format, ...);
 * @code
 *
 * @param
 * "const char *format, ..." these are the regular parametes of the variadic function
 * the same ones that are used in the function "printf()"
 *
 * @return
 * If the value is success the function returns the number of printed characters.
 * In case of error the function returns a negative value.
 * The return is the same as in the "printf()" function.
 *
 */
int	rtprintf(const char *format, ...);
#endif

/**
 * @brief Adjusts Interrupt Vector Table MCU pointer according to code location in flash memory.
 *
 * @code{c}
 * void AdjustIntVectTable(void);
 * @code
 *
 */
void AdjustIntVectTable(void);




/**
 * @brief The function checks the reason of reset at startup.
 *
 * @code{c}
 * ResetReason_e CheckResetReason();
 * @code
 *
 * @return Returns the reason of reset that can be: power-on, brown-out (power supply under voltage), watchdog, nrst-pin ("reset button pressed") reset.
 *
 */
ResetReason_e CheckResetReason();



void LoadConf(void);


#endif /* ____SYSTEMLIB_H__ */
