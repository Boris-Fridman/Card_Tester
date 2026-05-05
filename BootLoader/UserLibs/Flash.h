/*
 * Flash.h
 *
 *  Created on: 5 May 2026
 *      Author: boris
 */

#ifndef FLASH_H_
#define FLASH_H_

#include "CommonData.h"

#include <stdint.h>
#include <stdbool.h>


/*======================================================================================================================*/

void BurnData(TestData_s TestData, uint8_t TestPattern[], TestResult_s *TestResult);

#endif /* FLASH_H_ */
