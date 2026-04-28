#include "Network.h"

#include <string.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>



// static int TestID;                     // Defined temperary for test. Will be removed.
// static PeriphBitField_s Periph_B_F;    // Defined temperary for test. Will be removed.



static struct sockaddr_in dest_addr;
static struct hostent *host;
static int sockfd;

int InitNetwork()
 {
  bool NoPiping;
  NoPiping = isatty(STDERR_FILENO);

  host = gethostbyname(HOST_NAME);
  if(host == NULL)
   {
    if(NoPiping)fprintf(stderr, TermRed);
    //fprintf(stderr, "Cannot detect host address from the name.\n\r");
    perror("Cannot detect host address from the name.");
    if(NoPiping)fprintf(stderr, TermColorsReset);
    return -1;
   }

  // Create a UDP socket
  if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
   {
    if(NoPiping)fprintf(stderr, "%s", TermRed);
    perror("Socket creation failed");
    if(NoPiping)fprintf(stderr, "%s", TermColorsReset);
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
  // TestID = TestData->Test_ID;          // Defined for test only. Will be removed.
  // Periph_B_F = TestData->Periph_B_F;   // Defined for test only. Will be removed.

  uint8_t *Pack;
  size_t PackSizeNetto, PackSizeFull;
  ssize_t result;
  bool StdErrNoPiping, StdOutNoPiping;
  StdErrNoPiping = isatty(STDERR_FILENO);
  StdOutNoPiping = isatty(STDOUT_FILENO);

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
     {
      if(StdOutNoPiping)fprintf(stdout, "%s", TermGreen);
      printf("The message was sent successfully.  %ld bytes were sent.\n\r", result);
      if(StdOutNoPiping)fprintf(stdout, "%s", TermColorsReset);
     }
    else
     {
      if(StdOutNoPiping)fprintf(stdout, "%s", TermRed);
      printf("Error in sending.\n\r");
      if(StdOutNoPiping)fprintf(stdout, "%s", TermColorsReset);
     }
    free(Pack);
   }
  else
   {
    if(StdErrNoPiping)fprintf(stderr, "%s", TermRed);
    fprintf(stderr, "Cannot allocate the memory.\n\r");
    if(StdErrNoPiping)fprintf(stderr, "%s", TermColorsReset);
   }

  return 0;
 }

int WaitForResponse(TestResult_s *ResultData, uint32_t TimeOut)
 {
  // int r;                                                               // Defined for test only. Will be removed.
  // ResultData->Periph_B_F = Periph_B_F;                                 // Defined for test only. Will be removed.
  // ResultData->Test_ID = TestID;                                        // Defined for test only. Will be removed.
  // r = rand();                                                          // Defined for test only. Will be removed.
  // ResultData->TestResult = (r % 2) ? E_TEST_SUCCEEDED : E_TEST_FAILED; // Defined for test only. Will be removed.


  struct timeval tv;
  uint8_t buffer[BUFFER_SIZE];
  static struct sockaddr_in client_addr;
  socklen_t addr_len;

  bool StdErrNoPiping, StdOutNoPiping;
  StdErrNoPiping = isatty(STDERR_FILENO);
  StdOutNoPiping = isatty(STDOUT_FILENO);
  
  addr_len = sizeof(client_addr);

  tv.tv_sec  = TimeOut / 1000;
  tv.tv_usec = (TimeOut % 1000) * 1000;

  if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) 
   {
    if(StdErrNoPiping)fprintf(stderr, "%s", TermRed);
    perror("Error setting timeout");
    if(StdErrNoPiping)fprintf(stderr, "%s", TermColorsReset);
   }


  ssize_t len = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&client_addr, &addr_len);
  if(len >= (ssize_t)MIN_RECV_MSG_SIZE)
   {
    if(CRC_Correct(buffer, len))
     {
      if(StdOutNoPiping)fprintf(stdout, "%s", TermGreen);
      printf("The message was received successfully.  %ld bytes were received.\n\r", len);
      if(StdOutNoPiping)fprintf(stdout, "%s", TermColorsReset);
      memcpy(ResultData, buffer, sizeof(TestResult_s));
      return 0;
     }
   }
  else
   {
    if(StdErrNoPiping)fprintf(stderr, "%s", TermRed);
    //printf("%sNo Response.%s\n\r", TermRed, TermColorsReset);
    perror("Problem in receiving data");
    if(StdErrNoPiping)fprintf(stderr, "%s", TermColorsReset);
    return -1;
   }

  return -1;
 }


