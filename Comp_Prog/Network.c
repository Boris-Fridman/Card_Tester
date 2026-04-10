#include "Network.h"

#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>



static int TestID;                     // Defined temperary for test. Will be removed.
static PeriphBitField_s Periph_B_F;    // Defined temperary for test. Will be removed.



static struct sockaddr_in dest_addr;
static struct hostent *host;
static int sockfd;

int InitNetwork()
 {

  host = gethostbyname(HOST_NAME);
  if(host == NULL)
   {
    fprintf(stderr, "Cannot detect host address from the name. \n\r");
    return -1;
   }

  // Create a UDP socket
  if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
   {
    perror("Socket creation failed");
    return -1;
   }

  memset(&dest_addr, 0, sizeof(dest_addr));
  dest_addr.sin_family = AF_INET;
  dest_addr.sin_port = htons(DESTIN_PORT);

  memcpy(&dest_addr.sin_addr, host->h_addr_list[0], host->h_length);

  //dest_addr.sin_addr.s_addr = inet_addr(DESTIN_IP);  // Is used in case the address is known.

  return 0;
 }

void CloseNetwork()
 {
  close(sockfd);
 }

int SendCommandToNetwork(TestData_s *TestData, uint8_t TestPattern[])
 {
  TestID = TestData->Test_ID;          // Defined for test only. Will be removed.
  Periph_B_F = TestData->Periph_B_F;   // Defined for test only. Will be removed.


  uint8_t *Pack;
  size_t PackSizeNetto, PackSizeFull;
  uint32_t CRC;
  ssize_t result;

  PackSizeNetto = sizeof(TestData_s) + TestData->Bit_Pattern_Length;
  PackSizeFull = PackSizeNetto + sizeof(CRC);
  Pack = calloc(PackSizeFull, sizeof(uint8_t));
  if(Pack)
   {
    memcpy(Pack, TestData, sizeof(TestData_s));
    memcpy(Pack + sizeof(TestData_s), TestPattern, TestData->Bit_Pattern_Length);
    CRC = FindCRC(Pack, PackSizeNetto, DEF_INIT_VAL);
    memcpy(Pack + PackSizeNetto, &CRC, sizeof(CRC));
    result = sendto(sockfd, Pack, PackSizeFull, 0, (const struct sockaddr *)&dest_addr, sizeof(dest_addr));
    if(result >= 0 )
     printf("The message was sent successfully.  %ld bytes were sent.\n", result);
    else
     printf("Error in sending.\n");
    free(Pack);
   }
  else
   {
    fprintf(stderr, "Cannot allocate the memory.\n\r");
   }

  return 0;
 }

int WaitForResponse(TestResult_s *ResultData, uint32_t TimeOut)
 {
  int r;                                                               // Defined for test only. Will be removed.
  ResultData->Periph_B_F = Periph_B_F;                                 // Defined for test only. Will be removed.
  ResultData->Test_ID = TestID;                                        // Defined for test only. Will be removed.
  r = rand();                                                          // Defined for test only. Will be removed.
  ResultData->TestResult = (r % 2) ? E_TEST_SUCCEEDED : E_TEST_FAILED; // Defined for test only. Will be removed.
  
  return 0;
 }


