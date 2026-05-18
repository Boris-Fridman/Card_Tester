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
#include "SystemLib.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"
#include "semphr.h"
#include "FreeRTOSConfig.h"
#include "projdefs.h"

/*======================================================================================================================*/

#define xTESTER_PRIORITY       0                                          /* Priority of the tester tasks. */
#define xTESTER_STACK_SIZE     MAX(configMINIMAL_STACK_SIZE, 1024)        /* Stack size of the main-test-task. */


#define xDEV_TEST_STACK_SIZE   MAX(configMINIMAL_STACK_SIZE, 512)         /* Stack size of the single-dev-test-task. */
#define DEV_TEST_QUEUE_LEN     3 /*Will be changed to 1 in the future.*/  /* Length of the queue from the network-task to the main-test-task. */

#define TEST_REQ_QUEUE_LEN     5                                          /* Length of the queue of requests from the main-test-task to the single-dev-test-task. */
#define TEST_RESP_QUEUE_LEN   (DEV_TEST_QUEUE_LEN * E_NUM_PERIPHS)        /* Length of the queue of responses from the single-dev-test-task to the main-test-task. */

#define MessColor   TermBrightBlue
#define DevsColor   TermYello
#define ValuesColor TermMagenta
#define UnitsColor  TermCyan


/*----------------------------------------------------------------------------------------------------------------------*/


/**
 * 
 * 
 *        ┏━━━━━━━━━━━┳━━━━━━━━━━━━━━━━━━━━━┳━━━━━━━━━━━━━━━━━━━━━━┓
 *        ┃           ┃        Tester       ┃        Tested        ┃
 *        ┃           ┃                     ┃                      ┃
 *        ┣━━━━━━━━━━━╋━━━━━━━━━━━━━━━━━━━━━╋━━━━━━━━━━━━━━━━━━━━━━┫
 *        ┃   Timer   ┃   Timer_1  Ch_1     ┃   Timer_2   Ch_2     ┃
 *        ┣━━━━━━━━━━━╋━━━━━━━━━━━━━━━━━━━━━╋━━━━━━━━━━━━━━━━━━━━━━┫
 *        ┃   UART    ┃   UART_6            ┃   UART_4             ┃
 *        ┣━━━━━━━━━━━╋━━━━━━━━━━━━━━━━━━━━━╋━━━━━━━━━━━━━━━━━━━━━━┫
 *        ┃   SPI     ┃   SPI_1    Master   ┃   SPI_4     Slave    ┃
 *        ┣━━━━━━━━━━━╋━━━━━━━━━━━━━━━━━━━━━╋━━━━━━━━━━━━━━━━━━━━━━┫
 *        ┃   I2C     ┃   I2C_1    Master   ┃   I2C_2     Slave    ┃
 *        ┣━━━━━━━━━━━╋━━━━━━━━━━━━━━━━━━━━━╋━━━━━━━━━━━━━━━━━━━━━━┫
 *        ┃   ADC     ┃   DAC               ┃   ADC_1     Ch_0     ┃
 *        ┗━━━━━━━━━━━┻━━━━━━━━━━━━━━━━━━━━━┻━━━━━━━━━━━━━━━━━━━━━━┛
 * 
 */


#define INSPECTOR_UART               huart6
#define INSPECTED_UART               huart4

#define INSPECTOR_SPI                hspi1
#define INSPECTED_SPI                hspi4

#define INSPECTOR_I2C                hi2c1
#define INSPECTED_I2C                hi2c2

#define INSPECTOR_DAC                hdac
#define INSPECTED_ADC                hadc1

#define INSPECTOR_TIM                htim1
#define INSPECTED_TIM                htim2

#define INSPECTOR_TIM_CHANNEL        TIM_CHANNEL_1          /* Is used for Configuration*/
#define INSPECTED_TIM_CHANNEL        TIM_CHANNEL_2          /* Is used for Configuration*/

#define INSPECTOR_TIM_ACTIVE_CHANNEL HAL_TIM_ACTIVE_CHANNEL_1  /* Is used for Detection */
#define INSPECTED_TIM_ACTIVE_CHANNEL HAL_TIM_ACTIVE_CHANNEL_2  /* Is used for Detection */


/*======================================================================================================================*/

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


typedef struct DevTestInfo_s
 {
  int32_t TestVoltage;
  uint32_t TestTime;
  uint8_t Num_Interations;
  uint8_t Bit_Pattern_Length;
  uint8_t *TestPattern;
 }DevTestInfo_s;

 typedef struct DevTestMesg_s
  {
   DevTestInfo_s *TestInfo;
  }DevTestMesg_s;

typedef struct DevTaskParams_s
 {
  QueueHandle_t DevTestQue;
  PeriphType_e PeriphType;
 }DevTaskParams_s;

 /*======================================================================================================================*/

TaskHandle_t xTesterTaskHandle;                   /* Handle to Tester      task. */

TaskHandle_t xDevTestTaskHandles[E_NUM_PERIPHS];  /* Handles to UART, SPI,I2C, ADC and Timer tasks.  -- Will be implemented in the future. */

QueueHandle_t TestReqQueue;                       /* Queue of test requests.     Test Manager ◀▬▬▬ Network              */
QueueHandle_t TestRespQueue;                      /* Queue of test responses.    Peripheral Tester ▬▬▬▶ Test Manager    */
static SemaphoreHandle_t RespQueueMut;            /* Mutex for queue of responses. Because the responses are given from different tasks the mutex for accessing the queue of responses is required. */

/*======================================================================================================================*/

/* Distributes the given test request between the existing peripherals selected in the request.                         */
void MakeTest(DevTestInfo_s *DevTestInfo, PeriphBitField_s Periph_B_F);

