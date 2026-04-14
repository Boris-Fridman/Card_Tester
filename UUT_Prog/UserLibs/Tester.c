/*
 * Tester.c
 *
 *  Created on: 13 Apr 2026
 *      Author: boris
 */

#include "Tester.h"

#include <string.h>

#include "main.h"
#include "usart.h"
#include "spi.h"
#include "i2c.h"
#include "dac.h"
#include "adc.h"
#include "tim.h"

#include "CommonData.h"


#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"
#include "semphr.h"
#include "FreeRTOSConfig.h"
#include "projdefs.h"



#define xTESTER_PRIORITY     0
#define xTESTER_STACK_SIZE   MAX(configMINIMAL_STACK_SIZE, 512)

#define TEST_REQ_QUEUE_LEN   5
#define TEST_REQ_MSG_SIZE   (sizeof(TestData_s) + MAX_TEST_PATTERN_SIZE)

#define INSPECTOR_UART  huart6
#define INSPECTED_UART  huart4




typedef struct TestReqMesg_s
 {
  TestData_s TestData;
  uint8_t TestPattern[MAX_TEST_PATTERN_SIZE];
 }TestReqMesg_s;






TaskHandle_t xTesterTaskHandle;      // Handle to network task.


QueueHandle_t TestReqQueue;




void MakeTest(TestData_s TestData, uint8_t TestPattern[]);

bool TestUART(uint8_t NInt, uint8_t TestPattern[], uint8_t TestPatLen);





void ReqForTest(TestData_s TestData, uint8_t TestPattern[])
 {
  TestReqMesg_s Message;
  Message.TestData = TestData;
  memcpy(Message.TestPattern, TestPattern, TestData.Bit_Pattern_Length);
  xQueueSend(TestReqQueue, &Message, pdMS_TO_TICKS(10));
 }






void TesterTask(void *pvParameters)
 {
  TestReqMesg_s Message;
  TestReqQueue = xQueueCreate(TEST_REQ_QUEUE_LEN, TEST_REQ_MSG_SIZE);
  if(TestReqQueue == NULL)
   {
    printf("\n%sThe queue couldn't be created.\nHalting !!!%s\n", TermRed, TermColorsReset);
    for(;;);
   }

  for(;;)
   {
    if(pdPASS == xQueueReceive(TestReqQueue, &Message, portMAX_DELAY))
     {
      MakeTest(Message.TestData, Message.TestPattern);
     }

    vTaskDelay(pdMS_TO_TICKS(1));
   }
  vTaskDelete(NULL);
 }


void TesterInit()
 {
  BaseType_t result;
  result = xTaskCreate(TesterTask, "Tester Task", xTESTER_STACK_SIZE, NULL, xTESTER_PRIORITY, &xTesterTaskHandle);
  if(result != pdPASS)
   {
	printf("\n\r%sThe tester task couldn't be created.%s\n\r", TermRed, TermColorsReset);
	//for(;;);
   }

 }




void MakeTest(TestData_s TestData, uint8_t TestPattern[])
 {
  if(TestData.Periph_B_F.UART_Flag)
   {
    TestUART(TestData.Num_Interations, TestPattern, TestData.Bit_Pattern_Length);
   }
  if(TestData.Periph_B_F.SPI_Flag)
   {

   }
  if(TestData.Periph_B_F.I2C_Flag)
   {

   }
  if(TestData.Periph_B_F.ADC_Flag)
   {

   }
  if(TestData.Periph_B_F.Timer_Flag)
   {

   }
 }

static uint8_t MedTestPattern[MAX_TEST_PATTERN_SIZE];
static uint8_t MedTestPatLen;
static SemaphoreHandle_t TestSemaphore;

bool TestUART(uint8_t NInt, uint8_t TestPattern[], uint8_t TestPatLen)
 {
	HAL_StatusTypeDef Result;
	uint8_t RecvPat[MAX_TEST_PATTERN_SIZE];
	bool TestResult;
	uint8_t i;
	TestSemaphore = xSemaphoreCreateBinary();
	MedTestPatLen = TestPatLen;

	for(i = 0; i < NInt; i++)
	 {
      Result = HAL_UART_Receive_DMA(&INSPECTED_UART, MedTestPattern, TestPatLen);
      Result = HAL_UART_Receive_DMA(&INSPECTOR_UART, RecvPat, TestPatLen);
      Result = HAL_UART_Transmit_DMA(&INSPECTED_UART, TestPattern, TestPatLen);
      UNUSED(Result);
      xSemaphoreTake(TestSemaphore, portMAX_DELAY);
      TestResult = (bool)memcmp(TestPattern, MedTestPattern, TestPatLen);
	 }

	vSemaphoreDelete(TestSemaphore);
	return TestResult;
 }


void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  static BaseType_t xHigherPriorityTaskWoken;
  xHigherPriorityTaskWoken = pdFALSE;

  if(huart == &INSPECTOR_UART)
   {
    xSemaphoreGiveFromISR(TestSemaphore, &xHigherPriorityTaskWoken);
   }
  if(huart == &INSPECTED_UART)
   {
    HAL_UART_Transmit_DMA(&INSPECTOR_UART, MedTestPattern, MedTestPatLen);
   }

}



