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


/**
 *
 * @brief Burns the received segment.
 *
 * @code
 * void BurnData(TestData_s CodeSegInfo, uint8_t CodeSegment[], TestResult_s *BurnResult);
 * @code
 *
 * @param CodeSegInfo    Contains an information about received segment.
 *
 * @param CodeSegment    Code segment itself.
 *
 * @param BurnResult     Returned value given by pointer to memory where exists the information if the test passed or failed.
 */
void BurnData(TestData_s CodeSegInfo, uint8_t CodeSegment[], TestResult_s *BurnResult);

/**
 *
 * @brief  Gives result to main program if the burning of all of the program was finished. According to this result the main program decides if the bootloader must continue running or stop running and run the main program.
 *
 * @code
 * bool TheBurnIsFinished();
 * @code
 *
 * @return "true" if the program burning was finished or "false" otherwise.
 */
bool TheBurnIsFinished();


/**
 *
 * @brief Checks the main application existence at the beginning for bootloader to decide if it (bootloader) must run the main application (if it exists) or must run itself.
 *
 * @code
 * bool ApplicationExists();
 * @code
 *
 * @return "true" if the application exists or "false" if not.
 */
bool ApplicationExists();

#endif /* FLASH_H_ */
