#ifndef ____Network_h__
#define ____Network_h__

#include <stdint.h>
#include <stdbool.h>
#include "CommonData.h"

/*======================================================================================================================*/

/**
 * @brief Initilizes the network.
 * 
 * @code 
 * int InitNetwork();
 * @code
 * 
 * @return "0" The network was opened successfully. "-1" If the openning failed.
 */
int InitNetwork();

/**
 * @brief Closes network.
 * 
 * @code 
 * void CloseNetwork();
 * @code
 */
void CloseNetwork();

/*======================================================================================================================*/

/**
 * @brief Sends command via network to the slave device.
 * 
 * @code 
 * int SendCommandToNetwork(TestData_s const * const TestData, uint8_t TestPattern[]);
 * @code
 * 
 * @param TestData The data containing information about the test type.
 * 
 * @param TestPattern The array of bytes that must be sent via UART, SPI or I2C peripherals if they exist. Or NULL if no Pattern exist in command. 
 * 
 * @return "0" if the sending succeeded, "-1" if the sending failed or "-2" if the memory allocation failed.
 */
int SendCommandToNetwork(TestData_s const * const TestData, uint8_t TestPattern[]);

/**
 * @brief Waits for response for a while written in TimeOut value or infinittely if TimeOut is "0".
 * 
 * @code
 * int WaitForResponse(TestResult_s *ResultData, uint32_t TimeOut);
 * @code
 * 
 * @param ResultData   The pointer in the memory to which must be copied the data containing the results and information of the test.
 * 
 * @param TimeOut      The time given in milliseconds which the function must wait or zero ("0") if the function must wait forever.
 * 
 * @return "0" if the data was received successfully, "-1" if the data is corrupted or incorrect, "-2" if the data was cut or smaller than needed, "-3" if timout is incorrect or -4 any other or unknown error.
 */
int WaitForResponse(TestResult_s *ResultData, uint32_t TimeOut);

/*======================================================================================================================*/

#endif  //  ____Network_h__
