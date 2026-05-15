#include "Network.h"

#include "CommonData.h"
#include "Tester.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "api.h"
#include "netif.h"
#include "dhcp.h"
#include "netbuf.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"
#include "semphr.h"
#include "FreeRTOSConfig.h"
#include "projdefs.h"

#include "SystemLib.h"


/*======================================================================================================================*/

#define xNETWORK_PRIORITY       0
#define xNETWORK_STACK_SIZE     MAX(configMINIMAL_STACK_SIZE, 512)

#define TEST_REPORT_QUEUE_LEN   5        /*  Length of the test report queue. */

/*======================================================================================================================*/

QueueHandle_t TestReportQueue;

TaskHandle_t xNewtworkTaskHandle;      /*  Handle to network task.          */

/*======================================================================================================================*/

/* Network task procedure. Receives message from computer, requests for the test, waits for the test results and sends back response with them. */
void NetworkTask(void *pvParameters);

/*  Waits until the network connection is established.                                                                  */
void Wait_for_DHCP(void);

/*  Sends response via the network.                                                                                     */
void SendResponse(struct netconn *conn, ip4_addr_t *ip4addr, uint16_t ipport, TestResult_s TestResult);


/*======================================================================================================================*/
/*  Prepares the network task.                                                                                          */
void NetworkInit()
 {
  BaseType_t result;


  TestReportQueue = xQueueCreate(TEST_REPORT_QUEUE_LEN, sizeof(TestResult_s));
  if(TestReportQueue == NULL)
   {
    rtprintf("\n\r%sThe test report queue couldn't be created.\n\rHalting !!!%s\n\r", TermRed, TermColorsReset);
    for(;;);
   }

  result = xTaskCreate(NetworkTask, "Network Task", xNETWORK_STACK_SIZE, NULL, xNETWORK_PRIORITY, &xNewtworkTaskHandle);
  if(result != pdPASS)
   {
	rtprintf("\n\r%sThe network task couldn't be created.\n\rHalting !!!%s\n\r", TermRed, TermColorsReset);
	for(;;);
   }

 }

/*----------------------------------------------------------------------------------------------------------------------*/
/* Network task procedure. Receives message from computer, requests for the test, waits for the test results and sends back response with them. */
void NetworkTask(void *pvParameters)
 {
  struct netconn *conn;
  struct netbuf *buf;
  void *data;
  uint16_t len;
  char *ip_str;
  ip_addr_t addr;
  uint16_t port;
  extern struct netif gnetif;
  struct netif *netif = &gnetif;
  ip4_addr_t const *remote_ip;
  err_t error;

  Wait_for_DHCP();
  netif_set_hostname(&gnetif, HOST_NAME);

  remote_ip = netif_ip4_addr(netif);
  ip_str = ip4addr_ntoa(remote_ip);
  rtprintf("IP Address Assigned: %s\n", ip_str);

  conn = netconn_new(NETCONN_UDP);
  if(conn != NULL)
   {
    error = netconn_bind(conn, NULL, DESTIN_PORT);

    if(error == ERR_OK)
     {
      for(;;)
       {
        error = netconn_recv(conn, &buf);
        if (error == ERR_OK)
         {
          /* Process received data in buf->p->payload */

          netbuf_data(buf, &data, &len);
          ip_str = ip4addr_ntoa(&buf->addr);
          addr = buf->addr;
          port = buf->port;
          rtprintf("%sReceived data from %s:%d\n\r%s", TermYello, ip_str, port, TermColorsReset);
          TestData_s TestData;
          uint8_t *TestPattern = NULL;
          DecodeReqData(data, len, &TestData, &TestPattern);
          netbuf_delete(buf);

          ReqForTest(TestData, TestPattern);

          FreeData(&TestPattern);//FreeTestPattern(&TestPattern);

          TestResult_s TestResult = { .Test_ID = TestData.Test_ID, .Periph_B_F = TestData.Periph_B_F, .TestResult =  E_TEST_FAILED};
          if(pdPASS == xQueueReceive(TestReportQueue, &TestResult, portMAX_DELAY))  // In the future will be added limited timeout.
           {
            SendResponse(conn, &addr, port, TestResult);
           }
         }
        vTaskDelay(pdMS_TO_TICKS(1));
       }
     }
   }
  else
   {
    rtprintf("\n\r%sThe connection couldn't be established.%s\n\r", TermRed, TermColorsReset);
    netconn_delete(conn);
    //for(;;);
   }

  vTaskDelete(NULL);
 }


/*----------------------------------------------------------------------------------------------------------------------*/
/*  Waits until the network connection is established.                                                                  */
void Wait_for_DHCP(void)
 {
  extern struct netif gnetif;
  while (gnetif.ip_addr.addr == 0)
   {
    vTaskDelay(pdMS_TO_TICKS(100)); /* Wait until DHCP assigns an IP */
   }
  while (!dhcp_supplied_address(&gnetif))
   {
	vTaskDelay(pdMS_TO_TICKS(100));    /* Now safe to start UDP communication */
   }

  /* Proceed with Netconn UDP client... */
 }

/*----------------------------------------------------------------------------------------------------------------------*/
/*  Sends response via the network.                                                                                     */
void SendResponse(struct netconn *conn, ip4_addr_t *ip4addr, uint16_t ipport, TestResult_s TestResult)
 {
  struct netbuf *buf;
  char *ip_str;
  uint16_t port;
  uint8_t *Pack;
  size_t Len;

  Len = EncodeRespData(&TestResult, &Pack);
  if(Pack)
   {
    buf = netbuf_new();
    netbuf_ref(buf, Pack, Len);
    netconn_sendto(conn, buf, ip4addr, ipport);
    netbuf_delete(buf);                       /* Freeing buffer after sending */
    ip_str = ip4addr_ntoa(ip4addr);
    port = ipport;
    rtprintf("%sThe response was sent to %s:%d\n\r%s", TermYello, ip_str, port, TermColorsReset);

    FreeData(&Pack);  //  FreeRespData(&Pack);
   }


 }

/*----------------------------------------------------------------------------------------------------------------------*/
/*  Gives result to the "Network.c/h" for sending the answer back.                                                      */
void GiveResults(PeriphBitField_s DevResults, PeriphBitField_s DevsUnderTest, uint32_t Test_ID)
 {
  TestResult_s TestResult;
  uint8_t *ResultsBits;
  uint8_t *DUTBits;  /* DUT - Devices Under Test. */
  bool FinalResult;

  ResultsBits = (uint8_t*)&DevResults;
  DUTBits = (uint8_t*)&DevsUnderTest;

  FinalResult = (!((*ResultsBits)^(*DUTBits)));  /* (!(~(~((*ResultsBits)^(*DUTBits))))) - Bit-xoring devices under test with existing devices will give "1" in fails. Boolean not ("!") will give true if all bits were zeros that means all devices passed the test. otherwise will be given "false". */

  TestResult.Test_ID = Test_ID;
  TestResult.Periph_B_F = DevsUnderTest;
  TestResult.Results_B_F = DevResults;
  TestResult.TestResult = (FinalResult ? E_TEST_SUCCEEDED : E_TEST_FAILED);

  xQueueSend(TestReportQueue, &TestResult, pdMS_TO_TICKS(10));
 }


