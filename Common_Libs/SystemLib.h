/*
 * SystemLib.h
 *
 *  Created on: 18 Apr 2026
 *      Author: boris
 */

#ifndef ____SYSTEMLIB_H__
#define ____SYSTEMLIB_H__

#include "stdint.h"

/*
 * Sectors in flash definitions:
 * See page 79 in "rm0385-stm32f75xxx-and-stm32f74xxx-advanced-armbased-32bit-mcus-stmicroelectronics.pdf" file.
 *
 * 
 *                    Table 3. STM32F756xx and STM32F74xxx Flash memory organization
 *       ┏━━━━━━━━━━━━━━━━━━━━┳━━━━━━━━━━━━━━━━━━━━━━┳━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┳━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┳━━━━━━━━━━━━━━━━━━━┓
 *       ┃      Block         ┃        Name          ┃    Block base address on     ┃    Block base address on    ┃     Sector size   ┃
 *       ┃                    ┃                      ┃       AXIM interface         ┃       ICTM interface        ┃                   ┃
 *       ┣━━━━━━━━━━━━━━━━━━━━╋━━━━━━━━━━━━━━━━━━━━━━╋━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━╋━━━━━━━━━━━━━━━━━━━━━━━━━━━━━╋━━━━━━━━━━━━━━━━━━━┫
 *       ┃                    ┃      Sector 0        ┃  0x0800 0000 - 0x0800 7FFF   ┃  0x0020 0000 - 0x0020 7FFF  ┃     32 Kbytes     ┃
 *       ┃                    ┣━━━━━━━━━━━━━━━━━━━━━━╋━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━╋━━━━━━━━━━━━━━━━━━━━━━━━━━━━━╋━━━━━━━━━━━━━━━━━━━┫
 *       ┃                    ┃      Sector 1        ┃  0x0800 8000 - 0x0800 FFFF   ┃  0x0020 8000 - 0x0020 FFFF  ┃     32 Kbytes     ┃
 *       ┃                    ┣━━━━━━━━━━━━━━━━━━━━━━╋━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━╋━━━━━━━━━━━━━━━━━━━━━━━━━━━━━╋━━━━━━━━━━━━━━━━━━━┫
 *       ┃                    ┃      Sector 2        ┃  0x0801 0000 - 0x0801 7FFF   ┃  0x0021 0000 - 0x0021 7FFF  ┃     32 Kbytes     ┃
 *       ┃                    ┣━━━━━━━━━━━━━━━━━━━━━━╋━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━╋━━━━━━━━━━━━━━━━━━━━━━━━━━━━━╋━━━━━━━━━━━━━━━━━━━┫
 *       ┃    Main Memory     ┃      Sector 3        ┃  0x0801 8000 - 0x0801 FFFF   ┃  0x0021 8000 - 0x0021 FFFF  ┃     32 Kbytes     ┃
 *       ┃       block        ┣━━━━━━━━━━━━━━━━━━━━━━╋━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━╋━━━━━━━━━━━━━━━━━━━━━━━━━━━━━╋━━━━━━━━━━━━━━━━━━━┫
 *       ┃                    ┃      Sector 4        ┃  0x0802 0000 - 0x0803 FFFF   ┃  0x0022 0000 - 0x0023 FFFF  ┃    128 Kbytes     ┃
 *       ┃                    ┣━━━━━━━━━━━━━━━━━━━━━━╋━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━╋━━━━━━━━━━━━━━━━━━━━━━━━━━━━━╋━━━━━━━━━━━━━━━━━━━┫
 *       ┃                    ┃      Sector 5        ┃  0x0804 0000 - 0x0807 FFFF   ┃  0x0024 0000 - 0x0027 FFFF  ┃    256 Kbytes     ┃
 *       ┃                    ┣━━━━━━━━━━━━━━━━━━━━━━╋━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━╋━━━━━━━━━━━━━━━━━━━━━━━━━━━━━╋━━━━━━━━━━━━━━━━━━━┫
 *       ┃                    ┃      Sector 6        ┃  0x0808 0000 - 0x080B FFFF   ┃  0x0028 0000 - 0x002B FFFF  ┃    256 Kbytes     ┃
 *       ┃                    ┣━━━━━━━━━━━━━━━━━━━━━━╋━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━╋━━━━━━━━━━━━━━━━━━━━━━━━━━━━━╋━━━━━━━━━━━━━━━━━━━┫
 *       ┃                    ┃      Sector 7        ┃  0x080C 0000 - 0x080F FFFF   ┃  0x002C 0000 - 0x002F FFFF  ┃    256 Kbytes     ┃
 *       ┣━━━━━━━━━━━━━━━━━━━━╋━━━━━━━━━━━━━━━━━━━━━━╋━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━╋━━━━━━━━━━━━━━━━━━━━━━━━━━━━━╋━━━━━━━━━━━━━━━━━━━┫
 *       ┃                    ┃   System memory      ┃  0x1FF0 0000 - 0x1FF0 EDBF   ┃  0x0010 0000 - 0x0010 EDBF  ┃     60 Kbytes     ┃
 *       ┃                    ┣━━━━━━━━━━━━━━━━━━━━━━╋━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━╋━━━━━━━━━━━━━━━━━━━━━━━━━━━━━╋━━━━━━━━━━━━━━━━━━━┫
 *       ┃ Information block  ┃        OTP           ┃  0x1FF0 F000 - 0x1FF0 F41F   ┃  0x0010 F000 - 0x0010 F41F  ┃    1024 bytes     ┃
 *       ┃                    ┣━━━━━━━━━━━━━━━━━━━━━━╋━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━╋━━━━━━━━━━━━━━━━━━━━━━━━━━━━━╋━━━━━━━━━━━━━━━━━━━┫
 *       ┃                    ┃     Option bytes     ┃  0x1FFF 0000 - 0x1FFF 001F   ┃              -              ┃      32 bytes     ┃
 *       ┗━━━━━━━━━━━━━━━━━━━━┻━━━━━━━━━━━━━━━━━━━━━━┻━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┻━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┻━━━━━━━━━━━━━━━━━━━┛
 *                    
 *                       
 *                    Table 4. STM32F750xx Flash memory organization
 *       ┏━━━━━━━━━━━━━━━━━━━━┳━━━━━━━━━━━━━━━━━━━━━━┳━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┳━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┳━━━━━━━━━━━━━━━━━━━┓
 *       ┃      Block         ┃        Name          ┃    Block base address on     ┃    Block base address on    ┃     Sector size   ┃
 *       ┃                    ┃                      ┃       AXIM interface         ┃       ICTM interface        ┃                   ┃
 *       ┣━━━━━━━━━━━━━━━━━━━━╋━━━━━━━━━━━━━━━━━━━━━━╋━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━╋━━━━━━━━━━━━━━━━━━━━━━━━━━━━━╋━━━━━━━━━━━━━━━━━━━┫
 *       ┃    Main Memory     ┃      Sector 0        ┃  0x0800 0000 - 0x0800 7FFF   ┃  0x0020 0000 - 0x0020 7FFF  ┃     32 Kbytes     ┃
 *       ┃       block        ┣━━━━━━━━━━━━━━━━━━━━━━╋━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━╋━━━━━━━━━━━━━━━━━━━━━━━━━━━━━╋━━━━━━━━━━━━━━━━━━━┫
 *       ┃                    ┃      Sector 1        ┃  0x0800 8000 - 0x0800 FFFF   ┃  0x0020 8000 - 0x0020 FFFF  ┃     32 Kbytes     ┃
 *       ┣━━━━━━━━━━━━━━━━━━━━╋━━━━━━━━━━━━━━━━━━━━━━╋━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━╋━━━━━━━━━━━━━━━━━━━━━━━━━━━━━╋━━━━━━━━━━━━━━━━━━━┫
 *       ┃                    ┃   System memory      ┃  0x1FF0 0000 - 0x1FF0 EDBF   ┃  0x0010 0000 - 0x0010 EDBF  ┃     60 Kbytes     ┃
 *       ┃                    ┣━━━━━━━━━━━━━━━━━━━━━━╋━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━╋━━━━━━━━━━━━━━━━━━━━━━━━━━━━━╋━━━━━━━━━━━━━━━━━━━┫
 *       ┃ Information block  ┃        OTP           ┃  0x1FF0 F000 - 0x1FF0 F41F   ┃  0x0010 F000 - 0x0010 F41F  ┃    1024 bytes     ┃
 *       ┃                    ┣━━━━━━━━━━━━━━━━━━━━━━╋━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━╋━━━━━━━━━━━━━━━━━━━━━━━━━━━━━╋━━━━━━━━━━━━━━━━━━━┫
 *       ┃                    ┃     Option bytes     ┃  0x1FFF 0000 - 0x1FFF 001F   ┃              -              ┃      32 bytes     ┃
 *       ┗━━━━━━━━━━━━━━━━━━━━┻━━━━━━━━━━━━━━━━━━━━━━┻━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┻━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┻━━━━━━━━━━━━━━━━━━━┛
 *                       
 */


