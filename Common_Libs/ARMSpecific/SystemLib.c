/*
 * SystemLib.c
 *
 *  Created on: 18 Apr 2026
 *      Author: boris
 */


#include "SystemLib.h"
#include "main.h"

#include "stm32f7xx_hal_flash.h"

#include "usart.h"
#include <stdarg.h>
#include <string.h>

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

SectorAddr_s const SectorsAddr[FLASH_SECTOR_TOTAL] =
  {
    {0x08000000 , 0x08007FFF},
    {0x08008000 , 0x0800FFFF},
    {0x08010000 , 0x08017FFF},
    {0x08018000 , 0x0801FFFF},
    {0x08020000 , 0x0803FFFF},
    {0x08040000 , 0x0807FFFF},
    {0x08080000 , 0x080BFFFF},
    {0x080C0000 , 0x080FFFFF}
  };

/*======================================================================================================================*/
#ifdef USE_FREERTOS
/*
 * *************************************************************************************************************
 **          RTOS Printing without mixing Functions / Procedures
 * *************************************************************************************************************
 */

static SemaphoreHandle_t PrintfMut;

/*----------------------------------------------------------------------------------------------------------------------*/
/*  Initializes mutexes used by the function "rtprintf()".                                                              */
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


/*----------------------------------------------------------------------------------------------------------------------*/
/*  Is a part of the function "rtprintf()".                                                                             */
int	vrtprintf(const char *format, va_list args)
 {
  int result;
  result = vprintf(format, args);
  return result;
 }

/*----------------------------------------------------------------------------------------------------------------------*/
/*  The function is similar to "printf()" but it is using the internal mutex to prevent print-mixing                    */
/*  when two or more tasks try to print messages at the same time.                                                      */
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
/*
 * *************************************************************************************************************
 **          Printing to UART Function
 * *************************************************************************************************************
 */
/*----------------------------------------------------------------------------------------------------------------------*/
/*  Is used for sending characters to the required UART port (huart3) while calling the standard output functions       */
/*  as "printf()", "putch()" or "puts()".                                                                               */
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
/*
 * *************************************************************************************************************
 **          Adjusting Interrupt vector table Procedure
 * *************************************************************************************************************
 */

/*----------------------------------------------------------------------------------------------------------------------*/
/*  Adjusts Interrupt Vector Table MCU pointer according to code location in flash memory.                              */
void AdjustIntVectTable()  /* Adjusts Interrupt Vector Table MCU pointer according to code location in flash memory. */
 {
  __disable_irq();
#ifdef __GNUC__     /* In case of GCC compiler */
  extern void *g_pfnVectors[];
  SCB->VTOR = (uint32_t)g_pfnVectors;  /* In case of GCC compiler */
#endif
#ifdef __ICCARM__  /* In case of IAR compiler */
  extern void __vector_table;
  SCB->VTOR = (uint32_t)&__vector_table;  /* In case of IAR compiler */
#endif
#ifdef __CC_ARM    /* In case of Keil compiler  -- Not tested yet. */
  extern void __Vectors;
  SCB->VTOR = (uint32_t)&__Vectors;  /* In case of Keil compiler  -- Not tested yet. */
#endif

  __enable_irq();

 }

/*======================================================================================================================*/

uint8_t RestFlags[] = {RCC_FLAG_PORRST, RCC_FLAG_BORRST, RCC_FLAG_IWDGRST, RCC_FLAG_WWDGRST, RCC_FLAG_SFTRST, RCC_FLAG_PINRST, RCC_FLAG_LSIRDY, RCC_FLAG_LPWRRST};

/*----------------------------------------------------------------------------------------------------------------------*/
/*  The function checks the reason of reset at startup.                                                                 */
ResetReason_e CheckResetReason()
 {
  ResetReason_e i;
  for(i = 0; i < E_NUM_RESET_FLAGS; i++)
   {
    if(__HAL_RCC_GET_FLAG(RestFlags[i]))
     break;
   }
  __HAL_RCC_CLEAR_RESET_FLAGS();  /* Clearing flags to prevent their existence at the next reset reason checking. */
  return i;
 }


