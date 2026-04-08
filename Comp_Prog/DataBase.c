#include "DataBase.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>


#define DB_FILENAME "tests.sqlite3"


int CreateLoadDatabase(sqlite3 **conn)
 {
  int result;
  if(*conn == NULL)
   return -2;  // The pointer to the database wasn't given.
  result = sqlite3_open(DB_FILENAME, conn);
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

  result = sqlite3_open(DB_FILENAME, conn);

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

 int WriteToDataBase(sqlite3 **conn, int test_id, char date_time[], TestResType_e test_result)
  {
   //char *err_msg;
   //char *resstring[] = {"Fail", "Pass", "No response"};
   int result, valtoret = 0;
   //char buf[500];
   sqlite3_stmt* stmt = NULL;

   result = sqlite3_open(DB_FILENAME, conn);

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