/*  Forwards the request to selected peripheral.                                                                        */
void ReqDevTest(DevTestInfo_s *DevTestInfo, PeriphType_e PeriphType);

/*  Prints Test Information: Number of interations , Requested devices for test and optionally:                         */
/*  Test Pattern for ports , Test Voltage for ADC and Test Time for Timer                                               */
void PrintTestInfo(TestData_s TestData, uint8_t TestPattern[]);

/*======================================================================================================================*/
/*
 * *************************************************************************************************************
 **          Requests Responses Forwarding Procedures.
 * *************************************************************************************************************
 */

/*----------------------------------------------------------------------------------------------------------------------*/
/*    Sends request for test                                                                                            */
/*    Test Manager <---- Network                                                                                        */
void ReqForTest(TestData_s TestData, uint8_t TestPattern[])
 {
  TestReqMesg_s Message;
  Message.TestData = TestData;
  memcpy(Message.TestPattern, TestPattern, TestData.Bit_Pattern_Length);
  xQueueSend(TestReqQueue, &Message, pdMS_TO_TICKS(10));
 }

/*----------------------------------------------------------------------------------------------------------------------*/
/*    Sends Test Response from peripheral-testing-task to the test manager                                              */
/*    Peripheral Task  -----> Test Manager                                                                              */
void SendTestResponse(bool Result, PeriphType_e PeriphType)
 {
  TestRespMesg_s Message;
  Message.Result = Result;
  Message.DevType = PeriphType;
  xSemaphoreTake(RespQueueMut, portMAX_DELAY);
  xQueueSend(TestRespQueue, &Message, pdMS_TO_TICKS(10));
  xSemaphoreGive(RespQueueMut);
 }


/*======================================================================================================================*/
/*
 * *************************************************************************************************************
 **          Testing task Functions / Procedures.
 * *************************************************************************************************************
 */

/*----------------------------------------------------------------------------------------------------------------------*/
/*    Test Manager Task                                                                                                 */
void TesterTask(void *pvParameters)
 {
  TestReqMesg_s ReqMsg;
  TestRespMesg_s RespMsg;
  PeriphBitField_s DevResults, RespondedDevs;
  static DevTestInfo_s DevTestInfo = {0};

  for(;;)
   {
    if(pdPASS == xQueueReceive(TestReqQueue, &ReqMsg, portMAX_DELAY))
     {
      if(!memcmp(&ReqMsg.TestData, &ResetCondition, sizeof(TestData_s)))
       {
    	rtprintf("%sRestarting the device...%s\n\r", TermYello, TermColorsReset);
        __HAL_RCC_CLEAR_RESET_FLAGS();  // Clearing all reset flags before making reset to ensure that the bootloader will detect the correct reset reason.
        NVIC_SystemReset();             // Resetting Device.
       }
      PrintTestInfo(ReqMsg.TestData, ReqMsg.TestPattern);
      memset(&DevResults, 0, sizeof(DevResults));
      memset(&RespondedDevs, 0, sizeof(RespondedDevs));
      DevTestInfo.Bit_Pattern_Length = ReqMsg.TestData.Bit_Pattern_Length;
      DevTestInfo.Num_Interations = ReqMsg.TestData.Num_Interations;
      DevTestInfo.TestTime = ReqMsg.TestData.TestTime;
      DevTestInfo.TestVoltage = ReqMsg.TestData.TestVoltage;
      if(ReqMsg.TestData.Bit_Pattern_Length)
       {
        DevTestInfo.TestPattern = calloc(ReqMsg.TestData.Bit_Pattern_Length, sizeof(uint8_t));
        if(DevTestInfo.TestPattern != NULL)
         {
          memcpy(DevTestInfo.TestPattern, ReqMsg.TestPattern, ReqMsg.TestData.Bit_Pattern_Length);
         }
       }
      MakeTest(&DevTestInfo, ReqMsg.TestData.Periph_B_F);
      for(;;)
       {
        if(!memcmp((void*)&RespondedDevs, (void*)&(ReqMsg.TestData.Periph_B_F), sizeof(PeriphBitField_s)))    /* All requested devices responded. - The line is put before checking the queue to prevent hanging if no devices selected. */
         break;
        if(pdPASS == xQueueReceive(TestRespQueue, &RespMsg, portMAX_DELAY))
         {
          *(uint8_t*)&RespondedDevs |= (1 << RespMsg.DevType);                                                /* Marking the device as responded. */
          *(uint8_t*)&DevResults    |= ((1 << RespMsg.DevType))*((uint8_t)RespMsg.Result);                    /* Marking the device's answer.     */
         }
       }
      GiveResults(DevResults, ReqMsg.TestData.Periph_B_F, ReqMsg.TestData.Test_ID);
      if(DevTestInfo.TestPattern)
       {
        free(DevTestInfo.TestPattern);
        DevTestInfo.TestPattern = NULL;
       }
     }

    vTaskDelay(pdMS_TO_TICKS(1));
   }
  vTaskDelete(NULL); /* Emergency task deletion in case of break in loop existence */
 }


/*----------------------------------------------------------------------------------------------------------------------*/
/* Distributes the given test request between the existing peripherals selected in the request.                         */
/* Checks which peripherals were selected for test in the request and forwards the request only to them.                */
void MakeTest(DevTestInfo_s *DevTestInfo, PeriphBitField_s Periph_B_F)
 {
  uint8_t i;
  uint8_t PeriphFlags = *(uint8_t*)&Periph_B_F;
  for(i = 0; i < E_NUM_PERIPHS; i++)
   {
    if((PeriphFlags >> i) & 0x01)
     ReqDevTest(DevTestInfo, i);
   }
 }

/*----------------------------------------------------------------------------------------------------------------------*/

