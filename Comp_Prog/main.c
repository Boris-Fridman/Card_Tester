#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <libgen.h>
#include <time.h>
#include <unistd.h>
#include "CommonData.h"
#include "DataBase.h"
#include "Network.h"
    
/*======================================================================================================================*/

#define DevsColor   TermBlue //TermYello
#define ValuesColor TermMagenta 
#define UnitsColor  TermCyan

#define ARG_ERROR_RESULT -1
#define ARG_TEST_RESULT   0
#define ARG_HELP_RESULT   1
#define ARG_RESET_RESULT  2

/*======================================================================================================================*/

void PrinthelpMessage(char *ProgName);
void PrintErrorMessage(int argc, char *argv[]);
int CheckArgs(int argc, char *argv[], int *NInt, PeriphBitField_s *BFResult);
void ReqDevForMakingReset(void);
void ReqDevForMakingTest(int NInt, PeriphBitField_s PeriphBF);
void PrepereData(TestData_s *TestData, uint8_t TestPattern[], uint32_t *TimeOut, PeriphBitField_s PeriphBF, uint32_t NInt, uint32_t TestID);
void PrintPreperedData(TestData_s const * const TestData, uint8_t const TestPattern[]);
void PrintTestResults(TestResult_s const * const TestInfo);
void ConvertTime(time_t const * const TimeToConvert, char TimeAsStr[], size_t TimeStrSize);

/*======================================================================================================================*/

int main(int argc, char *argv[])
 {
  int NInt;
  int ArgResult;
  //int i;
  PeriphBitField_s PeriphBF;

  //ansi clear screen
  printf("\033[2J\033[H");
  
  //code
  srand(time(NULL));
  // printf("printing arguments...\n\r");
  // for(i = 0; i < argc; i++)
  //  {
  //   printf("%3d: %s\r\n", i, argv[i]);
  //  }
  
  ArgResult = CheckArgs(argc, argv, &NInt, &PeriphBF);
  
  switch(ArgResult)
   {
    case ARG_ERROR_RESULT: 
      PrintErrorMessage(argc, argv);
     break;
    case ARG_HELP_RESULT : 
      PrinthelpMessage(argv[0]);
     break;
    case ARG_TEST_RESULT : 
      ReqDevForMakingTest(NInt, PeriphBF);
     break;
    case ARG_RESET_RESULT: 
      ReqDevForMakingReset();
     break;
    default:
     break;
   }
  return 0;
 }

/*======================================================================================================================*/

