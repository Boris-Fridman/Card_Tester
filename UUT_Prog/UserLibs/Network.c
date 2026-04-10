#include "Network.h"

#include "CommonData.h"

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



#define xNETWORK_PRIORITY 0
#define xNETWORK_STACK_SIZE MAX(configMINIMAL_STACK_SIZE, 512)



TaskHandle_t xNewtworkTaskHandle;      // Handle to network task.


void NetworkTask(void *pvParameters);
void Wait_for_DHCP(void);
void DecodeData(uint8_t Data[], size_t Len);



void NetworkInit()
 {
  BaseType_t result;
  result = xTaskCreate(NetworkTask, "Network Task", xNETWORK_STACK_SIZE, NULL, xNETWORK_PRIORITY, &xNewtworkTaskHandle);
  if(result != pdPASS)
   {
	printf("\n\rThe network task couldn't be created. \n\r");
	//for(;;);
   }

 }



void NetworkTask(void *pvParameters)
 {
  struct netconn *conn;
  struct netbuf *buf;
  void *data;
  uint16_t len;
  char *ip_str;
  extern struct netif gnetif;
  struct netif *netif = &gnetif;
  ip4_addr_t const *remote_ip;
  err_t error;
  char *msg = "Hello from STM32 UDP Client";

  Wait_for_DHCP();
  netif_set_hostname(&gnetif, HOST_NAME);

  remote_ip = netif_ip4_addr(netif);
  ip_str = ip4addr_ntoa(remote_ip);
  printf("IP Address Assigned: %s\n", ip_str);

  conn = netconn_new(NETCONN_UDP);
  if(conn != NULL)
   {
    error = netconn_bind(conn, NULL, DESTIN_PORT);

    //error = netconn_connect(conn, remote_ip, DESTIN_PORT);
    if(error == ERR_OK)
     {
      for(;;)
       {

//        buf = netbuf_new();
//        netbuf_ref(buf, msg, strlen(msg));
//        netconn_send(conn, buf);
//        netbuf_delete(buf); // Free buffer after sending

        error = netconn_recv(conn, &buf);
        if (error == ERR_OK)
         {
          // Process received data in buf->p->payload
          netbuf_data(buf, &data, &len);
          DecodeData(data, len);
          netbuf_delete(buf);
         }


//        netconn_recv(conn, &buf);
//        netbuf_data(buf, &data, &len);
        vTaskDelay(pdMS_TO_TICKS(1));
       }
     }
   }
  else
   {
    printf("\n\rThe connection couldn't be established.\n\r");
    //for(;;);
   }
  vTaskDelete(NULL);
 }




void Wait_for_DHCP(void)
 {
  extern struct netif gnetif;
  while (gnetif.ip_addr.addr == 0)
   {
    vTaskDelay(pdMS_TO_TICKS(100)); // Wait until DHCP assigns an IP
   }
  while (!dhcp_supplied_address(&gnetif))
   {
	vTaskDelay(pdMS_TO_TICKS(100));    // Now safe to start UDP communication
   }

  // Proceed with Netconn UDP client...
 }


void DecodeData(uint8_t Data[], size_t Len)
 {
  TestData_s TestData;
  uint8_t *TestPattern;
  uint32_t RecvCRC, CalcCRC;
  size_t PackSizeNetto, PackSizeFull;
  uint8_t *Pack;
  Pack = Data;
  PackSizeFull = Len;
  PackSizeNetto = PackSizeFull - sizeof(RecvCRC);
  CalcCRC = FindCRC(Pack, Len - sizeof(RecvCRC), DEF_INIT_VAL);
  memcpy(&RecvCRC, Pack + PackSizeNetto , sizeof(RecvCRC));
  if(CalcCRC == RecvCRC)
   {
    memcpy(&TestData, Pack, sizeof(TestData_s));

    TestPattern = calloc(TestData.Bit_Pattern_Length, sizeof(uint8_t));
    if(TestPattern)
     {

      free(TestPattern);  // At this moment the free command exists here, but in the future maybe it will be moved to another place after proceeding the data.
     }
   }

 }