/*======================================================================================================================*/

#define FLASH_CONFIG_SECTOR  (FLASH_SECTOR_TOTAL - 1)
#define START_PROG_SECTOR    FLASH_SECTOR_4

/*======================================================================================================================*/

typedef enum ResetReason_e
 {
  E_POWER_ON_RESET,    /* Power on reset*/
  E_BROWN_OUT_RESET,   /* Brown out reset (power supply undervoltage detected.) */
  E_IWATCHDOC_RESET,   /* Independent watch-dog reset (watchdog counter underrun)  */
  E_WWATCHDOC_RESET,   /* Window watch-dog reset adcwatchdog counter underrun */
  E_SOFTWARE_RESET,    /* Software reset: The reset was done by the command "NVIC_SystemReset()" in the software. */
  E_NRST_PIN_RESET,    /* NRTST Pin Reset: The reset was done by the hardware for example the Reset-Button connected to the NRST Pin was pressed. */
  E_LOWPOWER_RESET,    /* Low Power Reset. The reset was done when the MCU was in standby. (Rarely Usable)*/
  E_RCC_READY,         /* The LSI Oscillator is running and stable. Not a reset reason. */
  // ...
  E_NUM_RESET_FLAGS
 }ResetReason_e;


 /*======================================================================================================================*/


