#ifndef ____DataBase_h__
#define ____DataBase_h__


#include <stdint.h>
#include <stdbool.h>
#include <time.h>
#include "CommonData.h"
#include <sqlite3.h>


int CreateLoadDatabase(sqlite3 **conn);
int GetLastTestIDFromDataBase(sqlite3 **conn);
int WriteToDataBase(sqlite3 **conn, int test_id, char date_time[], TestResType_e result);




#endif  //  ____DataBase_h__