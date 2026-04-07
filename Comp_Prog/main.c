#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <libgen.h>
#include "CommonData.h"
    


void PrinthelpMessage(char *ProgName);
int CheckArgs(int argc, char *argv[], int *NInt, PeriphBitField_s *BFResult);
int CreateLoadDatabase();

int main(int argc, char *argv[])
 {
  int NInt;
  int ArgResult;

  PeriphBitField_s PeriphBF;

  //ansi clear screen
  printf("\033[2J\033[H");
  
  //code
  printf("printing arguments...\n\r");
  int i;
  for(i = 0; i < argc; i++)
   {
    printf("%3d: %s\r\n", i, argv[i]);
   }
  
  ArgResult = CheckArgs(argc, argv, &NInt, &PeriphBF);
  if(ArgResult == 0)
   {
    CreateLoadDatabase();

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
             BFResult->Timer = 1;
            break;
           case 'u':
             BFResult->UART = 1;
            break;
           case 's':
             BFResult->SPI = 1;
            break;
           case 'i':
             BFResult->I2c = 1;
            break;
           case 'a':
             BFResult->ADC = 1;
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

int CreateLoadDatabase()
 {
  sqlite3 *db;
  int rc;
  rc = sqlite3_open("tests.sqlite3", &db);
  if(rc!=SQLITE_OK)
   {
    fprintf(stderr, "Cannot open database: %s\n\r", sqlite3_errmsg(db));
    return 1;
   }
  else
   {
    char *err_msg;
    rc = sqlite3_exec(db, "CREATE TABLE IF NOT EXISTS TESTS_RESULTS(test_id INT, date_time TEXT, test_result INT);", 0, 0, &err_msg);
    sqlite3_close(db);
    return 0;
   }
 }