typedef struct SectorAddr_s
 {
  uint32_t Start;
  uint32_t End;
 }SectorAddr_s;


 extern SectorAddr_s const SectorsAddr[];


typedef struct BL_Conf_s
 {
  void *StartProgAddr;
  void *EndProgAddr;
  uint32_t ProgCRC;
 }
BL_Conf_s;


typedef struct ProgConf_s
 {
  uint32_t ConfLen;
 }
ProgConf_s;

typedef struct MemConf_s
 {
  uint32_t DataLen;
  BL_Conf_s BootLoaderConfig;
//  uint32_t ConfCRC;
//  ProgConf_s ProgramConfig;
 }
MemConf_s;



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
 * "const char *format, ..." these are the regular parameters of the variadic function
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


/*======================================================================================================================*/

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

/*======================================================================================================================*/

/**
 * @brief Loads the program configuration saved in the flash memory.
 *
 * @code
 * void LoadConf(void);
 * @code
 *
 */
void LoadConf(void);

/**
 * @brief Returns configuration saved in the flash memory.
 *
 * @code
 * void GetBLConf(BL_Conf_s *BlConf);
 * @code
 *
 * @param BlConf Pointer to memory where the configuration data will be copied.
 */
void GetBLConf(BL_Conf_s *BlConf);

/**
 * @brief Saves Configuration to the flash memory.
 *
 * @code
 * void SetBLConf(BL_Conf_s BlConf);
 * @code
 *
 * @param BlConf Configuration data that must be saved.
 */
void SetBLConf(BL_Conf_s BlConf);



#endif /* ____SYSTEMLIB_H__ */