/*======================================================================================================================*/
/*
 * *************************************************************************************************************
 **          Flash saving / loading  Functions / Procedures
 * *************************************************************************************************************
 */

#define CODE_SIZE_KB (*((uint16_t*)FLASHSIZE_BASE))

 //FLASH_BASE
 //FLASHSIZE_BASE
 static MemConf_s MemConfig = {0};

 /*----------------------------------------------------------------------------------------------------------------------*/
/*  Loads the program configuration saved in the flash memory.                                                           */
void LoadConf()
 {
  MemConfig = *(MemConf_s *)(SectorsAddr[FLASH_CONFIG_SECTOR].Start);
 }

/*----------------------------------------------------------------------------------------------------------------------*/
/*  Returns configuration saved in the flash memory.                                                                    */
void SaveConf()
 {
  uint32_t NumBlocks, i;
  uint32_t SectorError = 0;
  FLASH_EraseInitTypeDef pEraseInit = {0};
  const uint32_t FirstConfAddress = SectorsAddr[FLASH_CONFIG_SECTOR].Start;
  const uint32_t LastConfAddress = SectorsAddr[FLASH_CONFIG_SECTOR].End;
  const uint8_t BlockSize = sizeof(uint32_t);
  HAL_StatusTypeDef Result;
  uint32_t *DataToWrite = NULL;
  uint32_t ConfDataSize = sizeof(MemConf_s);
  printf(TermYello);

  printf("Saving Configuration..\n\r");

  if((MemConfig.DataLen >= (LastConfAddress - FirstConfAddress))||(MemConfig.DataLen < ConfDataSize)) /* Means that the length of the data is invalid. */
   {
    MemConfig.DataLen = ConfDataSize;
   }

  printf("The start address: 0x%08lX\n\rThe last Address is: 0x%08lX\n\r", FirstConfAddress, LastConfAddress);
  printf("The size is: %ld\n\r", MemConfig.DataLen);

  NumBlocks = DIV_RND_UP(MemConfig.DataLen, BlockSize);
  DataToWrite = calloc(NumBlocks, BlockSize);  /* The "callock()" is required to ensure that the address can be divided by 4 without reminded parts. */
  if(DataToWrite != NULL)
   {
	printf("Memory was allocated successfully. The start address is: 0x%08lX\n\r", (uint32_t)DataToWrite);
    memset(DataToWrite, 0xFF, MemConfig.DataLen);
    memcpy(DataToWrite, (void*)FirstConfAddress, MemConfig.DataLen);
    *(MemConf_s *)DataToWrite = MemConfig;

    Result = HAL_FLASH_Unlock();
    if(Result == HAL_OK)
     {
      printf("The flash was unlocked.\n\r");
      pEraseInit.NbSectors = 1;
      pEraseInit.Sector = (FLASH_CONFIG_SECTOR);
      pEraseInit.TypeErase = FLASH_TYPEERASE_SECTORS;
      pEraseInit.VoltageRange = FLASH_VOLTAGE_RANGE_3;
      Result = HAL_FLASHEx_Erase(&pEraseInit, &SectorError);
      printf("The page was erased.\n\r");
      if(Result == HAL_OK)
       {
        for(i = 0; i < NumBlocks; i++)
         {
          Result = HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, FirstConfAddress + i * BlockSize, DataToWrite[i]);
         }
       }
      printf("The data was written.\n\r");
      Result = HAL_FLASH_Lock();
      printf("The flash was locked.\n\r");
     }

    free(DataToWrite);
    printf(TermColorsReset);
   }


 }



/*----------------------------------------------------------------------------------------------------------------------*/
/*  Returns configuration saved in the flash memory.                                                                    */
void GetBLConf(BL_Conf_s *BlConf)
 {
  LoadConf();
  *BlConf = MemConfig.BootLoaderConfig;
 }
 
/*----------------------------------------------------------------------------------------------------------------------*/
/*  Saves Configuration to the flash memory.                                                                            */
void SetBLConf(BL_Conf_s BlConf)
 {
  MemConfig.BootLoaderConfig = BlConf;
  SaveConf();
 }



