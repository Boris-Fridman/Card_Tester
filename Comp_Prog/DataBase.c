#include "DataBase.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <main.h>

/*======================================================================================================================*/

#define DB_FILENAME "tests.sqlite3"

/*======================================================================================================================*/

void PrintDBError(int ErrorCode);

/*======================================================================================================================*/

int CreateLoadDatabase(sqlite3 **conn)
 {
  int result;
  bool NoPiping;
  NoPiping = isatty(STDERR_FILENO); /* Checking if the output is not redirected to any other program or file to decide if to use colors or not. */

  if(*conn == NULL)
   return -2;  /* The pointer to the database wasn't given. */
  result = sqlite3_open(DB_FILENAME, conn);
  if(result != SQLITE_OK)
   {
    if(NoPiping)fprintf(stderr, TermRed);
    fprintf(stderr, "Cannot open database: %s\n\r", sqlite3_errmsg(*conn));
    if(NoPiping)fprintf(stderr, TermColorsReset);
    return -1;  /* Couldn't create the database. */
   }
  else
   {
    char *err_msg;
    result = sqlite3_exec(*conn, "CREATE TABLE IF NOT EXISTS TESTS_RESULTS(test_id INT, date_time TEXT, test_result INT);", 0, 0, &err_msg);
    if(result != SQLITE_OK)
     {
      if(NoPiping)fprintf(stderr, TermRed);
      fprintf(stderr, "Cannot prepare the table: %s\n\r", sqlite3_errmsg(*conn));
      if(NoPiping)fprintf(stderr, TermColorsReset);
     }
    sqlite3_close(*conn);
    return 0; /* The database was loaded successfully. */
   }
 }

int GetLastTestIDFromDataBase(sqlite3 **conn)
 {
  int result, valtoret = 0, v;
  sqlite3_stmt* stmt;
  bool NoPiping;
  NoPiping = isatty(STDERR_FILENO); /* Checking if the output is not redirected to any other program or file to decide if to use colors or not. */

  result = sqlite3_open(DB_FILENAME, conn);
  if(result != SQLITE_OK)
   {
    if(NoPiping)fprintf(stderr, TermRed);
    fprintf(stderr, "Cannot open the file with table: %s\n\r", sqlite3_errmsg(*conn));
    if(NoPiping)fprintf(stderr, TermColorsReset);
   }
  else
   {
    result = sqlite3_prepare_v2(*conn, "SELECT test_id FROM TESTS_RESULTS;", -1, &stmt, 0);
    if(result != SQLITE_OK)
     {
      if(NoPiping)fprintf(stderr, TermRed);
      fprintf(stderr, "Cannot prepere the table: %s\n\r", sqlite3_errmsg(*conn));
      if(NoPiping)fprintf(stderr, TermColorsReset);
     }
    else
     {
      do
       {
        result = sqlite3_step(stmt);
        if(result == SQLITE_ROW)
         {
          v = sqlite3_column_int(stmt,0);
          valtoret = MAX(valtoret, v);
         }
         
       }
      while(result == SQLITE_ROW);
     }
   }
  sqlite3_close(*conn);
  return valtoret;
 }

 int WriteToDataBase(sqlite3 **conn, int test_id, char date_time[], TestResType_e test_result)
  {
   //char *err_msg;
   //char *resstring[] = {"Fail", "Pass", "No response"};
   int valtoret = SQLITE_OK;
   //char buf[500];
   sqlite3_stmt* stmt = NULL;

   valtoret = sqlite3_open(DB_FILENAME, conn);
   if(valtoret != SQLITE_OK)
    {
     PrintDBError(valtoret);
     return valtoret;
    }
   else
    {
     do
      {
        valtoret = sqlite3_prepare_v2(*conn, "INSERT INTO TESTS_RESULTS (test_id, date_time, test_result) VALUES (?, ?, ?);", -1, &stmt, 0);
        if(valtoret != SQLITE_OK){PrintDBError(valtoret);break;}     
        valtoret = sqlite3_bind_int (stmt, 1, test_id                     );
        if(valtoret != SQLITE_OK){PrintDBError(valtoret);break;}
        valtoret = sqlite3_bind_text(stmt, 2, date_time, -1, SQLITE_STATIC);
        if(valtoret != SQLITE_OK){PrintDBError(valtoret);break;}
        valtoret = sqlite3_bind_int (stmt, 3, test_result                 );
        if(valtoret != SQLITE_OK){PrintDBError(valtoret);break;}
       //  snprintf(buf, sizeof(buf)-1, "INSERT INTO TESTS_RESULTS(test_id) VALUES(%d);"
       //                               "INSERT INTO TESTS_RESULTS(date_time) VALUES(%s);"
       //                               "INSERT INTO TESTS_RESULTS(test_result) VALUES(%d);",
       //           test_id, date_time,(int)test_result);
     
     
       //  snprintf(buf, sizeof(buf)-1, "INSERT INTO TESTS_RESULTS(test_id, date_time, test_result) VALUES(%d, %s, %d);",
       //           test_id, date_time,(int)test_result);
     
       // result = sqlite3_exec(*conn, buf, 0, 0, &err_msg);
     
       sqlite3_step(stmt);
       valtoret = sqlite3_finalize(stmt);
       if(valtoret != SQLITE_OK){PrintDBError(valtoret);break;}
      } 
     while (0);
     sqlite3_close(*conn);
    }
  return valtoret;
  }


