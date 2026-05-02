#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <libgen.h>
#include <unistd.h>
#include "CommonData.h"
#include "Network.h"
#include "string.h"
    
/*======================================================================================================================*/

#define ARG_ERROR_RESULT -1
#define ARG_BURN_RESULT   0
#define ARG_HELP_RESULT   1
#define ARG_RESET_RESULT  2

/*======================================================================================================================*/

int CheckArgs(int argc, char *argv[], char ImageImageFullPath[]);
void PrintHelpMessage(char *ProgName);
void PrintErrorMessage(int argc, char *argv[]);
int ReqDevForBurning(char ImageImageFullPath[]);
void ReqDevForMakingReset();

/*======================================================================================================================*/

int main(int argc, char *argv[])
 {
  int ArgResult;
  char ImageImageFullPath[500];
  bool NoPiping;
  NoPiping = isatty(STDOUT_FILENO); /* Checking if the output is not redirected to any other program or file to decide if to use colors or not. */

  //ansi clear screen
  if(NoPiping)
   printf("\033[2J\033[H");
  else
   printf("\n\r");
  
  //code

  ArgResult = CheckArgs(argc, argv, ImageImageFullPath);
   

  switch(ArgResult)
   {
    case ARG_ERROR_RESULT: 
      PrintErrorMessage(argc, argv);
     break;
    case ARG_HELP_RESULT : 
      PrintHelpMessage(argv[0]);
     break;
    case ARG_BURN_RESULT : 
      ArgResult = ReqDevForBurning(ImageImageFullPath);
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

int CheckArgs(int argc, char *argv[], char ImageImageFullPath[])
 {
  int i;
  if(argc<=1)
   return ARG_ERROR_RESULT;
  for(i = 0; i < argc; i++)
   {
    if(strcmp(argv[i], "h"))
     return ARG_HELP_RESULT;
    if(strcmp(argv[i], "r"))
     return ARG_RESET_RESULT;
   }

  strcpy(ImageImageFullPath, argv[1]);

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


int ReqDevForBurning(char ImageImageFullPath[])
 {
  
  return 0;
 } 

 void ReqDevForMakingReset()
 {
/*  ---------------------------------------------------  */
  InitNetwork();
  SendCommandToNetwork(&ResetCondition, NULL);
  //WaitForResponse(&ResultData, TimeOut);
  CloseNetwork();
/*  ---------------------------------------------------  */
 }
