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

#include "api.h"
#include "netif.h"
#include "dhcp.h"
#include "netbuf.h"

bool NetworkLoaded = false;

void DoNetwork()
 {
  extern struct netif gnetif;
  static ip4_addr_t const *remote_ip;
  static struct netif *netif = &gnetif;
  static char *ip_str;
  static struct netconn *conn;
  if(!NetworkLoaded)  /* Network isn't loaded yet. */
   {
    if(gnetif.ip_addr.addr != 0)
     {
      remote_ip = netif_ip4_addr(netif);
      ip_str = ip4addr_ntoa(remote_ip);
      printf("IP Address Assigned: %s\n", ip_str);
//      conn = netconn_new(NETCONN_UDP);
//      if(conn != NULL)
//       {
//        error = netconn_bind(conn, NULL, DESTIN_PORT);
//        if(error == ERR_OK)
//         {
//          NetworkLoaded = true;
//         }
//       }



     }
   }
  else  /* Network is Loaded */
   {

   }

 }


