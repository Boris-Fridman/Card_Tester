/*
 * SystemLib.c
 *
 *  Created on: 18 Apr 2026
 *      Author: boris
 */


#include "SystemLib.h"
#include "main.h"

#include "usart.h"
#include <stdarg.h>

#include "CommonData.h"

#ifdef USE_FREERTOS
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"
#include "semphr.h"
#include "FreeRTOSConfig.h"
#include "projdefs.h"
#endif

/*======================================================================================================================*/
#ifdef USE_FREERTOS
static SemaphoreHandle_t PrintfMut;

void InitRTPrintf()
 {
  PrintfMut = xSemaphoreCreateMutex();
  if(PrintfMut == NULL)
   {
    printf("\n%sThe printf-mutex couldn't be created.\n\rHalting !!!%s\n", TermRed, TermColorsReset);
    for(;;);
   }
  xSemaphoreGive(PrintfMut);
 }



int	vrtprintf(const char *format, va_list args)
 {
  int result;
  result = vprintf(format, args);
  return result;
 }

int	rtprintf(const char *format, ...)
 {
  int result;
  va_list args;
  xSemaphoreTake(PrintfMut, portMAX_DELAY);
  va_start(args, format);
  result = vrtprintf(format, args);
  va_end(args);
  xSemaphoreGive(PrintfMut);
  return result;
 }
#endif

/*======================================================================================================================*/

int _write(int file, char *ptr, int len)
 {
  HAL_UART_Transmit(&huart3, (uint8_t*)ptr, len - 1, HAL_MAX_DELAY);
  if (ptr[len - 1] == '\n')
   {
    HAL_UART_Transmit(&huart3, (uint8_t*)"\r\n", 2, HAL_MAX_DELAY);
   }
  else
   {
    HAL_UART_Transmit(&huart3, (uint8_t*)ptr + len - 1, 1, HAL_MAX_DELAY);
   }
return len;
}

/*======================================================================================================================*/

void AdjustIntVectTable()  /* Adjusts Interrupt Vector Table MCU pointer according to code location in flash memory. */
 {
  __disable_irq();
#ifdef __GNUC__     // In case of GCC compiler
  extern void *g_pfnVectors[];
  SCB->VTOR = (uint32_t)g_pfnVectors;  // In case of GCC compiler
#endif
#ifdef __ICCARM__  // In case of IAR compiler
  extern void __vector_table;
  SCB->VTOR = (uint32_t)&__vector_table;  // In case of IAR compiler
#endif
#ifdef __CC_ARM    // In case of Keil compiler  -- Not tested yet.
  extern void __Vectors;
  SCB->VTOR = (uint32_t)&__Vectors;  // In case of Keil compiler  -- Not tested yet.
#endif
  __enable_irq();
 }

/*======================================================================================================================*/

uint8_t RestFlags[] = {RCC_FLAG_PORRST, RCC_FLAG_BORRST, RCC_FLAG_IWDGRST, RCC_FLAG_WWDGRST, RCC_FLAG_SFTRST, RCC_FLAG_PINRST, RCC_FLAG_LSIRDY, RCC_FLAG_LPWRRST};

ResetReason_e CheckResetReason()
 {
  ResetReason_e i;
  for(i = 0; i < E_NUM_RESET_FLAGS; i++)
   {
    if(__HAL_RCC_GET_FLAG(RestFlags[i]))
     break;
   }
  __HAL_RCC_CLEAR_RESET_FLAGS();  // Clearing flags to prevent their existence at the next reason.
  return i;
 }



typedef struct BL_Conf_s
 {
  void *StartProgAddr;
  void *EndProgAddr;
  uint32_t ProgCRC;
 }BL_Conf_s;

typedef struct MemConf_s
 {
  BL_Conf_s BootLoaderConfig;
  uint32_t ConfCRC;
 }MemConf_s;


#define CODE_SIZE_KB (*((uint16_t*)FLASHSIZE_BASE))

 //FLASH_BASE
 //FLASHSIZE_BASE
 static MemConf_s MemConfig = {0};

void LoadConf()
 {
//  uint32_t fs = CODE_SIZE_KB;

  /* Declare symbols from the linker script */
  extern uint32_t _stext;   /* Start of code */
  extern uint32_t _etext;   /* End of code + rodata */
  extern uint32_t _sidata;  /* Start of data initialization in Flash */
  extern uint32_t _eidata;  /* End of data initialization in Flash */
  extern uint32_t _sdata;   /* Start of data in RAM */
  extern uint32_t _edata;   /* End of data in RAM */

  
//  uint32_t st = (uint32_t)&_stext;
//  uint32_t et = (uint32_t)&_etext;
//  uint32_t si = (uint32_t)&_sidata;
//  uint32_t ei = (uint32_t)&_eidata;
//  uint32_t sd = (uint32_t)&_sdata;
//  uint32_t ed = (uint32_t)&_edata;

  MemConfig.BootLoaderConfig.StartProgAddr = &_stext;
  MemConfig.BootLoaderConfig.EndProgAddr = &_eidata;

 }


 