#include "Network.h"

static int TestID;                     // Defined temperary for test. Will be removed.
static PeriphBitField_s Periph_B_F;    // Defined temperary for test. Will be removed.

int SendCommandToNetwork(TestData_s *TestData, uint8_t TestPattern[])
 {
  TestID = TestData->Test_ID;
  Periph_B_F = TestData->Periph_B_F;
  return 0;
 }

int WaitForResponse(TestResult_s *ResultData, uint32_t TimeOut)
 {
  int r;
  ResultData->Periph_B_F = Periph_B_F;
  ResultData->Test_ID = TestID;
  r = rand();
  ResultData->TestResult = (r % 2) ? E_TEST_SUCCEEDED : E_TEST_FAILED;
  return 0;
 }


