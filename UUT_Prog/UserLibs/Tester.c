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



#define xTESTER_PRIORITY       0                                          // Priority of the tester tasks.
#define xTESTER_STACK_SIZE     MAX(configMINIMAL_STACK_SIZE, 1024)        // Stack size of the main-test-task.


#define xDEV_TEST_STACK_SIZE   MAX(configMINIMAL_STACK_SIZE, 512)         // Stack size of the single-dev-test-task.
#define DEV_TEST_QUEUE_LEN     3 /*Will be changed to 1 in the future.*/  // Length of the queue from the network-task to the main-test-task.

#define TEST_REQ_QUEUE_LEN     5                                          // Length of the queue of requests from the main-test-task to the single-dev-test-task.
#define TEST_RESP_QUEUE_LEN   (DEV_TEST_QUEUE_LEN * E_NUM_PERIPHS)        // Length of the queue of responses from the single-dev-test-task to the main-test-task.




#define INSPECTOR_UART  huart6
#define INSPECTED_UART  huart4

#define INSPECTOR_SPI  hspi1
#define INSPECTED_SPI  hspi4

#define INSPECTOR_I2C  hi2c1
#define INSPECTED_I2C  hi2c2

#define INSPECTOR_DAC  hdac
#define INSPECTED_ADC  hadc1


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
  uint32_t TestVoltage;
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


TaskHandle_t xTesterTaskHandle;                   // Handle to Tester      task.

TaskHandle_t xDevTestTaskHandles[E_NUM_PERIPHS];  // Handles to UART, SPI,I2C, ADC and Timer tasks.  -- Will be implemented in the future.

QueueHandle_t TestReqQueue;
QueueHandle_t TestRespQueue;
static SemaphoreHandle_t RespQueueMut;



void MakeTest(DevTestInfo_s *DevTestInfo, PeriphBitField_s Periph_B_F);

