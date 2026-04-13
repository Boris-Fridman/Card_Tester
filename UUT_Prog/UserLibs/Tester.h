/*
 * Tester.h
 *
 *  Created on: 13 Apr 2026
 *      Author: boris
 */

#ifndef ____TESTER_H__
#define ____TESTER_H__


#include <stdint.h>
#include <stdbool.h>

#include "CommonData.h"




void ReqForTest(TestData_s TestData, uint8_t TestPattern[]);


void TesterInit();


#endif /* ____TESTER_H__ */
