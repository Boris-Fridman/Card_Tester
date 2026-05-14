#ifndef ____DataBase_h__
#define ____DataBase_h__

/*======================================================================================================================*/

#include <stdint.h>
#include <stdbool.h>
#include <time.h>
#include "CommonData.h"
#include <sqlite3.h>

/*======================================================================================================================*/

/**
 * @brief Loads existing database or creates a new one if any database doesn't exist.
 * 
 * @code
 * int CreateLoadDatabase(sqlite3 **conn);
 * @code
 * 
 * @param conn pointer to the sqlite database handle
 * 
 * @return "0" if success, "-1" if the database cannot be created or -2 if the pointer is problematic or not given.
 */
int CreateLoadDatabase(sqlite3 **conn);

/** 
 * @brief Checks all the test ids in the database and returns the biggest one. (It's required to decide which test id must be given to the new test.)
 * 
 * @code
 * int GetLastTestIDFromDataBase(sqlite3 **conn);
 * @code
 * 
 * @param conn pointer to the sqlite database handle.
 * 
 * @return The last test id.
 */
int GetLastTestIDFromDataBase(sqlite3 **conn);

/** 
 * @brief Writes results of the main test.
 * 
 * @code
 * int WriteToDataBase(sqlite3 **conn, int test_id, char date_time[], TestResType_e result);
 * @code
 * 
 * @param conn pointer to the sqlite database handle.
 * 
 * @param test_id the id of the made test. 
 * 
 * @param date_time the date and the time when the test was made.
 * 
 * @param result result of the made test: "E_TEST_SUCCEEDED" (== "0") if the test passed or "E_TEST_FAILED" (== "0xFF") if the test failed.
 * 
 * @return One of the database CAPI3REF Result Codes. (See lines 434 - 476 in the "sqlite3.h"-file).
 */
int WriteToDataBase(sqlite3 **conn, int test_id, char date_time[], TestResType_e result);


/*======================================================================================================================*/


#endif  //  ____DataBase_h__