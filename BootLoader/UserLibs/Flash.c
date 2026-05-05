/*
 * Flash.c
 *
 *  Created on: 5 May 2026
 *      Author: boris
 */


#include "Flash.h"


#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/*======================================================================================================================*/


void BurnData(TestData_s TestData, uint8_t TestPattern[], TestResult_s *TestResult)
 {
  memset(TestResult, 0, sizeof(TestResult_s));
  TestResult->Test_ID = TestData.Test_ID;
  TestResult->Results_B_F.OTA_UPDATE_bf = 1;
  TestResult->TestResult = E_TEST_SUCCEEDED;

 }