void ReqDevTest(DevTestInfo_s *DevTestInfo, PeriphType_e PeriphType);



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
  static DevTestInfo_s DevTestInfo = {0};

  for(;;)
   {
    if(pdPASS == xQueueReceive(TestReqQueue, &ReqMsg, portMAX_DELAY))
     {
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
        if(pdPASS == xQueueReceive(TestRespQueue, &RespMsg, portMAX_DELAY))
         {

          *(uint8_t*)&RespondedDevs |= (1 << RespMsg.DevType);                                                // Marking the device as responded.
          *(uint8_t*)&DevResults |= ((1 << RespMsg.DevType))*((uint8_t)RespMsg.Result);                       // Marking the device's answer
          if(!memcmp((void*)&RespondedDevs, (void*)&(ReqMsg.TestData.Periph_B_F), sizeof(PeriphBitField_s)))  // All requested devices responded.
           break;
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



void MakeTest(DevTestInfo_s *DevTestInfo, PeriphBitField_s Periph_B_F)
 {
  uint8_t i;
  uint8_t PeriphFlags = *(uint8_t*)&Periph_B_F;
  for(i = 0; i < E_NUM_PERIPHS; i++)
   {
    if((PeriphFlags >> i)&0x01)
     ReqDevTest(DevTestInfo, i);
   }
 }

DevTaskParams_s DevTaskParams[E_NUM_PERIPHS] = {0};

void ReqDevTest(DevTestInfo_s *DevTestInfo, PeriphType_e PeriphType)
 {
  DevTestMesg_s Message;
  Message.TestInfo = DevTestInfo;
  xQueueSend(DevTaskParams[PeriphType].DevTestQue, &Message, pdMS_TO_TICKS(10));
 }








/*
 * *************************************************************************************************************
 **          UART Test Functions
 * *************************************************************************************************************
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

//void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
// {
//  UNUSED(huart);
//  volatile int a = 2;
//  UNUSED(a);
// }



/*
 * *************************************************************************************************************
 **          SPI Test Functions
 * *************************************************************************************************************
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
    xSemaphoreTake(SPITestSem, 0);  // Setting semaphore to taken state without any waiting to ensure that after transferring the program will wait for interrupt.

    TestResult = !HAL_SPI_TransmitReceive_DMA(&INSPECTED_SPI, EmptyTestPattern, MedTestPattern, TestPatLen);  // Receiving    // Waiter    (Slave)
    if(!TestResult) break;
    TestResult = !HAL_SPI_TransmitReceive_DMA(&INSPECTOR_SPI, TestPattern, EmptyTestPattern, TestPatLen);    // Transmitting  // Activator (Master)
    if(!TestResult) break;
    TestResult = xSemaphoreTake(SPITestSem, pdMS_TO_TICKS(MaxTimeToWait));
    if(!TestResult) break;
    TestResult = !HAL_SPI_TransmitReceive_DMA(&INSPECTED_SPI, MedTestPattern, EmptyTestPattern, TestPatLen); // Transmitting  // Waiter    (Slave)
    if(!TestResult) break;
    TestResult = !HAL_SPI_TransmitReceive_DMA(&INSPECTOR_SPI, EmptyTestPattern, RecvPat, TestPatLen);        // Receiving     // Activator (Master)
    if(!TestResult) break;
    TestResult = xSemaphoreTake(SPITestSem, pdMS_TO_TICKS(MaxTimeToWait));
    if(!TestResult) break;
    TestResult = !memcmp(TestPattern, MedTestPattern, TestPatLen);
    if(!TestResult) break;
   }

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

//void HAL_SPI_ErrorCallback(SPI_HandleTypeDef *hspi)
// {
//  volatile uint32_t error;
//  error = HAL_SPI_GetError(hspi);
//  switch((uint32_t)hspi->Instance)
//   {
//    case (uint32_t)SPI1:
//     break;
//    case (uint32_t)SPI4:
//     break;
//   }
//  switch(error)
//   {
//    case HAL_SPI_ERROR_NONE:              //(0x00000000U)   /*!< No error                               */
//     break;
//    case HAL_SPI_ERROR_MODF:              //(0x00000001U)   /*!< MODF error                             */
//     break;
//    case HAL_SPI_ERROR_CRC:               //(0x00000002U)   /*!< CRC error                              */
//     break;
//    case HAL_SPI_ERROR_OVR:               //(0x00000004U)   /*!< OVR error                              */
//     break;
//    case HAL_SPI_ERROR_FRE:               //(0x00000008U)   /*!< FRE error                              */
//     break;
//    case HAL_SPI_ERROR_DMA:               //(0x00000010U)   /*!< DMA transfer error                     */
//     break;
//    case HAL_SPI_ERROR_FLAG:              //(0x00000020U)   /*!< Error on RXNE/TXE/BSY/FTLVL/FRLVL Flag */
//     break;
//    case HAL_SPI_ERROR_ABORT:             //(0x00000040U)   /*!< Error during SPI Abort procedure       */
//     break;
//   }
//  __HAL_SPI_CLEAR_OVRFLAG(hspi);
// }


/*
 * *************************************************************************************************************
 **          I2C Test Functions
 * *************************************************************************************************************
*/


static SemaphoreHandle_t I2CTestSem;

/*
 * TestI2C(uint8_t NInt, uint8_t TestPattern[], uint8_t TestPatLen)
 *
 * returns "true" if the test passed. Otherwise returns "false".
 */
#define SALVE_ADDR 0xAA  //0x55
bool TestI2C(uint8_t NInt, uint8_t TestPattern[], uint8_t TestPatLen)
 {
  uint8_t RecvPat[MAX_TEST_PATTERN_SIZE];
  uint8_t MedTestPattern[MAX_TEST_PATTERN_SIZE];
  bool TestResult;
  uint8_t i;
  uint32_t MaxTimeToWait;
  MaxTimeToWait = MAX(10, (TestPatLen * 1000) / MIN_I2C_FREQUENCY);

  for(i = 0; i < NInt; i++)
   {
    TestResult = !HAL_I2C_Slave_Receive_DMA(&INSPECTED_I2C, MedTestPattern, TestPatLen);  // Waiter    (Slave)
    if(!TestResult) break;
    TestResult = !HAL_I2C_Master_Transmit_DMA(&INSPECTOR_I2C, SALVE_ADDR, TestPattern, TestPatLen);    // Activator (Master)
    if(!TestResult) break;
    TestResult = xSemaphoreTake(I2CTestSem, pdMS_TO_TICKS(MaxTimeToWait));
    if(!TestResult) break;
    TestResult = !HAL_I2C_Slave_Transmit_DMA(&INSPECTED_I2C, MedTestPattern, TestPatLen); // Waiter    (Slave)
    if(!TestResult) break;
    TestResult = !HAL_I2C_Master_Receive_DMA(&INSPECTOR_I2C, SALVE_ADDR, RecvPat, TestPatLen);         // Activator (Master)
    if(!TestResult) break;
    TestResult = xSemaphoreTake(I2CTestSem, pdMS_TO_TICKS(MaxTimeToWait));
    if(!TestResult) break;
    TestResult = !memcmp(TestPattern, MedTestPattern, TestPatLen);
    if(!TestResult) break;
   }

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

//void HAL_I2C_ErrorCallback(I2C_HandleTypeDef *hi2c)
// {
//  UNUSED(hi2c);
//  volatile int a = 2;
//  UNUSED(a);
// }


/*
 * *************************************************************************************************************
 **          ADC Test Functions
 * *************************************************************************************************************
*/



#define REF_INT_VOLT       1180 // mV     1.18v ⩽ Vrefint ⩽ 1.24v
#define MAX_ROUGH_VOLTAGE ((1<<12) - 1)

#define VREFIN_CAL    (*(VREFINT_CAL_ADDR_CMSIS))

//VREFINT_CAL_ADDR_CMSIS
static SemaphoreHandle_t ADCTestSem;



bool TestADC(uint8_t NInt, uint32_t TestVoltage)
 {
  bool TestResult;
  uint8_t i;
  uint32_t MaxTimeToWait;
  uint32_t RoughVoltageDAC;
  uint32_t RoughVrefInt = 0, ADCRoughData = 0;
  uint32_t VrefInt;
  ADC_ChannelConfTypeDef TerstedChannel = {.Channel = ADC_CHANNEL_0,       .Offset = 0, .Rank = ADC_REGULAR_RANK_1, .SamplingTime = ADC_SAMPLETIME_15CYCLES};
  ADC_ChannelConfTypeDef VrefintChannel = {.Channel = ADC_CHANNEL_VREFINT, .Offset = 0, .Rank = ADC_REGULAR_RANK_1, .SamplingTime = ADC_SAMPLETIME_15CYCLES};
  //uint32_t RoughVoltageADC;
  MaxTimeToWait = 10;  // Must be rechecked.

  //RoughVoltageDAC = TestVoltage*0x1000/3243+29;  //  TestVoltage*0x1000/3200
  xSemaphoreTake(ADCTestSem, 0);  // Setting semaphore to taken state without any waiting to ensure that after transferring the program will wait for interrupt.
  for(i = 0; i < NInt; i++)
   {
    TestResult = !HAL_ADC_ConfigChannel(&INSPECTED_ADC, &VrefintChannel);
    TestResult = !HAL_ADC_Start_DMA(&INSPECTED_ADC, &RoughVrefInt, 1);
    TestResult = xSemaphoreTake(ADCTestSem, pdMS_TO_TICKS(MaxTimeToWait));
    TestResult = !HAL_ADC_Stop_DMA(&INSPECTED_ADC);

    VrefInt = DIV_RND(3300 * VREFIN_CAL , RoughVrefInt);

    RoughVoltageDAC = DIV_RND(TestVoltage*(1<<12), VrefInt);
    TestResult = !HAL_DAC_SetValue(&INSPECTOR_DAC, DAC_CHANNEL_1, DAC_ALIGN_12B_R, RoughVoltageDAC);
    if(!TestResult) break;
    TestResult = !HAL_DAC_Start(&INSPECTOR_DAC, DAC_CHANNEL_1);
    if(!TestResult) break;

    TestResult = !HAL_ADC_ConfigChannel(&INSPECTED_ADC, &TerstedChannel);
    vTaskDelay(1);
    TestResult = !HAL_ADC_Start_DMA(&INSPECTED_ADC, &ADCRoughData, 1);
    if(!TestResult) break;
    TestResult = xSemaphoreTake(ADCTestSem, pdMS_TO_TICKS(MaxTimeToWait));
    if(!TestResult) break;
    TestResult = !HAL_ADC_Stop_DMA(&INSPECTED_ADC);
    if(!TestResult) break;

    TestResult = !HAL_DAC_Stop(&INSPECTOR_DAC, DAC_CHANNEL_1);
    if(!TestResult) break;
   }
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


uint32_t DAC_VoltsToRoghData(uint32_t Volts, VoltsConvMethod_e ConvMethod)
 {
  switch(ConvMethod)
   {
    case E_IDEAL_CONV:
     break;
    case E_LINEAR_CONV:
     break;
    case E_POLYNOM_CONV:
     break;
   }
 }
uint32_t ADC_RoghDataToVolts(uint32_t RoghData, VoltsConvMethod_e ConvMethod)
 {

 }

/*
 * *************************************************************************************************************
 **          Timer Test Functions
 * *************************************************************************************************************
*/


bool TestTimer(uint8_t NInt, uint32_t TestTime)
 {
  return false;
 }

/*
 * *************************************************************************************************************
 **          Common Devices Test Functions
 * *************************************************************************************************************
*/
#define DevsColor   TermBlue //TermYello
#define ValuesColor TermMagenta
#define UnitsColor  TermCyan

void DevTestTask(void *pvParameters)//void DevTestTask(DevTaskParams_s const * const pvParameters)
 {
#ifdef DEBUG
  char const * const OwnTaskName = pcTaskGetName(NULL); // Is defined to make debugging easier.
  UNUSED(OwnTaskName);
#endif
  DevTaskParams_s *DevTaskParams = pvParameters;
  DevTestMesg_s Message;
  bool Result = false;
//  char buf[MAX_TEST_PATTERN_SIZE*2+50];
//  char *bpnt;
  int i;
  for(;;)
   {
    if(pdPASS == xQueueReceive(DevTaskParams->DevTestQue, &Message, portMAX_DELAY))
     {
      rtprintf("\n\r%sStarting %s Test...%s\n\r", TermBlue, PeriphNames[DevTaskParams->PeriphType], TermColorsReset);
      switch(DevTaskParams->PeriphType)
       {
        case E_UART:
        case E_SPI:
        case E_I2C:
          rtprintf("%sThe test pattern is: %s0x%s", TermBlue, ValuesColor,TermColorsReset);
          for(i = 0; i < Message.TestInfo->Bit_Pattern_Length; i++)
           rtprintf("%s%02X%s", ValuesColor, Message.TestInfo->TestPattern[i], TermColorsReset);
          rtprintf("\n\r");
         break;
        case E_ADC:
          rtprintf("%sThe voltage is: %s%d%smV%s\n\r", TermBlue, ValuesColor, Message.TestInfo->TestVoltage, UnitsColor, TermColorsReset);
         break;
        case E_TIMER:
          rtprintf("%sThe time is: %s%d%sms%s\n\r", TermBlue, ValuesColor, Message.TestInfo->TestTime, UnitsColor, TermColorsReset);
         break;
        default:
         break;
       }

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
      rtprintf("\n\r%sThe %s Test %s%s%s\n\r", TermBlue, PeriphNames[DevTaskParams->PeriphType], ResultColors[Result], ResultMessages[Result], TermColorsReset);
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

 }




