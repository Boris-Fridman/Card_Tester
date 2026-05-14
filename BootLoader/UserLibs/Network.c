/*
 * Network.c
 *
 *  Created on: 3 May 2026
 *      Author: boris
 */


#include "Network.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lwip/udp.h"

#include "api.h"
#include "netif.h"
#include "dhcp.h"
#include "netbuf.h"

#include "Flash.h"

/*======================================================================================================================*/

static volatile struct udp_pcb *upcb;

/*======================================================================================================================*/

/*
 * *************************************************************************************************************
 **          Network Receive Callback.
 * *************************************************************************************************************
 */
/*----------------------------------------------------------------------------------------------------------------------*/
/*   Network Receive Callback.                                                                                          */
void UDPReceiveCB(void *arg, struct udp_pcb *pcb, struct pbuf *p, const ip_addr_t *addr, u16_t port)
 {
  if (p != NULL)
   {
    /* Process your data here (p->payload) */
    TestData_s TestData;
    uint8_t *TestPattern = NULL;
    TestResult_s TestResult = {0};
    ssize_t Len;
    uint8_t *Pack;

    DecodeReqData(p->payload, p->len, &TestData, &TestPattern);
    BurnData(TestData, TestPattern, &TestResult);
    FreeTestPattern(&TestPattern);

    Len = EncodeRespData(&TestResult, &Pack);


    /* Sending a reply. */
    struct pbuf *p_reply = pbuf_alloc(PBUF_TRANSPORT, Len, PBUF_RAM);
    if (p_reply != NULL)
     {
      memcpy(p_reply->payload, Pack, Len);
      udp_sendto(pcb, p_reply, addr, port); // Send to sender's IP/Port
      pbuf_free(p_reply);
      FreeRespData(&Pack);
     }

    /* Freeing received pbuf p. */
    pbuf_free(p);
   }
 }

/*======================================================================================================================*/
/*
 * *************************************************************************************************************
 **          Init Deinit Network Functions
 * *************************************************************************************************************
 */
/*----------------------------------------------------------------------------------------------------------------------*/
/*   Initializes the network UDP Server.                                                                                */
void InitNetwork(void)
 {
  upcb = udp_new();
  if (upcb != NULL)
   {
    err_t err = udp_bind(upcb, IP_ADDR_ANY, DESTIN_PORT);
    if (err == ERR_OK)
     {
      udp_recv(upcb, UDPReceiveCB, NULL);
     }
   }
 }


/*----------------------------------------------------------------------------------------------------------------------*/
/*  Deinitializes the network.                                                                                          */
void DeinitNetwork(void)
 {
  if(upcb != NULL)
   {
    udp_recv(upcb, NULL, NULL);
    udp_remove(upcb);
    upcb = NULL;
   }
 }

/*======================================================================================================================*/