DevTaskParams_s DevTaskParams[E_NUM_PERIPHS] = {0};
/*----------------------------------------------------------------------------------------------------------------------*/
/*  Forwards the request to selected peripheral.                                                                        */
void ReqDevTest(DevTestInfo_s *DevTestInfo, PeriphType_e PeriphType)
 {
  DevTestMesg_s Message;
  Message.TestInfo = DevTestInfo;
  xQueueSend(DevTaskParams[PeriphType].DevTestQue, &Message, pdMS_TO_TICKS(10));
 }



/*======================================================================================================================*/
/*
 * *************************************************************************************************************
 **          UART Test Functions
 * *************************************************************************************************************
 */

/**
 *        ┏━━━━━━━━━━━━━━━━━━━━━┳━━━━━━━━━━━━┳━━━━━━━━━━━━━━━━━━━━━┓
 *        ┃    UART_6 Tester    ┃ Direction  ┃    UART_4 Tested    ┃
 *        ┣━━━━━━━━━━┳━━━━━━━━━━╋━━━━━━━━━━━━╋━━━━━━━━━━┳━━━━━━━━━━┫
 *        ┃   TX     ┃   PD8    ┃    ▬▬▬▶    ┃   PC11   ┃   RX     ┃
 *        ┣━━━━━━━━━━╋━━━━━━━━━━╋━━━━━━━━━━━━╋━━━━━━━━━━╋━━━━━━━━━━┫
 *        ┃   RX     ┃   PD9    ┃    ◀▬▬▬    ┃   PC10   ┃   TX     ┃
 *        ┗━━━━━━━━━━┻━━━━━━━━━━┻━━━━━━━━━━━━┻━━━━━━━━━━┻━━━━━━━━━━┛
 */

static SemaphoreHandle_t UARTTestSem;

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

	xSemaphoreTake(UARTTestSem, 0);  /* Setting semaphore to taken state without any waiting to ensure that after transferring the program will wait for interrupt. */

    for(i = 0; i < NInt; i++)
     {
      TestResult = !HAL_UART_Receive_DMA(&INSPECTED_UART, MedTestPattern, TestPatLen);  /* Waiter     */
      if(!TestResult) break;
      TestResult = !HAL_UART_Transmit_DMA(&INSPECTOR_UART, TestPattern, TestPatLen);    /* Activator  */
      if(!TestResult) break;
      TestResult = xSemaphoreTake(UARTTestSem, pdMS_TO_TICKS(MaxTimeToWait));
      if(!TestResult) break;
      TestResult = !HAL_UART_Receive_DMA(&INSPECTOR_UART, RecvPat, TestPatLen);         /* Waiter     */
      if(!TestResult) break;
      TestResult = !HAL_UART_Transmit_DMA(&INSPECTED_UART, MedTestPattern, TestPatLen); /* Activator  */
      if(!TestResult) break;
      TestResult = xSemaphoreTake(UARTTestSem, pdMS_TO_TICKS(MaxTimeToWait));
      if(!TestResult) break;
      TestResult = !memcmp(TestPattern, MedTestPattern, TestPatLen);
      if(!TestResult) break;
     }
    HAL_UART_Abort(&INSPECTOR_UART);  /* Aborting Inspector UART before the "MedTestPattern[]" and "RecvPat[]" are disappeared on exit to prevent accessing to the incorrect memory. */
    HAL_UART_Abort(&INSPECTED_UART);  /* Aborting Inspected UART before the "MedTestPattern[]" and "RecvPat[]" are disappeared on exit to prevent accessing to the incorrect memory. */
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




/*
 * *************************************************************************************************************
 **          SPI Test Functions
 * *************************************************************************************************************
*/
/**
 *        ┏━━━━━━━━━━━━━━━━━━━━━┳━━━━━━━━━━━━┳━━━━━━━━━━━━━━━━━━━━━┓
 *        ┃    SPI_1 Tester     ┃ Direction  ┃    SPI_4 Tested     ┃
 *        ┃    Master           ┃            ┃    Slave            ┃
 *        ┣━━━━━━━━━━┳━━━━━━━━━━╋━━━━━━━━━━━━╋━━━━━━━━━━┳━━━━━━━━━━┫
 *        ┃   MOSI   ┃   PB5    ┃    ▬▬▬▶    ┃   PE6    ┃   MOSI   ┃
 *        ┣━━━━━━━━━━╋━━━━━━━━━━╋━━━━━━━━━━━━╋━━━━━━━━━━╋━━━━━━━━━━┫
 *        ┃   MISO   ┃   PA6    ┃    ◀▬▬▬    ┃   PE5    ┃   MISO   ┃
 *        ┣━━━━━━━━━━╋━━━━━━━━━━╋━━━━━━━━━━━━╋━━━━━━━━━━╋━━━━━━━━━━┫
 *        ┃   CLK    ┃   PA5    ┃    ▬▬▬▶    ┃   PE2    ┃   CLK    ┃
 *        ┣━━━━━━━━━━╋━━━━━━━━━━╋━━━━━━━━━━━━╋━━━━━━━━━━╋━━━━━━━━━━┫
 *        ┃   NSS    ┃   PA15   ┃    ▬▬▬▶    ┃   PE4    ┃   NSS    ┃
 *        ┗━━━━━━━━━━┻━━━━━━━━━━┻━━━━━━━━━━━━┻━━━━━━━━━━┻━━━━━━━━━━┛
 */

static SemaphoreHandle_t SPITestSem;

/*
 * TestSPI(uint8_t NInt, uint8_t TestPattern[], uint8_t TestPatLen)
 *
 * returns "true" if the test passed. Otherwise returns "false".
 */
