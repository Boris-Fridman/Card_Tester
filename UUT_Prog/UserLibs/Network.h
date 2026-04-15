#ifndef ____Network_h__
#define ____Network_h__

#include <stdint.h>
#include <stdbool.h>
#include <CommonData.h>


void NetworkInit();
void GiveResults(PeriphBitField_s DevResults, uint32_t Test_ID);  // Not finished yet. Defined only for sucess compilation. The function is called from the "Tester.c/h" library and gives result to the "Network.c/h" for sending the answer back.

#endif  /*  ____Network_h__  */
