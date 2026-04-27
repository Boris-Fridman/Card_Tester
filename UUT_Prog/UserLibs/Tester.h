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


/**
 * @brief Sends requrest for test
 *
 * @code{c}
 * void ReqForTest(TestData_s TestData, uint8_t TestPattern[]);
 * @endcode
 *
 * @param TestData is given as the structure containing all required parameters for the test except the Test Pattern that is given as the next parameter.
 * The parameters contained in the structure are: Test ID, TestTime if the timer must be tested, TestVoltage if the ADC must be tested,
 * "Periph_B_F" - Peripherals' bitfield where each bit means device to be tested. the bits if the device must be tested the referred bit will be "1",
 * Number of interactions the number of times the test must be done, Bit Pattern Length - the length of bit pattern that must be greater than zero
 * if UART, SPI or I2C must be tested. otherwise it will be zero.
 *
 * @param TestPattern The pointer to the pattern given for the test if the UART, SPI or I2C is included, otherwise the pointer will be NULL.
 *
 */
void ReqForTest(TestData_s TestData, uint8_t TestPattern[]);

/**
 * @brief Preperes the testertasks.
 *
 * @code{c}
 * void TesterInit(void);
 * @code
 *
 */
void TesterInit(void);


#endif /* ____TESTER_H__ */
