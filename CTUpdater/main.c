#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <libgen.h>
#include <unistd.h>
#include "CommonData.h"
#include "Network.h"
#include "string.h"
#include "ProgImage.h"
#include <netdb.h>

/*======================================================================================================================*/

#define DEV_REBOOT_TIME   10  /* The reboot time of the device for waiting. */

#define ARG_ERROR_RESULT -1
#define ARG_BURN_RESULT   0
#define ARG_HELP_RESULT   1
#define ARG_RESET_RESULT  2

/*======================================================================================================================*/

int CheckArgs(int argc, char *argv[], char ImageFullPathName[]);
void PrintHelpMessage(char *ProgName);
void PrintErrorMessage(int argc, char *argv[]);
int ReqDevForBurning(char ImageFullPathName[]);
int ReqDevForMakingReset();

/*======================================================================================================================*/

int main(int argc, char *argv[])
 {
  int ArgResult;
  char ImageFullPathName[PATH_FILE_NAME_LEN];
  bool NoPiping;
  NoPiping = isatty(STDOUT_FILENO); /* Checking if the output is not redirected to any other program or file to decide if to use colors or not. */

  //ansi clear screen
  if(NoPiping)
   printf("\033[2J\033[H");
  else
   printf("\n\r");
  
  //code

  ArgResult = CheckArgs(argc, argv, ImageFullPathName);
   

  switch(ArgResult)
   {
    case ARG_ERROR_RESULT: 
      PrintErrorMessage(argc, argv);
     break;
    case ARG_HELP_RESULT : 
      PrintHelpMessage(argv[0]);
     break;
    case ARG_BURN_RESULT : 
      ArgResult = ReqDevForBurning(ImageFullPathName);
     break;
    case ARG_RESET_RESULT: 
      ReqDevForMakingReset();
     break;
    default:
     break;
   }


  return ( (ArgResult == ARG_ERROR_RESULT) ? -1 : 0 );
 }


/*======================================================================================================================*/

int CheckArgs(int argc, char *argv[], char ImageFullPathName[])
 {
  int i;
  if(argc<=1)
   return ARG_ERROR_RESULT;
  for(i = 1; i < argc; i++)
   {
    if(!strcmp(argv[i], "h"))
     return ARG_HELP_RESULT;
    if(!strcmp(argv[i], "r"))
     return ARG_RESET_RESULT;
   }

  strcpy(ImageFullPathName, argv[1]);

  return ARG_BURN_RESULT;
 }



 void PrintHelpMessage(char *ProgName)
  {
   char *fname = basename(ProgName);
   printf("\n\rTo use the program it's needed to type the next parameters:\n\r");
   printf("%s [<filename>.bin] [r] \n\r", fname);
   printf("Or to type h to see the help message.\n\r\n\r");
  }


void PrintErrorMessage(int argc, char *argv[])
 {
  UNUSED(argc);
  fprintf(stderr, "Error in arguments.\n\r");
  PrintHelpMessage(argv[0]);
 }


int ReqDevForBurning(char ImageFullPathName[])
 {
  int Result = 0;
  bool StdErrNoPiping, StdOutNoPiping;
  StdErrNoPiping = isatty(STDERR_FILENO); /* Checking if the output is not redirected to any other program or file to decide if to use colors or not. */
  StdOutNoPiping = isatty(STDOUT_FILENO); /* Checking if the output is not redirected to any other program or file to decide if to use colors or not. */

  Result = InitNetwork(BL_HOST_NAME);
  if(Result == -1) /* Bootloader not detected. */
   {
    Result = ReqDevForMakingReset();
    if(Result == 0) /* Device Itself was found.*/
     {
      int i;
      if(StdOutNoPiping)
       {
        char *ScaleSymbs[] = {"░", "▓"}; // ░▒▓
        char *ScaleColors[] = {TermCyan, TermRed};

        for(i = 0; i < DEV_REBOOT_TIME; i++)
         {
          MoveCursToCol(1);
          PrintHorizScale(DEV_REBOOT_TIME, i, ScaleColors, ScaleSymbs, DEV_REBOOT_TIME, DEV_REBOOT_TIME - i, "");
          sleep(1);
         }
        MoveCursToCol(1);         
        ClearLine(E_FULL_LINE);
       }
      else
       {
        sleep(DEV_REBOOT_TIME);
       }
      printf("Retrying... \n\r");
      Result = InitNetwork(BL_HOST_NAME);
     }
    if(Result == 0)
     {
      if(StdOutNoPiping)fprintf(stdout, TermGreen);
      fprintf(stdout, "The device restarted successfully.\n\r");
      if(StdOutNoPiping)fprintf(stdout, TermColorsReset);
     }
    else
     {
      if(StdErrNoPiping)fprintf(stderr, TermRed);
      fprintf(stderr, "Cannot restart the device to the bootloader mode. try to restart it manually.\n\r");
      if(StdErrNoPiping)fprintf(stderr, TermColorsReset);
     }
   }
  if(Result == 0)
   {
    InitImage(ImageFullPathName);
    SendImageToNetwork();
    CloseImage();
    CloseNetwork();
   }
  else
   {

   }
  return Result;
 } 

 int ReqDevForMakingReset()
 {
  int Result;
/*  ---------------------------------------------------  */
  Result = InitNetwork(HOST_NAME);
  if(Result == 0)
   {
    SendCommandToNetwork(&ResetCondition, NULL);
    //WaitForResponse(&ResultData, TimeOut);
    CloseNetwork();
   }
/*  ---------------------------------------------------  */
  return Result;
 }
