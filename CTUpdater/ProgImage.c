#include "ProgImage.h"

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <limits.h>


#include "CommonData.h"
#include "Network.h"


/*======================================================================================================================*/

static char ImgFileName[PATH_FILE_NAME_LEN];                   /* Name of file. */
static int fd;                                                 /* File handle pointer. */
static bool FileIsOpen = false;                                /* Flag signalizes if the log file is open or not. */


/*======================================================================================================================*/

/*
 * *************************************************************************************************************
 **          Image Programming Functions / Procedures
 * *************************************************************************************************************
 */

/*----------------------------------------------------------------------------------------------------------------------*/
/*  Initilizes image. Checks existance of the given file with image (".bin") file.                                      */
int InitImage(char ImgFilePathName[])
 {
  bool StdErrNoPiping, StdOutNoPiping;
  StdErrNoPiping = isatty(STDERR_FILENO); /* Checking if the output is not redirected to any other program or file to decide if to use colors or not. */
  StdOutNoPiping = isatty(STDOUT_FILENO); /* Checking if the output is not redirected to any other program or file to decide if to use colors or not. */

  strncpy(ImgFileName,ImgFilePathName, PATH_FILE_NAME_LEN);
  ImgFileName[PATH_FILE_NAME_LEN] = '\0'; /* Ensures that the last character of the destination will be 0 in case the source is longer than "PATH_FILE_NAME_LEN". */
  fd = open(ImgFileName, O_RDONLY);
  if(fd < 0)
   {
    if(StdErrNoPiping)fprintf(stderr, TermRed);
    fprintf(stderr, "Error in opening file!\n\r");
    if(StdErrNoPiping)fprintf(stderr, TermColorsReset);
    return -1;
   }
  else 
   {
    FileIsOpen = true;
    if(StdOutNoPiping)fprintf(stdout, TermGreen);
    fprintf(stdout, "The file was opened successfully.\n\r");
    if(StdOutNoPiping)fprintf(stdout, TermColorsReset);
    return 0;
   }

 }

/*----------------------------------------------------------------------------------------------------------------------*/
/* Sends the image with update existed in ".bin" file to the network segment by segment. If the respond wasn't given the function will wait infinitly. */
int SendImageToNetwork()
 {
  uint8_t CodeSegment[MAX_TEST_PATTERN_SIZE];
  ssize_t NumReadBytes;
  TestData_s NetPacket = {0};
  TestResult_s Result;
  uint32_t StartSegAddr = 0;
  if(FileIsOpen)
   {
    printf("Sending Image to the network");
    NetPacket.Periph_B_F.OTA_UPDATE_bf = 1;

    NetPacket.Num_Interations = OTA_START;
    NetPacket.Bit_Pattern_Length = 0;
    NetPacket.Test_ID = 0;  

    printf("Sending Start Command...\n\r");
    SendCommandToNetwork(&NetPacket, CodeSegment);
    WaitForResponse(&Result, 0);
    
    printf("Sending Data...\n\r");
    NetPacket.Num_Interations = OTA_DATA;
    while( (NumReadBytes = read(fd, CodeSegment, sizeof(CodeSegment)) ) > 0)
     {
      NetPacket.Bit_Pattern_Length = NumReadBytes;
      NetPacket.Test_ID = START_PROG_ADDRESS + StartSegAddr;  
      SendCommandToNetwork(&NetPacket, CodeSegment);
      WaitForResponse(&Result, 0);
      StartSegAddr += NumReadBytes;
     }
    printf("Sending Finish Command...\n\r");
    NetPacket.Num_Interations = OTA_END;
    NetPacket.Bit_Pattern_Length = 0;
    NetPacket.Test_ID = 0;  
    SendCommandToNetwork(&NetPacket, CodeSegment);
    WaitForResponse(&Result, 0);
    printf("The data was sent...\n\r");
   }
  return 0;
 }
/*----------------------------------------------------------------------------------------------------------------------*/
/*  Closes the image file.                                                                                              */
void CloseImage()
 {
  if(FileIsOpen)
   {
    close(fd);
    FileIsOpen = false;
   }

 }


/*======================================================================================================================*/
