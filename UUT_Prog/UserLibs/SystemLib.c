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

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"
#include "semphr.h"
#include "FreeRTOSConfig.h"
#include "projdefs.h"


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



