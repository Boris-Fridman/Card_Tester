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

void BurnData(TestData_s CodeSegInfo, uint8_t CodeSegment[], TestResult_s *BurnResult);

#endif /* FLASH_H_ */
