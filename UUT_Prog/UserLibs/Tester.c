/*
 * Tester.c
 *
 *  Created on: 13 Apr 2026
 *      Author: boris
 */

#include "Tester.h"

#include <string.h>

#include "Network.h"
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



#define xTESTER_PRIORITY       0
#define xTESTER_STACK_SIZE     MAX(configMINIMAL_STACK_SIZE, 1024)


#define xDEV_TEST_STACK_SIZE   MAX(configMINIMAL_STACK_SIZE, 512)
#define DEV_TEST_QUEUE_LEN     3

#define TEST_REQ_QUEUE_LEN     5
#define TEST_RESP_QUEUE_LEN   (DEV_TEST_QUEUE_LEN * E_NUM_PERIPHS)




#define INSPECTOR_UART  huart6
#define INSPECTED_UART  huart4

#define INSPECTOR_SPI  hspi1
#define INSPECTED_SPI  hspi4

#define INSPECTOR_I2C  hi2c1
#define INSPECTED_I2C  hi2c4




typedef struct TestReqMesg_s
 {
  TestData_s TestData;
  uint8_t TestPattern[MAX_TEST_PATTERN_SIZE];
 }TestReqMesg_s;

 typedef struct TestRespMesg_s
  {
   bool Result;
   PeriphType_e DevType;
  }TestRespMesg_s;



typedef struct PortTestMesg_s
 {
  uint8_t Num_Interations;
  uint8_t Bit_Pattern_Length;
  uint8_t TestPattern[MAX_TEST_PATTERN_SIZE];
 }PortTestMesg_s;



TaskHandle_t xTesterTaskHandle;        // Handle to Tester      task.
TaskHandle_t xUARTTestTaskHandle;      // Handle to UART   Test task.
TaskHandle_t xSPITestTaskHandle;       // Handle to SPI    Test task.
TaskHandle_t xI2CTestTaskHandle;       // Handle to I2C    Test task.
TaskHandle_t xADCTestTaskHandle;       // Handle to ADC    Test task.
TaskHandle_t xTimerTestTaskHandle;     // Handle to Timer  Test task.


QueueHandle_t TestReqQueue;
QueueHandle_t TestRespQueue;
static SemaphoreHandle_t RespQueueMut;



void MakeTest(TestData_s TestData, uint8_t TestPattern[]);


void ReqUARTTest(uint8_t TestPattern[], uint8_t PatLen, uint8_t Num_Interations);
void ReqSPITest(uint8_t TestPattern[], uint8_t PatLen, uint8_t Num_Interations);




void ReqForTest(TestData_s TestData, uint8_t TestPattern[])
 {
  TestReqMesg_s Message;
  Message.TestData = TestData;
  memcpy(Message.TestPattern, TestPattern, TestData.Bit_Pattern_Length);
  xQueueSend(TestReqQueue, &Message, pdMS_TO_TICKS(10));
 }


void SendTestResponse(bool Result, PeriphType_e PeriphType)
 {
  TestRespMesg_s Message;
  Message.Result = Result;
  Message.DevType = PeriphType;
  xSemaphoreTake(RespQueueMut, portMAX_DELAY);
  xQueueSend(TestRespQueue, &Message, pdMS_TO_TICKS(10));
  xSemaphoreGive(RespQueueMut);
 }



void TesterTask(void *pvParameters)
 {
  TestReqMesg_s ReqMsg;
  TestRespMesg_s RespMsg;
  PeriphBitField_s DevResults, RespondedDevs;

  for(;;)
   {
    if(pdPASS == xQueueReceive(TestReqQueue, &ReqMsg, portMAX_DELAY))
     {
      memset(&DevResults, 0, sizeof(DevResults));
      memset(&RespondedDevs, 0, sizeof(RespondedDevs));
      MakeTest(ReqMsg.TestData, ReqMsg.TestPattern);
      for(;;)
       {
        if(pdPASS == xQueueReceive(TestRespQueue, &RespMsg, portMAX_DELAY))
         {
          *(uint8_t*)&RespondedDevs |= (1 << RespMsg.DevType);                                                // Marking the device as responded.
          *(uint8_t*)&DevResults |= ((1 << RespMsg.DevType))*((uint8_t)RespMsg.Result);                       // Marking the device's answer
          if(!memcmp((void*)&RespondedDevs, (void*)&(ReqMsg.TestData.Periph_B_F), sizeof(PeriphBitField_s)))  // All requested devices responded.
           break;
         }
       }
      GiveResults(DevResults, ReqMsg.TestData.Periph_B_F, ReqMsg.TestData.Test_ID);
     }

    vTaskDelay(pdMS_TO_TICKS(1));
   }
  vTaskDelete(NULL); /* Emergency task deletion in case of break in loop existence */
 }