bool TestSPI(uint8_t NInt, uint8_t TestPattern[], uint8_t TestPatLen)
 {
  uint8_t RecvPat[MAX_TEST_PATTERN_SIZE] = {0};
  uint8_t MedTestPattern[MAX_TEST_PATTERN_SIZE] = {0};
  uint8_t EmptyTestPattern[MAX_TEST_PATTERN_SIZE] = {0};
  bool TestResult;
  uint8_t i;
  uint32_t MaxTimeToWait;
  MaxTimeToWait = MAX(10, (TestPatLen * 1000) / MIN_SPI_FREQUENCY);

  for(i = 0; i < NInt; i++)
   {
    //xQueueReset((QueueHandle_t)SPITestSem);
    xSemaphoreTake(SPITestSem, 0);  /* Setting semaphore to taken state without any waiting to ensure that after transferring the program will wait for interrupt. */

    TestResult = !HAL_SPI_TransmitReceive_DMA(&INSPECTED_SPI, EmptyTestPattern, MedTestPattern, TestPatLen);  /* Receiving    */ /*    Waiter    (Slave) */
    if(!TestResult) break;
    TestResult = !HAL_SPI_TransmitReceive_DMA(&INSPECTOR_SPI, TestPattern, EmptyTestPattern, TestPatLen);     /* Transmitting */ /*    Activator (Master) */
    if(!TestResult) break;
    TestResult = xSemaphoreTake(SPITestSem, pdMS_TO_TICKS(MaxTimeToWait));
    if(!TestResult) break;
    TestResult = !HAL_SPI_TransmitReceive_DMA(&INSPECTED_SPI, MedTestPattern, EmptyTestPattern, TestPatLen);  /* Transmitting */ /*     Waiter    (Slave) */
    if(!TestResult) break;
    TestResult = !HAL_SPI_TransmitReceive_DMA(&INSPECTOR_SPI, EmptyTestPattern, RecvPat, TestPatLen);         /* Receiving    */ /*     Activator (Master) */
    if(!TestResult) break;
    TestResult = xSemaphoreTake(SPITestSem, pdMS_TO_TICKS(MaxTimeToWait));
    if(!TestResult) break;
    TestResult = !memcmp(TestPattern, MedTestPattern, TestPatLen);
    if(!TestResult) break;
   }

  HAL_SPI_Abort(&INSPECTOR_SPI);  /* Aborting Inspector SPI before the "MedTestPattern[]" and "RecvPat[]" are disappeared on exit to prevent accessing to the incorrect memory. */
  HAL_SPI_Abort(&INSPECTED_SPI);  /* Aborting Inspected SPI before the "MedTestPattern[]" and "RecvPat[]" are disappeared on exit to prevent accessing to the incorrect memory. */
  return TestResult;
 }


void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi)
 {
  static BaseType_t xHigherPriorityTaskWoken;
  xHigherPriorityTaskWoken = pdFALSE;

  if( (hspi == &INSPECTOR_SPI) || (hspi == &INSPECTED_SPI) )
   {
    xSemaphoreGiveFromISR(SPITestSem, &xHigherPriorityTaskWoken);
   }

 }



/*
 * *************************************************************************************************************
 **          I2C Test Functions
 * *************************************************************************************************************
*/
/**
 *        ┏━━━━━━━━━━━━━━━━━━━━━┳━━━━━━━━━━━━┳━━━━━━━━━━━━━━━━━━━━━┓
 *        ┃    I2C_1 Tester     ┃ Direction  ┃    I2C_2 Tested     ┃
 *        ┃    Master           ┃            ┃    Slave            ┃
 *        ┣━━━━━━━━━━┳━━━━━━━━━━╋━━━━━━━━━━━━╋━━━━━━━━━━┳━━━━━━━━━━┫
 *        ┃   SCL    ┃   PB8    ┃    ▬▬▬▶    ┃   PB10   ┃   SCL    ┃
 *        ┣━━━━━━━━━━╋━━━━━━━━━━╋━━━━━━━━━━━━╋━━━━━━━━━━╋━━━━━━━━━━┫
 *        ┃   SDA    ┃   PB9    ┃    ◀▬▬▶    ┃   PB11   ┃   SDA    ┃
 *        ┗━━━━━━━━━━┻━━━━━━━━━━┻━━━━━━━━━━━━┻━━━━━━━━━━┻━━━━━━━━━━┛
 */

static SemaphoreHandle_t I2CTestSem;

/*
 * TestI2C(uint8_t NInt, uint8_t TestPattern[], uint8_t TestPatLen)
 *
 * returns "true" if the test passed. Otherwise returns "false".
 */
