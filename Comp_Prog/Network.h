#ifndef ____Network_h__
#define ____Network_h__

#include <stdint.h>
#include <stdbool.h>
#include "CommonData.h"

int InitNetwork();
void CloseNetwork();

int SendCommandToNetwork(TestData_s *TestData, uint8_t TestPattern[]);
int WaitForResponse(TestResult_s *ResultData, uint32_t TimeOut);

#endif  //  ____Network_h__
