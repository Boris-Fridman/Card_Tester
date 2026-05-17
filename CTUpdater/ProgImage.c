#include "ProgImage.h"

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <limits.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>



#include "CommonData.h"
#include "Network.h"


/*======================================================================================================================*/

static char ImgFileName[PATH_FILE_NAME_LEN];                   /* Name of file. */
static int fd;                                                 /* File handle pointer. */
static bool FileIsOpen = false;                                /* Flag signalizes if the log file is open or not. */
static off_t FileSize;
static off_t NumSentBytes;

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
  struct stat st;
  StdErrNoPiping = isatty(STDERR_FILENO); /* Checking if the output is not redirected to any other program or file to decide if to use colors or not. */
  StdOutNoPiping = isatty(STDOUT_FILENO); /* Checking if the output is not redirected to any other program or file to decide if to use colors or not. */

  strncpy(ImgFileName,ImgFilePathName, PATH_FILE_NAME_LEN);
  ImgFileName[PATH_FILE_NAME_LEN] = '\0'; /* Ensures that the last character of the destination will be 0 in case the source is longer than "PATH_FILE_NAME_LEN". */
  if(stat(ImgFileName, &st) == 0)
   {
    FileSize = st.st_size;
   }
  else
   {
    FileSize = 0;
   }
  NumSentBytes = 0;
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
  struct winsize SizeOfWindow;
  char *ScaleSymbs[] = {"░", "▓"}; // ░▒▓
  char *ScaleColors[] = {TermBlue, TermGreen};
  uint8_t CodeSegment[MAX_TEST_PATTERN_SIZE];
  ssize_t NumReadBytes;
  TestData_s NetPacket = {0};
  TestResult_s Result;
  uint32_t StartOffsAddr = 0;
  uint32_t ScaleLength = 100;  // Will be in the future adjusted to screen size.
  uint32_t DoneScaleLength = 0;
  uint32_t Percents = 0;
  bool NoPiping;
  NoPiping = isatty(STDOUT_FILENO); /* Checking if the output is not redirected to any other program or file to decide if to use colors or not. */
  if(FileIsOpen)
   {
    printf("Sending Image to the network...\n\r");
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
      NetPacket.Test_ID = START_PROG_ADDRESS + StartOffsAddr;  
      SendCommandToNetwork(&NetPacket, CodeSegment);
      WaitForResponse(&Result, 0);
      StartOffsAddr += NumReadBytes;
      NumSentBytes += NumReadBytes;
      if(FileSize) /* File size was detected*/
       {
        if(NoPiping)
         {
          ioctl(STDOUT_FILENO, TIOCGWINSZ, &SizeOfWindow);  // Can be explained in the site "https://www.qnx.com/developers/docs/8.0/com.qnx.doc.neutrino.lib_ref/topic/i/ioctl.html".
          ScaleLength = DIV_RND(SizeOfWindow.ws_col * 4 , 5);
          DoneScaleLength = DIV_RND(ScaleLength * NumSentBytes, FileSize);
          Percents = DIV_RND(100 * NumSentBytes, FileSize);
          MoveCursToCol(1);
          PrintHorizScale(ScaleLength, DoneScaleLength, ScaleColors, ScaleSymbs, 100, Percents, "%");
         }
        else
         {
          printf("Were sent %ld bytes from %ld\n\r", NumSentBytes, FileSize);
         }
  
       }
      else /* File size was not detected*/
       printf("%ld bytes were sent \n\r", NumSentBytes);
     }
    MoveCursToCol(1);
    ClearLine(E_FULL_LINE);
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