#define SALVE_ADDR 0xAA
bool TestI2C(uint8_t NInt, uint8_t TestPattern[], uint8_t TestPatLen)
 {
  uint8_t RecvPat[MAX_TEST_PATTERN_SIZE];
  uint8_t MedTestPattern[MAX_TEST_PATTERN_SIZE];
  bool TestResult;
  uint8_t i;
  uint32_t MaxTimeToWait;
  MaxTimeToWait = MAX(10, (TestPatLen * 1000) / MIN_I2C_FREQUENCY);

  xSemaphoreTake(I2CTestSem, 0);  /* Setting semaphore to taken state without any waiting to ensure that after transferring the program will wait for interrupt. */

  for(i = 0; i < NInt; i++)
   {
    TestResult = !HAL_I2C_Slave_Receive_DMA(&INSPECTED_I2C, MedTestPattern, TestPatLen);               /* Waiter    (Slave)  */
    if(!TestResult) break;
    TestResult = !HAL_I2C_Master_Transmit_DMA(&INSPECTOR_I2C, SALVE_ADDR, TestPattern, TestPatLen);    /* Activator (Master) */
    if(!TestResult) break;
    TestResult = xSemaphoreTake(I2CTestSem, pdMS_TO_TICKS(MaxTimeToWait));
    if(!TestResult) break;
    TestResult = !HAL_I2C_Slave_Transmit_DMA(&INSPECTED_I2C, MedTestPattern, TestPatLen);              /* Waiter    (Slave)  */
    if(!TestResult) break;
    TestResult = !HAL_I2C_Master_Receive_DMA(&INSPECTOR_I2C, SALVE_ADDR, RecvPat, TestPatLen);         /* Activator (Master) */
    if(!TestResult) break;
    TestResult = xSemaphoreTake(I2CTestSem, pdMS_TO_TICKS(MaxTimeToWait));
    if(!TestResult) break;
    TestResult = !memcmp(TestPattern, MedTestPattern, TestPatLen);
    if(!TestResult) break;
   }

/*
 * Aborting DMA and Reinitializing the Inspector and Inspected I2Cs before the "MedTestPattern[]" and "RecvPat[]"
 * are disappeared on exit to prevent accessing to the incorrect memory.
 */
  HAL_DMA_Abort(INSPECTOR_I2C.hdmarx);
  HAL_DMA_Abort(INSPECTOR_I2C.hdmatx);

  HAL_DMA_Abort(INSPECTED_I2C.hdmarx);
  HAL_DMA_Abort(INSPECTED_I2C.hdmatx);

  HAL_I2C_DeInit(&INSPECTOR_I2C);
  HAL_I2C_DeInit(&INSPECTED_I2C);

  HAL_I2C_Init(&INSPECTOR_I2C);
  HAL_I2C_Init(&INSPECTED_I2C);

  return TestResult;
 }



void HAL_I2C_RxCpltCallback(I2C_HandleTypeDef *hi2c)
{
  static BaseType_t xHigherPriorityTaskWoken;
  xHigherPriorityTaskWoken = pdFALSE;

  if( (hi2c == &INSPECTOR_I2C) || (hi2c == &INSPECTED_I2C) )
   {
    xSemaphoreGiveFromISR(I2CTestSem, &xHigherPriorityTaskWoken);
   }

}


void HAL_I2C_MasterRxCpltCallback(I2C_HandleTypeDef *hi2c)
 {
  HAL_I2C_RxCpltCallback(hi2c);
 }

void HAL_I2C_SlaveRxCpltCallback(I2C_HandleTypeDef *hi2c)
 {
  HAL_I2C_RxCpltCallback(hi2c);
 }


/*
 * *************************************************************************************************************
 **          ADC Test Functions
 * *************************************************************************************************************
*/
/**
 *        ┏━━━━━━━━━━━━━━━━━━━━━┳━━━━━━━━━━━━┳━━━━━━━━━━━━━━━━━━━━━┓
 *        ┃     DAC Tester      ┃ Direction  ┃  ADC_1 Ch_0 Tested  ┃
 *        ┣━━━━━━━━━━┳━━━━━━━━━━╋━━━━━━━━━━━━╋━━━━━━━━━━┳━━━━━━━━━━┫
 *        ┃   Aout   ┃   PA4    ┃    ▬▬▬▶    ┃   PA0    ┃   Ain    ┃
 *        ┗━━━━━━━━━━┻━━━━━━━━━━┻━━━━━━━━━━━━┻━━━━━━━━━━┻━━━━━━━━━━┛
 */

#define REF_INT_VOLT       1180 /* mV     1.18v ⩽ Vrefint ⩽ 1.24v */
#define MAX_ROUGH_VOLTAGE ((1<<12))

#define VREFIN_CAL        (*(VREFINT_CAL_ADDR_CMSIS))

static SemaphoreHandle_t ADCTestSem;

#define ADC_PERMITTED_ERROR 30  /* mV */

bool TestADC(uint8_t NInt, int32_t TestVoltage)
 {
  bool TestResult = true;
  uint8_t i;
  uint32_t MaxTimeToWait;
  uint32_t RoughVoltageDAC;
  uint32_t RoughVrefInt = 0, ADCRoughData = 0;
  int32_t VrefInt, ADCVoltage;
  ADC_ChannelConfTypeDef VrefintChannel = {.Channel = ADC_CHANNEL_VREFINT, .Offset = 0, .Rank = ADC_REGULAR_RANK_1, .SamplingTime = ADC_SAMPLETIME_15CYCLES};
  ADC_ChannelConfTypeDef TerstedChannel = {.Channel = ADC_CHANNEL_0,       .Offset = 0, .Rank = ADC_REGULAR_RANK_1, .SamplingTime = ADC_SAMPLETIME_15CYCLES};
  MaxTimeToWait = 10;  // Must be rechecked.

  xSemaphoreTake(ADCTestSem, 0);  /* Setting semaphore to taken state without any waiting to ensure that after transferring the program will wait for interrupt. */

  TestResult = !HAL_ADC_ConfigChannel(&INSPECTED_ADC, &VrefintChannel);
  if(!TestResult) return TestResult;
  TestResult = !HAL_ADC_Start_DMA(&INSPECTED_ADC, &RoughVrefInt, 1);
  if(!TestResult) return TestResult;

  TestResult = xSemaphoreTake(ADCTestSem, pdMS_TO_TICKS(MaxTimeToWait));
  TestResult &= !HAL_ADC_Stop_DMA(&INSPECTED_ADC);
  if(!TestResult) return TestResult;

  VrefInt = DIV_RND(3300 * VREFIN_CAL , RoughVrefInt);
  RoughVoltageDAC = DIV_RND(TestVoltage*MAX_ROUGH_VOLTAGE, VrefInt);

  TestResult = !HAL_DAC_Start_DMA(&INSPECTOR_DAC, DAC_CHANNEL_1, &RoughVoltageDAC, 1, DAC_ALIGN_12B_R);
  if(!TestResult) return TestResult;
  TestResult &= !HAL_ADC_ConfigChannel(&INSPECTED_ADC, &TerstedChannel);
  TestResult &= xSemaphoreTake(ADCTestSem, pdMS_TO_TICKS(MaxTimeToWait));
  if(TestResult)
   for(i = 0; i < NInt; i++)
    {

     TestResult = !HAL_ADC_Start_DMA(&INSPECTED_ADC, &ADCRoughData, 1);
     if(!TestResult) break;
     TestResult = xSemaphoreTake(ADCTestSem, pdMS_TO_TICKS(MaxTimeToWait));
     /* if(!TestResult) break;  --  No need to break the loop here because it is needed previously to run the function "HAL_ADC_Stop_DMA()" anyway. */
     TestResult &= !HAL_ADC_Stop_DMA(&INSPECTED_ADC);  /* Here is needed to make anding to the previous TestResult anyway to break the loop here if the test result was "false" before. */
     if(!TestResult) break;
     ADCVoltage = DIV_RND(ADCRoughData * VrefInt, MAX_ROUGH_VOLTAGE);
     TestResult = abs(ADCVoltage - TestVoltage) <= ADC_PERMITTED_ERROR;
     if(!TestResult) break;

    }
  TestResult &= !HAL_DAC_Stop_DMA(&INSPECTOR_DAC, DAC_CHANNEL_1);


  return TestResult;
 }


