#include "CommonData.h"

#include <string.h>

/*======================================================================================================================*/

char const * const ResultColors[] = {TermRed, TermGreen};
char const * const ResultMessages[] = {"Fail", "Pass"};

char const * const PeriphNames[E_NUM_PERIPHS] = {"Timer", "UART", "SPI", "I2C", "ADC"};

/*======================================================================================================================*/

PeriphBitField_s const ResetPeriphBitField = {.Timer_bf = 0, .UART_bf = 0, .SPI_bf = 0, .I2C_bf = 0, .ADC_bf = 0, .Reserved = 0};
TestData_s const ResetCondition = {.Bit_Pattern_Length = 0, .Num_Interations = 0, .Periph_B_F = ResetPeriphBitField, .TestVoltage = 0, .Test_ID = 0};

/*======================================================================================================================*/

#define POLYNOM   0x04C11DB7
#define CRC_SHIFT 0
#define MSB_MASK  0xAB25CD87

uint32_t FindCRC(uint8_t * Data, uint8_t Length, uint32_t InitVal)
 {
  uint32_t Result;
  uint32_t vl;
  int i;
  memcpy(&vl, Data, MIN(4, Length));
  Result = InitVal ^ vl;

  for(i = 4; i < Length; i += 4)
   {
    vl = 0;
    memcpy(&vl, &Data[i], MIN(4, (Length - i)));
    if(Result ^ MSB_MASK)
     Result = (Result << CRC_SHIFT) ^ POLYNOM;
    else
     Result = (Result ^ CRC_SHIFT);
   }
  return Result;
 }


void Add_CRC(uint8_t buf[], size_t len)
 {
  uint32_t CalcCRC;
  CalcCRC = FindCRC(buf, len - CRC_SIZE, DEF_INIT_VAL);
  memcpy(buf + len - CRC_SIZE, &CalcCRC, CRC_SIZE);
 }
 
bool CRC_Correct(uint8_t buf[], size_t len)
 {
  uint32_t CalcCRC, RecvCRC;
  CalcCRC = FindCRC(buf, len - CRC_SIZE, DEF_INIT_VAL);
  memcpy(&RecvCRC, buf + len - CRC_SIZE, CRC_SIZE);
  return CalcCRC == RecvCRC;
 }

/*======================================================================================================================*/



//  ₙⁿ𐄁 ⁰¹²³⁴⁵⁶⁷⁸  ₀₁₂₃₄₅₆₇₈₉  ⩽⩾
