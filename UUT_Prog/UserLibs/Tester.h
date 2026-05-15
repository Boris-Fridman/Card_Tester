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
 * @brief Sends request for test
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
 * @brief Prepares the tester-tasks.
 *
 * @code{c}
 * void TesterInit(void);
 * @code
 *
 */
void TesterInit(void);

/**
 * @brief The selected devices are:
 *        ┏━━━━━━━━━━━┳━━━━━━━━━━━━━━━━━━━━━┳━━━━━━━━━━━━━━━━━━━━━━┓
 *        ┃           ┃        Tester       ┃        Tested        ┃
 *        ┃           ┃                     ┃                      ┃
 *        ┣━━━━━━━━━━━╋━━━━━━━━━━━━━━━━━━━━━╋━━━━━━━━━━━━━━━━━━━━━━┫
 *        ┃   Timer   ┃   Timer_1  Ch_1     ┃   Timer_2   Ch_2     ┃
 *        ┣━━━━━━━━━━━╋━━━━━━━━━━━━━━━━━━━━━╋━━━━━━━━━━━━━━━━━━━━━━┫
 *        ┃   UART    ┃   UART_6            ┃   UART_4             ┃
 *        ┣━━━━━━━━━━━╋━━━━━━━━━━━━━━━━━━━━━╋━━━━━━━━━━━━━━━━━━━━━━┫
 *        ┃   SPI     ┃   SPI_1    Master   ┃   SPI_4     Slave    ┃
 *        ┣━━━━━━━━━━━╋━━━━━━━━━━━━━━━━━━━━━╋━━━━━━━━━━━━━━━━━━━━━━┫
 *        ┃   I2C     ┃   I2C_1    Master   ┃   I2C_2     Slave    ┃
 *        ┣━━━━━━━━━━━╋━━━━━━━━━━━━━━━━━━━━━╋━━━━━━━━━━━━━━━━━━━━━━┫
 *        ┃   ADC     ┃   DAC               ┃   ADC_1     Ch_0     ┃
 *        ┗━━━━━━━━━━━┻━━━━━━━━━━━━━━━━━━━━━┻━━━━━━━━━━━━━━━━━━━━━━┛
 * 
 * 
 * @brief PinOuts given to each device and connection between the tester and the tested devices:
 *        ┏━━━━━━━━━━━━━━━━━━━━━┳━━━━━━━━━━━━┳━━━━━━━━━━━━━━━━━━━━━┓
 *        ┃    SPI_1 Tester     ┃ Direction  ┃    SPI_4 Tested     ┃
 *        ┃    Master           ┃            ┃    Slave            ┃
 *        ┣━━━━━━━━━━┳━━━━━━━━━━╋━━━━━━━━━━━━╋━━━━━━━━━━┳━━━━━━━━━━┫
 *        ┃   MOSI   ┃   PB5    ┃    ▬▬▬▶    ┃   PE6    ┃   MOSI   ┃
 *        ┣━━━━━━━━━━╋━━━━━━━━━━╋━━━━━━━━━━━━╋━━━━━━━━━━╋━━━━━━━━━━┫
 *        ┃   MISO   ┃   PA6    ┃    ◀▬▬▬    ┃   PE5    ┃   MISO   ┃
 *        ┣━━━━━━━━━━╋━━━━━━━━━━╋━━━━━━━━━━━━╋━━━━━━━━━━╋━━━━━━━━━━┫
 *        ┃   CLK    ┃   PA5    ┃    ▬▬▬▶    ┃   PE2    ┃   CLK    ┃
 *        ┣━━━━━━━━━━╋━━━━━━━━━━╋━━━━━━━━━━━━╋━━━━━━━━━━╋━━━━━━━━━━┫
 *        ┃   NSS    ┃   PA15   ┃    ▬▬▬▶    ┃   PE4    ┃   NSS    ┃
 *        ┗━━━━━━━━━━┻━━━━━━━━━━┻━━━━━━━━━━━━┻━━━━━━━━━━┻━━━━━━━━━━┛
 * 
 *        ┏━━━━━━━━━━━━━━━━━━━━━┳━━━━━━━━━━━━┳━━━━━━━━━━━━━━━━━━━━━┓
 *        ┃    I2C_1 Tester     ┃ Direction  ┃    I2C_2 Tested     ┃
 *        ┃    Master           ┃            ┃    Slave            ┃
 *        ┣━━━━━━━━━━┳━━━━━━━━━━╋━━━━━━━━━━━━╋━━━━━━━━━━┳━━━━━━━━━━┫
 *        ┃   SCL    ┃   PB8    ┃    ▬▬▬▶    ┃   PB10   ┃   SCL    ┃
 *        ┣━━━━━━━━━━╋━━━━━━━━━━╋━━━━━━━━━━━━╋━━━━━━━━━━╋━━━━━━━━━━┫
 *        ┃   SDA    ┃   PB9    ┃    ◀▬▬▶    ┃   PB11   ┃   SDA    ┃
 *        ┗━━━━━━━━━━┻━━━━━━━━━━┻━━━━━━━━━━━━┻━━━━━━━━━━┻━━━━━━━━━━┛
 * 
 *        ┏━━━━━━━━━━━━━━━━━━━━━┳━━━━━━━━━━━━┳━━━━━━━━━━━━━━━━━━━━━┓
 *        ┃    UART_6 Tester    ┃ Direction  ┃    UART_4 Tested    ┃
 *        ┣━━━━━━━━━━┳━━━━━━━━━━╋━━━━━━━━━━━━╋━━━━━━━━━━┳━━━━━━━━━━┫
 *        ┃   TX     ┃   PD8    ┃    ▬▬▬▶    ┃   PC11   ┃   RX     ┃
 *        ┣━━━━━━━━━━╋━━━━━━━━━━╋━━━━━━━━━━━━╋━━━━━━━━━━╋━━━━━━━━━━┫
 *        ┃   RX     ┃   PD9    ┃    ◀▬▬▬    ┃   PC10   ┃   TX     ┃
 *        ┗━━━━━━━━━━┻━━━━━━━━━━┻━━━━━━━━━━━━┻━━━━━━━━━━┻━━━━━━━━━━┛
 * 
 *        ┏━━━━━━━━━━━━━━━━━━━━━┳━━━━━━━━━━━━┳━━━━━━━━━━━━━━━━━━━━━┓
 *        ┃     DAC Tester      ┃ Direction  ┃  ADC_1 Ch_0 Tested  ┃
 *        ┣━━━━━━━━━━┳━━━━━━━━━━╋━━━━━━━━━━━━╋━━━━━━━━━━┳━━━━━━━━━━┫
 *        ┃   Aout   ┃   PA4    ┃    ▬▬▬▶    ┃   PA0    ┃   Ain    ┃
 *        ┗━━━━━━━━━━┻━━━━━━━━━━┻━━━━━━━━━━━━┻━━━━━━━━━━┻━━━━━━━━━━┛
 * 
 *        ┏━━━━━━━━━━━━━━━━━━━━━┳━━━━━━━━━━━━┳━━━━━━━━━━━━━━━━━━━━━┓
 *        ┃ Timer_1 Ch_1 Tester ┃ Direction  ┃ Timer_2 Ch_2 Tested ┃
 *        ┣━━━━━━━━━━┳━━━━━━━━━━╋━━━━━━━━━━━━╋━━━━━━━━━━┳━━━━━━━━━━┫
 *        ┃ PWM out  ┃   PE9    ┃    ▬▬▬▶    ┃   PB3    ┃  PWM in  ┃
 *        ┗━━━━━━━━━━┻━━━━━━━━━━┻━━━━━━━━━━━━┻━━━━━━━━━━┻━━━━━━━━━━┛
 * 
 */



#endif /* ____TESTER_H__ */
