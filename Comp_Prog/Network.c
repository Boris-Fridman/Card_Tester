#include "Network.h"

#include <string.h>
#include <sys/time.h>
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
    fprintf(stderr, "%sCannot detect host address from the name.%s\n\r", TermRed, TermColorsReset);
    return -1;
   }

  // Create a UDP socket
  if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
   {
    //fprintf(stderr, "%sSocket creation failed.%s\n\r", TermRed, TermColorsReset);
    perror("Socket creation failed");
    return -1;
   }

  memset(&dest_addr, 0, sizeof(dest_addr));
  dest_addr.sin_family = AF_INET;
  dest_addr.sin_port = htons(DESTIN_PORT);
  memcpy(&dest_addr.sin_addr, host->h_addr_list[0], host->h_length);

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
  ssize_t result;

  PackSizeNetto = sizeof(TestData_s) + TestData->Bit_Pattern_Length;
  PackSizeFull = PackSizeNetto + CRC_SIZE;
  Pack = calloc(PackSizeFull, sizeof(uint8_t));
  if(Pack)
   {
    memcpy(Pack, TestData, sizeof(TestData_s));
    memcpy(Pack + sizeof(TestData_s), TestPattern, TestData->Bit_Pattern_Length);

    Add_CRC(Pack, PackSizeFull);

    result = sendto(sockfd, Pack, PackSizeFull, 0, (const struct sockaddr *)&dest_addr, sizeof(dest_addr));
    if(result >= 0 )
     printf("%sThe message was sent successfully.  %ld bytes were sent.%s\n\r", TermGreen, result, TermColorsReset);
    else
     printf("%sError in sending.%s\n\r", TermRed, TermColorsReset);
    free(Pack);
   }
  else
   {
    fprintf(stderr, "%sCannot allocate the memory.%s\n\r", TermRed, TermColorsReset);
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


  struct timeval tv;
  uint8_t buffer[BUFFER_SIZE];
  static struct sockaddr_in client_addr;
  socklen_t addr_len = sizeof(client_addr);
  
  tv.tv_sec  = TimeOut / 1000;
  tv.tv_usec = (TimeOut % 1000) * 1000;

  if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) 
   {
    perror("Error setting timeout");
   }


  ssize_t len = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&client_addr, &addr_len);
  if(len >= (ssize_t)MIN_RECV_MSG_SIZE)
   {
    if(CRC_Correct(buffer, len))
     {
      printf("%sThe message was received successfully.  %ld bytes were received.%s\n\r", TermGreen, len, TermColorsReset);
      memcpy(ResultData, buffer, sizeof(TestResult_s));
      return 0;
     }
   }

  return -1;
 }


