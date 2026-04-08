#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <libgen.h>
#include <time.h>
#include <pthread.h>
#include "CommonData.h"
    


void PrinthelpMessage(char *ProgName);
int CheckArgs(int argc, char *argv[], int *NInt, PeriphBitField_s *BFResult);
int CreateLoadDatabase(sqlite3 **conn);
int GetLastTestIDFromDataBase(sqlite3 **conn);
int WriteToDataBase(sqlite3 **conn, int test_id, char date_time[], bool result);

int main(int argc, char *argv[])
 {
  int NInt;
  int ArgResult;
  int test_id;
  sqlite3 *conn;
  time_t current_time; 
  struct tm tmp;
  char timebuf[100];

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
    CreateLoadDatabase(&conn); // Yes, the given pointer to database must be given as pointer to pointer to database because it's address is updated in this function.
    test_id = GetLastTestIDFromDataBase(&conn);

    time(&current_time);
    localtime_r(&current_time, &tmp);
    strftime(timebuf,sizeof(timebuf), "%G/%m/%d - %H:%M:%S", &tmp);

    WriteToDataBase(&conn, test_id + 1, timebuf, true);
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

int CreateLoadDatabase(sqlite3 **conn)
 {
  int result;
  if(*conn == NULL)
   return -2;  // The pointer to the database wasn't given.
  result = sqlite3_open("tests.sqlite3", conn);
  if(result!=SQLITE_OK)
   {
    fprintf(stderr, "Cannot open database: %s\n\r", sqlite3_errmsg(*conn));
    return -1;  // Couldn't create the database.
   }
  else
   {
    char *err_msg;
    result = sqlite3_exec(*conn, "CREATE TABLE IF NOT EXISTS TESTS_RESULTS(test_id INT, date_time TEXT, test_result INT);", 0, 0, &err_msg);
    sqlite3_close(*conn);
    return 0; // The database was loaded successfully.
   }
 }

int GetLastTestIDFromDataBase(sqlite3 **conn)
 {
  int result, valtoret = 0;
  sqlite3_stmt* stmt;

  result = sqlite3_open("tests.sqlite3", conn);

  result = sqlite3_prepare_v2(*conn, "SELECT test_id FROM TESTS_RESULTS;", -1, &stmt, 0);

  do
   {
    result = sqlite3_step(stmt);
    if(result == SQLITE_ROW)
    valtoret = sqlite3_column_int(stmt,0);
   }
  while(result == SQLITE_ROW);
  sqlite3_close(*conn);
  return valtoret;
 }

 int WriteToDataBase(sqlite3 **conn, int test_id, char date_time[], bool test_result)
  {
   char *err_msg;
   char *resstring[] = {"Fail", "Pass"};
   int result, valtoret = 0;
   //char buf[500];
   sqlite3_stmt* stmt = NULL;

   result = sqlite3_open("tests.sqlite3", conn);

   result = sqlite3_prepare_v2(*conn, "INSERT INTO TESTS_RESULTS (test_id, date_time, test_result) VALUES (?, ?, ?);", -1, &stmt, 0);

   result = sqlite3_bind_int (stmt, 1, test_id                     );
   result = sqlite3_bind_text(stmt, 2, date_time, -1, SQLITE_STATIC);
   result = sqlite3_bind_int (stmt, 3, test_result                 );

  //  snprintf(buf, sizeof(buf)-1, "INSERT INTO TESTS_RESULTS(test_id) VALUES(%d);"
  //                               "INSERT INTO TESTS_RESULTS(date_time) VALUES(%s);"
  //                               "INSERT INTO TESTS_RESULTS(test_result) VALUES(%d);",
  //           test_id, date_time,(int)test_result);


  //  snprintf(buf, sizeof(buf)-1, "INSERT INTO TESTS_RESULTS(test_id, date_time, test_result) VALUES(%d, %s, %d);",
  //           test_id, date_time,(int)test_result);

  // result = sqlite3_exec(*conn, buf, 0, 0, &err_msg);

  sqlite3_step(stmt);
  result = sqlite3_finalize(stmt);

  sqlite3_close(*conn);
  return valtoret;
  }