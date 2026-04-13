/*
 * Tester.c
 *
 *  Created on: 13 Apr 2026
 *      Author: boris
 */

#include "Tester.h"

#include <string.h>

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




typedef struct TestReqMesg_s
 {
  TestData_s TestData;
  uint8_t TestPattern[MAX_TEST_PATTERN_SIZE];
 }TestReqMesg_s;






TaskHandle_t xTesterTaskHandle;      // Handle to network task.


QueueHandle_t TestReqQueue;




void MakeTest(TestData_s TestData, uint8_t TestPattern[]);







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

 }