void MakeTest(TestData_s TestData, uint8_t TestPattern[])
 {
  if(TestData.Periph_B_F.UART_bf)
   {
    ReqUARTTest(TestPattern, TestData.Bit_Pattern_Length, TestData.Num_Interations);
   }
  if(TestData.Periph_B_F.SPI_bf)
   {
    ReqSPITest(TestPattern, TestData.Bit_Pattern_Length, TestData.Num_Interations);
   }
  if(TestData.Periph_B_F.I2C_bf)
   {

   }
  if(TestData.Periph_B_F.ADC_bf)
   {

   }
  if(TestData.Periph_B_F.Timer_bf)
   {

   }
 }








/*
 * *************************************************************************************************************
 **          UART Test Functions
 * *************************************************************************************************************
*/
static SemaphoreHandle_t UARTTestSem;
QueueHandle_t UARTTestQue;

void ReqUARTTest(uint8_t TestPattern[], uint8_t PatLen, uint8_t Num_Interations)
 {
  PortTestMesg_s Message;
  Message.Bit_Pattern_Length = PatLen;
  Message.Num_Interations = Num_Interations;
  memcpy(Message.TestPattern, TestPattern, PatLen);
  xQueueSend(UARTTestQue, &Message, pdMS_TO_TICKS(10));
 }


/*
 * TestUART(uint8_t NInt, uint8_t TestPattern[], uint8_t TestPatLen)
 *
 * returns "true" if the test passed. Otherwise returns "false".
 */
bool TestUART(uint8_t NInt, uint8_t TestPattern[], uint8_t TestPatLen)
 {
	uint8_t RecvPat[MAX_TEST_PATTERN_SIZE];
	uint8_t MedTestPattern[MAX_TEST_PATTERN_SIZE];
	bool TestResult;
	uint8_t i;
	uint32_t MaxTimeToWait;
	MaxTimeToWait = MAX(10, (TestPatLen * 1000) / MIN_UART_FREQUENCY);

    printf("\n\r%sStarting UART Test...%s\n\r", TermBlue, TermColorsReset);
    for(i = 0; i < NInt; i++)
     {
      TestResult = !HAL_UART_Receive_DMA(&INSPECTED_UART, MedTestPattern, TestPatLen);  // Waiter
      if(!TestResult) break;
      TestResult = !HAL_UART_Transmit_DMA(&INSPECTOR_UART, TestPattern, TestPatLen);    // Activator
      if(!TestResult) break;
      TestResult = xSemaphoreTake(UARTTestSem, pdMS_TO_TICKS(MaxTimeToWait));
      if(!TestResult) break;
      TestResult = !HAL_UART_Receive_DMA(&INSPECTOR_UART, RecvPat, TestPatLen);         // Waiter
      if(!TestResult) break;
      TestResult = !HAL_UART_Transmit_DMA(&INSPECTED_UART, MedTestPattern, TestPatLen); // Activator
      if(!TestResult) break;
      TestResult = xSemaphoreTake(UARTTestSem, pdMS_TO_TICKS(MaxTimeToWait));
      if(!TestResult) break;
      TestResult = !memcmp(TestPattern, MedTestPattern, TestPatLen);
      if(!TestResult) break;
     }
    printf("\n\r%sThe UART Test %s%s%s\n\r", TermBlue, ResultColors[TestResult], ResultMessages[TestResult], TermColorsReset);

    return TestResult;
 }