void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
 {
  static BaseType_t xHigherPriorityTaskWoken;
  xHigherPriorityTaskWoken = pdFALSE;

  if(hadc == &INSPECTED_ADC)
   {
    xSemaphoreGiveFromISR(ADCTestSem, &xHigherPriorityTaskWoken);
   }
 }




void HAL_DAC_ConvCpltCallbackCh1(DAC_HandleTypeDef *hdac)
 {
  static BaseType_t xHigherPriorityTaskWoken;
  DAC_HandleTypeDef *inspector_dac_p;
  {
   /*
    * Attention !
    * Because with the name "hdac" is defined the global variable in dac.c ("DAC_HandleTypeDef hdac;") to which
    * is referred the "INSPECTOR_DAC" definition and the local variable given as the parameter in this function
    * ( "(DAC_HandleTypeDef *hdac)" ) in this functions is used special pointer to the global variable
    * "DAC_HandleTypeDef *inspector_dac_p;" to enable using the global variable shadowed by the local.
    * None of these variables neither local nor global cannot be simply renamed because the "dac.c" library
    * and the header of these function "void HAL_DAC_ConvCpltCallbackCh1(DAC_HandleTypeDef *hdac);" are
    * originally defined by the HAL Cube MX and their names shouldn't be changed otherwise it would cause
    * the problem after regenerating this project by the CobeMX or by user for consistent definition.
    */
   extern DAC_HandleTypeDef INSPECTOR_DAC;
   inspector_dac_p = &INSPECTOR_DAC;
  }
  xHigherPriorityTaskWoken = pdFALSE;
  if(hdac == inspector_dac_p)
   {
    xSemaphoreGiveFromISR(ADCTestSem, &xHigherPriorityTaskWoken);
   }
 }


/*
 * *************************************************************************************************************
 **          Timer Test Functions
 * *************************************************************************************************************
*/
/**
 *        ┏━━━━━━━━━━━━━━━━━━━━━┳━━━━━━━━━━━━┳━━━━━━━━━━━━━━━━━━━━━┓
 *        ┃ Timer_1 Ch_1 Tester ┃ Direction  ┃ Timer_2 Ch_2 Tested ┃
 *        ┣━━━━━━━━━━┳━━━━━━━━━━╋━━━━━━━━━━━━╋━━━━━━━━━━┳━━━━━━━━━━┫
 *        ┃ PWM out  ┃   PE9    ┃    ▬▬▬▶    ┃   PB3    ┃  PWM in  ┃
 *        ┗━━━━━━━━━━┻━━━━━━━━━━┻━━━━━━━━━━━━┻━━━━━━━━━━┻━━━━━━━━━━┛
 */

static SemaphoreHandle_t TIMTestSem;

#define TIM_PERMITTED_ERROR  2

static int32_t MeasuredTime = 0;
static uint16_t cnt;
static uint16_t NPerPairs = 2;


bool TestTimer(uint8_t NInt, uint32_t TestTime)
 {
  bool TestResult = true;
  uint8_t i;
  uint32_t MaxTimeToWait;
  if(TestTime <= 4)
   return false;
  MaxTimeToWait = MAX(10, DIV_RND_UP(TestTime * 4, 1000));  // Must be rechecked.
  NPerPairs = 2*MAX(1, 1000/TestTime);
  xSemaphoreTake(TIMTestSem, 0);  /* Setting semaphore to taken state without any waiting to ensure that after transferring the program will wait for interrupt. */

  TestResult = !HAL_TIM_PWM_Start(&INSPECTOR_TIM,INSPECTOR_TIM_CHANNEL);
  if(!TestResult) return TestResult;
  SetPeriod(&INSPECTOR_TIM,INSPECTOR_TIM_CHANNEL, TestTime);

  cnt = 0;
  TestResult &= !HAL_TIM_IC_Start_IT(&INSPECTED_TIM, INSPECTED_TIM_CHANNEL);
  if(TestResult)
   for(i = 0; i < NInt; i++)
    {
     cnt = 0;
     TestResult = xSemaphoreTake(TIMTestSem, pdMS_TO_TICKS(MaxTimeToWait));
     if(!TestResult) break;
     TestResult = (abs(TestTime - MeasuredTime) < TIM_PERMITTED_ERROR);
     if(!TestResult) break;
    }

  TestResult &= !HAL_TIM_IC_Stop_IT(&INSPECTED_TIM, INSPECTED_TIM_CHANNEL);
  TestResult &= !HAL_TIM_PWM_Stop(&INSPECTOR_TIM,INSPECTOR_TIM_CHANNEL);

  return TestResult;
 }