void PrintDBError(int ErrorCode)
 {
  bool NoPiping;
  NoPiping = isatty(STDERR_FILENO); /* Checking if the output is not redirected to any other program or file to decide if to use colors or not. */
  if(NoPiping) fprintf(stderr, "%s", ResultColors[!ErrorCode]);
  switch(ErrorCode)
   {
    case SQLITE_OK         : fprintf(stderr, "Successful result\n\r"); break;
    case SQLITE_ERROR      : fprintf(stderr, "Generic error\n\r"); break;
    case SQLITE_INTERNAL   : fprintf(stderr, "Internal logic error in SQLite\n\r"); break;
    case SQLITE_PERM       : fprintf(stderr, "Access permission denied\n\r"); break;
    case SQLITE_ABORT      : fprintf(stderr, "Callback routine requested an abort\n\r"); break;
    case SQLITE_BUSY       : fprintf(stderr, "The database file is locked\n\r"); break;
    case SQLITE_LOCKED     : fprintf(stderr, "A table in the database is locked\n\r"); break;
    case SQLITE_NOMEM      : fprintf(stderr, "A malloc() failed\n\r"); break;
    case SQLITE_READONLY   : fprintf(stderr, "Attempt to write a readonly database\n\r"); break;
    case SQLITE_INTERRUPT  : fprintf(stderr, "Operation terminated by sqlite3_interrupt(\n\r"); break;
    case SQLITE_IOERR      : fprintf(stderr, "Some kind of disk I/O error occurred\n\r"); break;
    case SQLITE_CORRUPT    : fprintf(stderr, "The database disk image is malformed\n\r"); break;
    case SQLITE_NOTFOUND   : fprintf(stderr, "Unknown opcode in sqlite3_file_control()\n\r"); break;
    case SQLITE_FULL       : fprintf(stderr, "Insertion failed because database is full\n\r"); break;
    case SQLITE_CANTOPEN   : fprintf(stderr, "Unable to open the database file\n\r"); break;
    case SQLITE_PROTOCOL   : fprintf(stderr, "Database lock protocol error\n\r"); break;
    case SQLITE_EMPTY      : fprintf(stderr, "Internal use only\n\r"); break;
    case SQLITE_SCHEMA     : fprintf(stderr, "The database schema changed\n\r"); break;
    case SQLITE_TOOBIG     : fprintf(stderr, "String or BLOB exceeds size limit\n\r"); break;
    case SQLITE_CONSTRAINT : fprintf(stderr, "Abort due to constraint violation\n\r"); break;
    case SQLITE_MISMATCH   : fprintf(stderr, "Data type mismatch\n\r"); break;
    case SQLITE_MISUSE     : fprintf(stderr, "Library used incorrectly\n\r"); break;
    case SQLITE_NOLFS      : fprintf(stderr, "Uses OS features not supported on host\n\r"); break;
    case SQLITE_AUTH       : fprintf(stderr, "Authorization denied\n\r"); break;
    case SQLITE_FORMAT     : fprintf(stderr, "Not used\n\r"); break;
    case SQLITE_RANGE      : fprintf(stderr, "2nd parameter to sqlite3_bind out of range\n\r"); break;
    case SQLITE_NOTADB     : fprintf(stderr, "File opened that is not a database file\n\r"); break;
    case SQLITE_NOTICE     : fprintf(stderr, "Notifications from sqlite3_log()\n\r"); break;
    case SQLITE_WARNING    : fprintf(stderr, "Warnings from sqlite3_log()\n\r"); break;
    case SQLITE_ROW        : fprintf(stderr, "sqlite3_step() has another row ready\n\r"); break;
    case SQLITE_DONE       : fprintf(stderr, "sqlite3_step() has finished executing\n\r"); break;
   }
  if(NoPiping) fprintf(stderr, "%s", TermColorsReset);
 }


/*======================================================================================================================*/