void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  static BaseType_t xHigherPriorityTaskWoken;
  xHigherPriorityTaskWoken = pdFALSE;

  if( (huart == &INSPECTOR_UART) || (huart == &INSPECTED_UART) )
   {
    xSemaphoreGiveFromISR(UARTTestSem, &xHigherPriorityTaskWoken);
   }

}


void UARTTestTask(void *pvParameters)
 {
  PortTestMesg_s Message;
  bool Result;
  for(;;)
   {
    if(pdPASS == xQueueReceive(UARTTestQue, &Message, portMAX_DELAY))
     {
      Result = TestUART(Message.Num_Interations, Message.TestPattern, Message.Bit_Pattern_Length);
      SendTestResponse(Result, E_UART);
     }
    vTaskDelay(pdMS_TO_TICKS(1));
   }
  vTaskDelete(NULL); /* Emergency task deletion in case of break in loop existence */
 }



/*
 * *************************************************************************************************************
 **          SPI Test Functions
 * *************************************************************************************************************
*/


static SemaphoreHandle_t SPITestSem;
QueueHandle_t SPITestQue;

void ReqSPITest(uint8_t TestPattern[], uint8_t PatLen, uint8_t Num_Interations)
 {
  PortTestMesg_s Message;
  Message.Bit_Pattern_Length = PatLen;
  Message.Num_Interations = Num_Interations;
  memcpy(Message.TestPattern, TestPattern, PatLen);
  xQueueSend(SPITestQue, &Message, pdMS_TO_TICKS(10));
 }

/*
 * TestSPI(uint8_t NInt, uint8_t TestPattern[], uint8_t TestPatLen)
 *
 * returns "true" if the test passed. Otherwise returns "false".
 */
bool TestSPI(uint8_t NInt, uint8_t TestPattern[], uint8_t TestPatLen)
 {
  uint8_t RecvPat[MAX_TEST_PATTERN_SIZE];
  uint8_t MedTestPattern[MAX_TEST_PATTERN_SIZE];
  bool TestResult;
  uint8_t i;
  uint32_t MaxTimeToWait;
  MaxTimeToWait = MAX(10, (TestPatLen * 1000) / MIN_SPI_FREQUENCY);

  printf("\n\r%sStarting SPI Test...%s\n\r", TermBlue, TermColorsReset);
  for(i = 0; i < NInt; i++)
   {
    TestResult = !HAL_SPI_Receive_DMA(&INSPECTED_SPI, MedTestPattern, TestPatLen);  // Waiter    (Slave)
    if(!TestResult) break;
    TestResult = !HAL_SPI_Transmit_DMA(&INSPECTOR_SPI, TestPattern, TestPatLen);    // Activator (Master)
    if(!TestResult) break;
    TestResult = xSemaphoreTake(SPITestSem, pdMS_TO_TICKS(MaxTimeToWait));
    if(!TestResult) break;
    TestResult = !HAL_SPI_Transmit_DMA(&INSPECTED_SPI, MedTestPattern, TestPatLen); // Waiter    (Slave)
    if(!TestResult) break;
    TestResult = !HAL_SPI_Receive_DMA(&INSPECTOR_SPI, RecvPat, TestPatLen);         // Activator (Master)
    if(!TestResult) break;
    TestResult = xSemaphoreTake(SPITestSem, pdMS_TO_TICKS(MaxTimeToWait));
    if(!TestResult) break;
    TestResult = !memcmp(TestPattern, MedTestPattern, TestPatLen);
    if(!TestResult) break;
   }
  printf("\n\r%sThe SPI Test %s%s%s\n\r", TermBlue, ResultColors[TestResult], ResultMessages[TestResult], TermColorsReset);

  return true;
 }


void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef *hspi)
{
  static BaseType_t xHigherPriorityTaskWoken;
  xHigherPriorityTaskWoken = pdFALSE;

  if( (hspi == &INSPECTOR_SPI) || (hspi == &INSPECTED_SPI) )
   {
    xSemaphoreGiveFromISR(SPITestSem, &xHigherPriorityTaskWoken);
   }

}


