#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <libgen.h>
#include <time.h>
#include "CommonData.h"
#include "DataBase.h"
#include "Network.h"
    

void PrinthelpMessage(char *ProgName);
int CheckArgs(int argc, char *argv[], int *NInt, PeriphBitField_s *BFResult);
void PrepereData(TestData_s *TestData, uint8_t TestPattern[], uint32_t *TimeOut, PeriphBitField_s PeriphBF, int NInt, int TestID);
void ConvertTime(time_t const * const TimeToConvert, char TimeAsStr[], size_t TimeStrSize);

int main(int argc, char *argv[])
 {
  int NInt;
  int ArgResult;
  int TestID;
  sqlite3 *conn;
  time_t CurrentTime; 
  char timebuf[100];
  PeriphBitField_s PeriphBF;
  TestResult_s ResultData;
  uint32_t TimeOut;

  TestData_s TestData;
  uint8_t TestPattern[MAX_TEST_PATTERN_SIZE] = {0};

  //ansi clear screen
  printf("\033[2J\033[H");
  
  //code
  srand(time(NULL));
  printf("printing arguments...\n\r");
  int i;
  for(i = 0; i < argc; i++)
   {
    printf("%3d: %s\r\n", i, argv[i]);
   }
  
  ArgResult = CheckArgs(argc, argv, &NInt, &PeriphBF);
  if(ArgResult == 0)
   {
    CreateLoadDatabase(&conn); // Yes, the given pointer to database must be given as pointer to pointer to database because it's address is updated in this function.
    TestID = GetLastTestIDFromDataBase(&conn);
    TestID++;

    time(&CurrentTime);
    ConvertTime(&CurrentTime, timebuf,sizeof(timebuf));

    PrepereData(&TestData, TestPattern, &TimeOut, PeriphBF, NInt, TestID);

    InitNetwork();
    SendCommandToNetwork(&TestData, TestPattern);
    WaitForResponse(&ResultData, TimeOut);
    CloseNetwork();

    WriteToDataBase(&conn, ResultData.Test_ID, timebuf, ResultData.TestResult);
   }

  return 0;
 }

int CheckArgs(int argc, char *argv[], int *NInt,PeriphBitField_s *BFResult)
 {
  int i;
  size_t j;
  char *st, ch;
  int nintres;
  bool nintloaded = false;
  
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
             PrinthelpMessage(argv[0]);
             return 1;
            break;
         }
       }
     }
   }
  if(!nintloaded)
   {
    if(nintres ==  EOF)
     fprintf(stderr, "The parameter nxx is incorrect.");
    else
     fprintf(stderr, "Was not loaded.");
    fprintf(stderr, "Using default number inteructions that is \"1\"\n\r");
    *NInt = -1;
   }            

  return 0;
 }

void PrinthelpMessage(char *ProgName)
 {
  //UNUSED(ProgName);
  char *fname = basename(ProgName);
  printf("To use the program it's needed to type the next parameters:\n\r");
  printf("%s [test [t][u][s][i][a][r]] [n] \n\r", fname);
  printf("t  - Timer\n\r");
  printf("u  - UART/USART\n\r");
  printf("s  - SPI\n\r");
  printf("i  - I2C\n\r");
  printf("a  - ADC\n\r");
  printf("nx - Number of interactions. Where x is number. Default is 1.");
  printf("Or to type h to see the help message.\n\r");
 }


void PrepereData(TestData_s *TestData, uint8_t TestPattern[], uint32_t *TimeOut, PeriphBitField_s PeriphBF, int NInt, int TestID)
 {
  uint32_t MinFreq, MaxAddDelay = 0;

  MaxAddDelay = 0;
  if((PeriphBF.I2C_bf)||(PeriphBF.SPI_bf)||(PeriphBF.UART_bf))
   {
    MinFreq = MAX_POSSIBLE_FREAUENCY;
    if(PeriphBF.I2C_bf)   MinFreq = MIN(MinFreq, MIN_I2C_FREQUENCY);
    if(PeriphBF.SPI_bf)   MinFreq = MIN(MinFreq, MIN_SPI_FREQUENCY);
    if(PeriphBF.UART_bf)  MinFreq = MIN(MinFreq, MIN_UART_FREQUENCY);
    MaxAddDelay += 1000 * TestData->Bit_Pattern_Length * 8 / MinFreq;
    TestData->Bit_Pattern_Length = rand() % MAX_TEST_PATTERN_SIZE;  // Determining Test Pattern Length randomly but less than Maximum permitted length.
    for(uint8_t i = 0; i< TestData->Bit_Pattern_Length; i++)
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
    TestData->TestTime = rand() % (1000);    // In ms
    MaxAddDelay += TestData->TestTime;
   }
  else
   {
    TestData->TestTime = 0;
   }
  if(PeriphBF.ADC_bf)
   {
    TestData->TestVoltage = rand() % (5000); // In mV
   }
  else
   {
    TestData->TestVoltage = 0;
   }
  TestData->Num_Interations = NInt;
  TestData->Periph_B_F = PeriphBF;
  TestData->Test_ID = TestID;
  TestData->TestVoltage = rand() % (5000); // In mV
  *TimeOut = MIN_TIME_OUT + MaxAddDelay;

 }

void ConvertTime(time_t const * const TimeToConvert, char TimeAsStr[], size_t TimeStrSize)
 {
  struct tm tmp;
  localtime_r(TimeToConvert, &tmp);
  strftime(TimeAsStr, TimeStrSize, "%G/%m/%d - %H:%M:%S", &tmp);  
 }


