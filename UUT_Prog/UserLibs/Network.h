#ifndef ____Network_h__
#define ____Network_h__

#include <stdint.h>
#include <stdbool.h>
#include <CommonData.h>

/**
 * @brief Prepares the network task.
 *
 * @code{c}
 * void NetworkInit(void);
 * @code
 */
void NetworkInit(void);

/**
 * @brief The function is called from the "Tester.c/h" library and gives result to the "Network.c/h" for sending the answer back.
 *
 * @code{c}
 * void GiveResults(PeriphBitField_s DevResults, PeriphBitField_s DevsUnderTest, uint32_t Test_ID);
 * @code
 *
 * @param DevResults Is the bitfield of devices when each bit is referred to its device and set to "1" if its device passed the test.
 *
 * @param DevsUnderTest Is the bitfield of devices when each bit is referred to its device and set to "1" if its device was tested.
 *
 * @param Test_ID The test ID of the test of which the results were given.
 */
void GiveResults(PeriphBitField_s DevResults, PeriphBitField_s DevsUnderTest, uint32_t Test_ID);

#endif  /*  ____Network_h__  */