void SPITestTask(void *pvParameters)
 {
  PortTestMesg_s Message;
  bool Result;
  for(;;)
   {
    if(pdPASS == xQueueReceive(SPITestQue, &Message, portMAX_DELAY))
     {
      Result = TestSPI(Message.Num_Interations, Message.TestPattern, Message.Bit_Pattern_Length);
      SendTestResponse(Result, E_SPI);
     }
    vTaskDelay(pdMS_TO_TICKS(1));
   }
  vTaskDelete(NULL); /* Emergency task deletion in case of break in loop existence */
 }




/*
 * *************************************************************************
 *
 * ***********  Tester Initializations. *******
 *
 * *************************************************************************
 */


void TesterInit()
 {
  BaseType_t result;

  TestReqQueue = xQueueCreate(TEST_REQ_QUEUE_LEN, sizeof(TestReqMesg_s));
  if(TestReqQueue == NULL)
   {
    printf("\n%sThe test request queue couldn't be created.\n\rHalting !!!%s\n", TermRed, TermColorsReset);
    for(;;);
   }
  TestRespQueue = xQueueCreate(TEST_RESP_QUEUE_LEN, sizeof(TestRespMesg_s));
  if(TestReqQueue == NULL)
   {
    printf("\n%sThe test response queue couldn't be created.\n\rHalting !!!%s\n", TermRed, TermColorsReset);
    for(;;);
   }
  RespQueueMut = xSemaphoreCreateMutex();
  if(RespQueueMut == NULL)
   {
    printf("\n%sThe test-response-queue-mutex couldn't be created.\n\rHalting !!!%s\n", TermRed, TermColorsReset);
    for(;;);
   }
  xSemaphoreGive(RespQueueMut);

  result = xTaskCreate(TesterTask, "Tester Task", xTESTER_STACK_SIZE, NULL, xTESTER_PRIORITY, &xTesterTaskHandle);
  if(result != pdPASS)
   {
	printf("\n\r%sThe tester task couldn't be created.\n\rHalting !!!%s\n\r", TermRed, TermColorsReset);
	for(;;);
   }

 /*       UART Task Creating             */

  UARTTestQue = xQueueCreate(DEV_TEST_QUEUE_LEN, sizeof(PortTestMesg_s));
  if(UARTTestQue == NULL)
   {
    printf("\n\r%sThe UART Test Queue couldn't be created.\n\rHalting !!!%s\n\r", TermRed, TermColorsReset);
    for(;;);
   }
  UARTTestSem = xSemaphoreCreateBinary();
  if(UARTTestSem == NULL)
   {
    printf("\n\r%sThe UART Test Queue couldn't be created.\n\rHalting !!!%s\n\r", TermRed, TermColorsReset);
    for(;;);
   }

  result = xTaskCreate(UARTTestTask, "UART Test Task", xDEV_TEST_STACK_SIZE, NULL, xTESTER_PRIORITY, &xUARTTestTaskHandle);
  if(result != pdPASS)
   {
    printf("\n\r%sThe UART Test Task couldn't be created.\n\rHalting !!!%s\n\r", TermRed, TermColorsReset);
    for(;;);
   }






  /*       SPI Task Creating             */


  SPITestQue = xQueueCreate(DEV_TEST_QUEUE_LEN, sizeof(PortTestMesg_s));
  if(SPITestQue == NULL)
   {
    printf("\n\r%sThe SPI Test Queue couldn't be created.\n\rHalting !!!%s\n\r", TermRed, TermColorsReset);
    for(;;);
   }
  SPITestSem = xSemaphoreCreateBinary();
  if(SPITestSem == NULL)
   {
    printf("\n\r%sThe SPI Test Queue couldn't be created.\n\rHalting !!!%s\n\r", TermRed, TermColorsReset);
    for(;;);
   }

  result = xTaskCreate(SPITestTask, "SPI Test Task", xDEV_TEST_STACK_SIZE, NULL, xTESTER_PRIORITY, &xSPITestTaskHandle);
  if(result != pdPASS)
   {
    printf("\n\r%sThe SPI Test Task couldn't be created.\n\rHalting !!!%s\n\r", TermRed, TermColorsReset);
    for(;;);
   }




 }