int CheckArgs(int argc, char *argv[], int *NInt,PeriphBitField_s *BFResult)
 {
  int i;
  size_t j;
  char *st, ch;
  int nintres;
  bool nintloaded = false;
  memset(BFResult, 0, sizeof(PeriphBitField_s));
  
  for(i = 1; i < argc; i++)
   {
    st = argv[i];
    if(st[0] == 'n')
     {
      nintres = sscanf(&st[1], "%d", NInt);
      nintloaded |= (nintres != EOF);
     }
    else
     {
      for(j = 0; j < strlen(st); j++)
       {
        ch = st[j];
        switch(ch)
         {
           case 't':
             BFResult->Timer_bf = 1;
            break;
           case 'u':
             BFResult->UART_bf = 1;
            break;
           case 's':
             BFResult->SPI_bf = 1;
            break;
           case 'i':
             BFResult->I2C_bf = 1;
            break;
           case 'a':
             BFResult->ADC_bf = 1;
            break;
           case 'h':     
             return ARG_HELP_RESULT;
            break;
           case 'r':
             return ARG_RESET_RESULT;
            break;
         }
       }
     }
   }
  if(*(uint8_t *)BFResult == 0) /* No prepheral loaded */
   {
    return ARG_ERROR_RESULT;
   }
  if(!nintloaded)
   {
    if(nintres ==  EOF)
     fprintf(stderr, "The parameter nxx is incorrect.");
    else
     fprintf(stderr, "Was not loaded.");
    fprintf(stderr, "Using default number inteructions that is \"1\"\n\r");
    *NInt = 1;
   }            

  return ARG_TEST_RESULT;
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

void ReqDevForMakingTest(int NInt, PeriphBitField_s PeriphBF)
 {
  int TestID;
  int NetResult;
  static sqlite3 *conn;
  time_t CurrentTime; 
  char timebuf[100];

  TestResult_s ResultData;
  uint32_t TimeOut;

  TestData_s TestData;
  uint8_t TestPattern[MAX_TEST_PATTERN_SIZE] = {0};

  CreateLoadDatabase(&conn); // Yes, the given pointer to database must be given as pointer to pointer to database because it's address is updated in this function.
  TestID = GetLastTestIDFromDataBase(&conn);
  TestID++;
  time(&CurrentTime);
  ConvertTime(&CurrentTime, timebuf,sizeof(timebuf));
  PrepereData(&TestData, TestPattern, &TimeOut, PeriphBF, NInt, TestID);
  PrintPreperedData(&TestData, TestPattern);
  
/*  ---------------------------------------------------  */
  InitNetwork();
  SendCommandToNetwork(&TestData, TestPattern);
  NetResult = WaitForResponse(&ResultData, TimeOut);
  CloseNetwork();
/*  ---------------------------------------------------  */

  if(NetResult == 0)
   {
    PrintTestResults(&ResultData);
   }
  WriteToDataBase(&conn, ResultData.Test_ID, timebuf, ResultData.TestResult);
 }

void PrinthelpMessage(char *ProgName)
 {
  char *fname = basename(ProgName);
  printf("To use the program it's needed to type the next parameters:\n\r");
  printf("%s [t][u][s][i][a][r] [n] \n\r", fname);
  printf("t  - Timer\n\r");
  printf("u  - UART/USART\n\r");
  printf("s  - SPI\n\r");
  printf("i  - I2C\n\r");
  printf("a  - ADC\n\r");
  printf("r  - To reset the board.");
  printf("nx - Number of interactions. Where x is number. Default is 1.");
  printf("Or to type h to see the help message.\n\r");
 }

void PrintErrorMessage(int argc, char *argv[])
 {
  fprintf(stderr, "No peripheral was selected.\n\r");
  PrinthelpMessage(argv[0]);
 }

void PrepereData(TestData_s *TestData, uint8_t TestPattern[], uint32_t *TimeOut, PeriphBitField_s PeriphBF, uint32_t NInt, uint32_t TestID)
 {
  uint32_t MinFreq, MaxAddDelay = 0;

  MaxAddDelay = 0;
  if((PeriphBF.I2C_bf)||(PeriphBF.SPI_bf)||(PeriphBF.UART_bf))
   {
    MinFreq = MAX_POSSIBLE_FREAUENCY;
    if(PeriphBF.I2C_bf)   MinFreq = MIN(MinFreq, MIN_I2C_FREQUENCY);
    if(PeriphBF.SPI_bf)   MinFreq = MIN(MinFreq, MIN_SPI_FREQUENCY);
    if(PeriphBF.UART_bf)  MinFreq = MIN(MinFreq, MIN_UART_FREQUENCY);
    MaxAddDelay += DIV_RND_UP(1000 * TestData->Bit_Pattern_Length * 8 , MinFreq);
    TestData->Bit_Pattern_Length = MAX(1, rand() % MAX_TEST_PATTERN_SIZE);  /* Determining Test Pattern Length randomly starting from 1, but less than Maximum permitted length. */
    for(uint8_t i = 0; i < TestData->Bit_Pattern_Length; i++)
     {
      TestPattern[i] = rand() % 256;
     }
   }
  else
   {
    TestData->Bit_Pattern_Length = 0;
   }
   
  if(PeriphBF.Timer_bf)
   {
    TestData->TestTime = MAX(10, rand() % (60001));    /* In µs */
    MaxAddDelay = MAX(MaxAddDelay, DIV_RND_UP(TestData->TestTime * 2 * NInt, 1000));
   }
  else
   {
    TestData->TestTime = 0;
   }
  if(PeriphBF.ADC_bf)
   {
    TestData->TestVoltage = rand() % (3200); /* In mV */
    MaxAddDelay = MAX(MaxAddDelay, DIV_RND_UP(MAX_ADC_CHECK_TIME * NInt, 1000));
   }
  else
   {
    TestData->TestVoltage = 0;
   }
  TestData->Num_Interations = NInt;
  TestData->Periph_B_F = PeriphBF;
  TestData->Test_ID = TestID;
  *TimeOut = MIN_TIME_OUT + MaxAddDelay;

 }


void PrintPreperedData(TestData_s const * const TestData, uint8_t const TestPattern[])
 {
  uint8_t i;
  uint8_t Devs;
  bool NoPiping;
  NoPiping = isatty(STDOUT_FILENO); /* Checking if the output is not redirected to any other program or file to decide if to use colors or not. */

  printf("\n\r");
  printf("Running the test...\n\r");
  printf("Test ID: %d\n\r", TestData->Test_ID);
  Devs = *(uint8_t *)&TestData->Periph_B_F;
  printf("The devices will be tested:\n\r");
  if(NoPiping)printf(DevsColor);
  for(i = 0; i < E_NUM_PERIPHS; i++)
   {
    if(Devs & (1<<i))
     {
      printf("%s\n\r", PeriphNames[i]);
     }
   }
  if(NoPiping)printf(TermColorsReset);
  if(Devs & (UART_Flag | SPI_Flag | I2C_Flag ))
   {
    printf("Peripheral chekcing pattern: ");
    if(NoPiping)printf(ValuesColor);
    printf("0x");
    for(i = 0; i < TestData->Bit_Pattern_Length; i++)
     {
      printf("%02X",TestPattern[i]);
     }
    if(NoPiping)printf(TermColorsReset);
    printf("\n\r");
   }
  
  if(Devs & Timer_Flag)
   {
    printf("Timer checking time: ");
    if(NoPiping)printf(ValuesColor);
    printf("%d",TestData->TestTime);
    if(NoPiping)printf(UnitsColor);
    printf("µs");
    if(NoPiping)printf(TermColorsReset);
    printf("\n\r");
    //printf("Timer checking time: %s%d%sµs%s\n\r", ValuesColor, TestData->TestTime, UnitsColor, TermColorsReset);
   }
  
  if(Devs & ADC_Flag)
   {
    printf("ADC checking voltage: ");
    if(NoPiping)printf(ValuesColor);
    printf("%d", TestData->TestVoltage);
    if(NoPiping)printf(UnitsColor);
    printf("mV");
    if(NoPiping)printf(TermColorsReset);
    printf("\n\r");
    //printf("ADC checking voltage: %s%d%smV%s\n\r", ValuesColor, TestData->TestVoltage, UnitsColor, TermColorsReset);
   }


  printf("The number interrations is: ");
  if(NoPiping)printf(ValuesColor);
  printf("%d", TestData->Num_Interations);
  if(NoPiping)printf(TermColorsReset);
  printf("\n\r");

  //printf("The number interrations is: %s%d%s\n\r", ValuesColor, TestData->Num_Interations, TermColorsReset);

  printf("\n\r");
 }

void PrintTestResults(TestResult_s const * const TestInfo)
 {
  uint8_t i, j;
  uint8_t Devs, Results;
  bool TRes;
  uint8_t MaxNameLen;
  bool NoPiping;
  NoPiping = isatty(STDOUT_FILENO); /* Checking if the output is not redirected to any other program or file to decide if to use colors or not. */

  printf("The test Results:\n\r");
  printf("Test ID: %d\n\r", TestInfo->Test_ID);

  Devs = *(uint8_t *)&TestInfo->Periph_B_F;
  Results = *(uint8_t *)&TestInfo->Results_B_F;
  
  
  for(i = 0, MaxNameLen = 0; i < E_NUM_PERIPHS; i++)
   {
    MaxNameLen = MAX(MaxNameLen, strlen(PeriphNames[i]));
   }

  printf("The tested devices:\n\r");
  for(i = 0; i < E_NUM_PERIPHS; i++)
   {
    if(Devs & (1<<i))
     {
      if(NoPiping)printf(DevsColor);
      printf("%s:    ", PeriphNames[i]);
      if(NoPiping)printf(TermColorsReset);
      //printf("%s%s:    %s", DevsColor, PeriphNames[i], TermColorsReset);
      for(j = 0; j < (MaxNameLen - strlen(PeriphNames[i])); j++)
       {
        printf(" ");
       }
      TRes = Results  & (1<<i);
      if(NoPiping)printf("%s", ResultColors[TRes]);
      printf("%s\n\r", ResultMessages[TRes]);
      if(NoPiping)printf(TermColorsReset);
      //printf("%s%s%s\n\r", ResultColors[TRes], ResultMessages[TRes], TermColorsReset);
     }
   }
  TRes = TestInfo->TestResult == E_TEST_SUCCEEDED;
  
  printf("The final Result: ");  
  if(NoPiping)printf("%s", ResultColors[TRes]);
  printf("%s", ResultMessages[TRes]);  
  if(NoPiping)printf(TermColorsReset);
  printf("\n\r");  
  //printf("The final Result: %s%s%s\n\r", ResultColors[TRes], ResultMessages[TRes], TermColorsReset);  
 }

void ConvertTime(time_t const * const TimeToConvert, char TimeAsStr[], size_t TimeStrSize)
 {
  struct tm tmp;
  localtime_r(TimeToConvert, &tmp);
  strftime(TimeAsStr, TimeStrSize, "%G/%m/%d - %H:%M:%S", &tmp);  
 }

/*======================================================================================================================*/