void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
 {
  static int32_t v[2];
  static bool state = false;
  static BaseType_t xHigherPriorityTaskWoken;
  xHigherPriorityTaskWoken = pdFALSE;

  if(htim == &INSPECTED_TIM)
   {
    if(htim->Channel == INSPECTED_TIM_ACTIVE_CHANNEL)
     {
      v[state] =  HAL_TIM_ReadCapturedValue(htim, INSPECTED_TIM_CHANNEL);
      state = !state;
      MeasuredTime = abs(v[1]-v[0]);
      cnt++;
      if(cnt >= NPerPairs)
       {
        xSemaphoreGiveFromISR(TIMTestSem, &xHigherPriorityTaskWoken);
        cnt = 0;
       }
     }
   }
 }



/*
 * *************************************************************************************************************
 **          Common Devices Test Functions
 * *************************************************************************************************************
*/

/*
 * void DevTestTask(void *pvParameters)
 * The task-procedure for testing peripherals..
 * It is implemented in several (5) separate tasks when each one of them tests one of the next peripherals:
 * UART, SPI, I2C, ADC or Timer. This procedure contains the common algorithm for all the task tester while
 * the specific task parts of algorithm exist in separate functions selected in the switch cases.
 * Because this procedure is running in several (5) copies - to use the static variables for specific task
 * is not possible. Instead of it any required task-specific data can be given via the parameter "void *pvParameters".
 */

void DevTestTask(void *pvParameters)
 {
#ifdef DEBUG
  char const * const OwnTaskName = pcTaskGetName(NULL); // Is defined to make debugging easier.
  UNUSED(OwnTaskName);
#endif
  DevTaskParams_s *DevTaskParams = pvParameters;
  DevTestMesg_s Message;
  bool Result = false;
  for(;;)
   {
    if(pdPASS == xQueueReceive(DevTaskParams->DevTestQue, &Message, portMAX_DELAY))
     {
      switch(DevTaskParams->PeriphType)
       {
        case E_UART:
          Result = TestUART(Message.TestInfo->Num_Interations, Message.TestInfo->TestPattern, Message.TestInfo->Bit_Pattern_Length);
         break;
        case E_SPI:
          Result = TestSPI(Message.TestInfo->Num_Interations, Message.TestInfo->TestPattern, Message.TestInfo->Bit_Pattern_Length);
         break;
        case E_I2C:
          Result = TestI2C(Message.TestInfo->Num_Interations, Message.TestInfo->TestPattern, Message.TestInfo->Bit_Pattern_Length);
         break;
        case E_ADC:
          Result = TestADC(Message.TestInfo->Num_Interations, Message.TestInfo->TestVoltage);
         break;
        case E_TIMER:
          Result = TestTimer(Message.TestInfo->Num_Interations, Message.TestInfo->TestTime);
         break;
        default:
          Result = false;
         break;
       }
      rtprintf("\n\r%sThe %s%s%s Test %s%s%s\n\r", MessColor, DevsColor, PeriphNames[DevTaskParams->PeriphType], MessColor,ResultColors[Result], ResultMessages[Result], TermColorsReset);
      SendTestResponse(Result, DevTaskParams->PeriphType);
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
/*----------------------------------------------------------------------------------------------------------------------*/
/*   Prepares the tester-tasks.                                                                                         */
void TesterInit()
 {
  BaseType_t result;
  uint8_t i;

  TestReqQueue = xQueueCreate(TEST_REQ_QUEUE_LEN, sizeof(TestReqMesg_s));
  if(TestReqQueue == NULL)
   {
    rtprintf("\n\r%sThe test request queue couldn't be created.\n\rHalting !!!%s\n\r", TermRed, TermColorsReset);
    for(;;);
   }
  TestRespQueue = xQueueCreate(TEST_RESP_QUEUE_LEN, sizeof(TestRespMesg_s));
  if(TestReqQueue == NULL)
   {
    rtprintf("\n\r%sThe test response queue couldn't be created.\n\rHalting !!!%s\n\r", TermRed, TermColorsReset);
    for(;;);
   }
  RespQueueMut = xSemaphoreCreateMutex();
  if(RespQueueMut == NULL)
   {
    rtprintf("\n\r%sThe test-response-queue-mutex couldn't be created.\n\rHalting !!!%s\n\r", TermRed, TermColorsReset);
    for(;;);
   }
  xSemaphoreGive(RespQueueMut);

  result = xTaskCreate(TesterTask, "Tester Task", xTESTER_STACK_SIZE, NULL, xTESTER_PRIORITY, &xTesterTaskHandle);
  if(result != pdPASS)
   {
	rtprintf("\n\r%sThe tester task couldn't be created.\n\rHalting !!!%s\n\r", TermRed, TermColorsReset);
	for(;;);
   }


  char TaskName[configMAX_TASK_NAME_LEN];

  for(i = 0; i < E_NUM_PERIPHS; i++)
   {
    DevTaskParams[i].PeriphType = i;
    DevTaskParams[i].DevTestQue = xQueueCreate(DEV_TEST_QUEUE_LEN, sizeof(DevTestMesg_s));
    if(DevTaskParams[i].DevTestQue == NULL)
     {
      rtprintf("\n\r%sThe %s Test Queue couldn't be created.\n\rHalting !!!%s\n\r", TermRed, PeriphNames[i], TermColorsReset);
      for(;;);
     }

    snprintf(TaskName, sizeof(TaskName), "%s Test Task", PeriphNames[i]);
    result = xTaskCreate(DevTestTask, TaskName, xDEV_TEST_STACK_SIZE, &DevTaskParams[i], xTESTER_PRIORITY, &xDevTestTaskHandles[i]);
    if(result != pdPASS)
     {
      rtprintf("\n\r%sThe %s Test Task couldn't be created.\n\rHalting !!!%s\n\r", TermRed, PeriphNames[i], TermColorsReset);
      for(;;);
     }

   }


 /*       UART Specific Parameters Creating             */

  UARTTestSem = xSemaphoreCreateBinary();
  if(UARTTestSem == NULL)
   {
    rtprintf("\n\r%sThe UART Test Semaphore couldn't be created.\n\rHalting !!!%s\n\r", TermRed, TermColorsReset);
    for(;;);
   }


  /*       SPI Specific Parameters Creating             */

  SPITestSem = xSemaphoreCreateBinary();
  if(SPITestSem == NULL)
   {
    rtprintf("\n\r%sThe SPI Test Semaphore couldn't be created.\n\rHalting !!!%s\n\r", TermRed, TermColorsReset);
    for(;;);
   }


  /*       I2C Specific Parameters Creating             */

  I2CTestSem = xSemaphoreCreateBinary();
  if(I2CTestSem == NULL)
   {
    rtprintf("\n\r%sThe I2C Test Semaphore couldn't be created.\n\rHalting !!!%s\n\r", TermRed, TermColorsReset);
    for(;;);
   }


  /*       ADC Specific Parameters Creating             */

  ADCTestSem = xSemaphoreCreateBinary();
  if(ADCTestSem == NULL)
   {
    rtprintf("\n\r%sThe ADC Test Semaphore couldn't be created.\n\rHalting !!!%s\n\r", TermRed, TermColorsReset);
    for(;;);
   }


  /*       Timer Specific Parameters Creating             */

  TIMTestSem = xSemaphoreCreateBinary();
  if(TIMTestSem == NULL)
   {
    rtprintf("\n\r%sThe Timer Test Semaphore couldn't be created.\n\rHalting !!!%s\n\r", TermRed, TermColorsReset);
    for(;;);
   }


 }

/*======================================================================================================================*/

/*----------------------------------------------------------------------------------------------------------------------*/
/*  Prints Test pattern for UART, SPI and/or I2C if anyone of them is requested for test.                               */
void PrintPattern(uint32_t Len, uint8_t Pattern[])
 {
  uint32_t i;
  rtprintf("%sThe test pattern is: %s0x%s", MessColor, ValuesColor,TermColorsReset);
  for(i = 0; i < Len; i++)
   rtprintf("%s%02X%s", ValuesColor, Pattern[i], TermColorsReset);
  rtprintf("\n\r");

 }

/*----------------------------------------------------------------------------------------------------------------------*/
/*  Prints the voltage for ADC if the ADC is requested for test.                                                        */
void PrintVoltage(int32_t TestVoltage)
 {
  rtprintf("%sThe voltage is: %s%d%smV%s\n\r", MessColor, ValuesColor, TestVoltage, UnitsColor, TermColorsReset);
 }

/*----------------------------------------------------------------------------------------------------------------------*/
/*  Prints the time for timer if the timer is requested for test.                                                       */
void PrintTime(uint32_t TestTime)
 {
  rtprintf("%sThe time is: %s%d%sms%s\n\r", MessColor, ValuesColor, TestTime, UnitsColor, TermColorsReset);
 }

/*----------------------------------------------------------------------------------------------------------------------*/
/*  Prints the peripherals are requested for test.                                                                      */
void PrintPeripherals(PeriphBitField_s SelectedPeripherals)
 {
  PeriphType_e i;
  bool FirstDevPrinted = false;
  rtprintf("%sThe devices under test: %s", MessColor, TermColorsReset);
  for(i = 0, FirstDevPrinted = false; i < E_NUM_PERIPHS; ++i)
   {
    if( ( (*(uint8_t*)&SelectedPeripherals) >> i) & 0x01 )
     {
      if((i > 0) && FirstDevPrinted)
       {
        rtprintf("%s,%s ", MessColor, TermColorsReset);
       }
      rtprintf("%s%s%s", DevsColor, PeriphNames[i], TermColorsReset);
      FirstDevPrinted = true;
     }
   }
  rtprintf("%s.%s\n\r", MessColor, TermColorsReset);
 }

/*----------------------------------------------------------------------------------------------------------------------*/
/*  Prints number of times the requested devices must be tested                                                         */
void PrintNumInterations(uint8_t Num_Interations)
 {
  rtprintf("%sThe number of interations is: %s%d%s\n\r", MessColor, UnitsColor, Num_Interations, TermColorsReset);
 }

/*----------------------------------------------------------------------------------------------------------------------*/
/*  Prints Test Information: Number of interations , Requested devices for test and optionally:                         */
/*  Test Pattern for ports , Test Voltage for ADC and Test Time for Timer                                               */
void PrintTestInfo(TestData_s TestData, uint8_t TestPattern[])
 {
  uint8_t Periph;
  Periph = (*(uint8_t*)&TestData.Periph_B_F);

  PrintPeripherals(TestData.Periph_B_F);
  PrintNumInterations(TestData.Num_Interations);

  if( Periph & ( UART_Flag | SPI_Flag | I2C_Flag ) )
   {
    PrintPattern(TestData.Bit_Pattern_Length , TestPattern);
   }
  if( Periph | ADC_Flag)
   {
    PrintVoltage(TestData.TestVoltage);
   }
  if( Periph | Timer_Flag)
   {
    PrintTime(TestData.TestTime);
   }
 }

/*======================================================================================================================*/

