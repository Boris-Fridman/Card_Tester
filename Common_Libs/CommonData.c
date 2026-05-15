#include "CommonData.h"

#include <string.h>



/*======================================================================================================================*/

char const * const ResultColors[] = {TermRed, TermGreen};
char const * const ResultMessages[] = {"Fail", "Pass"};

char const * const PeriphNames[E_NUM_PERIPHS] = {"Timer", "UART", "SPI", "I2C", "ADC"};

/*======================================================================================================================*/

PeriphBitField_s const ResetPeriphBitField = {.Timer_bf = 0, .UART_bf = 0, .SPI_bf = 0, .I2C_bf = 0, .ADC_bf = 0, .Reserved = 0, .OTA_UPDATE_bf = 1};
TestData_s const ResetCondition = {.Bit_Pattern_Length = 0, .Num_Interations = 0, .Periph_B_F = ResetPeriphBitField, .TestVoltage = 0, .Test_ID = 0};

/*======================================================================================================================*/

#define DEF_INIT_VAL 0xEF45AB12

#define POLYNOM   0x04C11DB7
#define CRC_SHIFT 0
#define MSB_MASK  0xAB25CD87

/*
 * *************************************************************************************************************
 **          CRC Checking Functions / Procedures
 * *************************************************************************************************************
 */
/*----------------------------------------------------------------------------------------------------------------------*/
/*   Calculates CRC from given block of data.                                                                           */
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

/*----------------------------------------------------------------------------------------------------------------------*/
/*   Appends to the end of the data array the calculated CRC from it. The length must include the place of the CRC.     */
/* For example if the data has length 8 the length given as parameter must be 12 = 8 + 4. The CRC has 4 bytes of length.*/
void Add_CRC(uint8_t buf[], size_t len)
 {
  uint32_t CalcCRC;
  CalcCRC = FindCRC(buf, len - CRC_SIZE, DEF_INIT_VAL);
  memcpy(buf + len - CRC_SIZE, &CalcCRC, CRC_SIZE);
 }
/*----------------------------------------------------------------------------------------------------------------------*/
/*  Checks if the CRC is correct.                                                                                       */
/*  For example if the given length is 12 the CRC checking will be made from the first 8 bytes                          */
/*  and the result will be compaired to the last 4 bytes.                                                               */
bool CRC_Correct(uint8_t buf[], size_t len)
 {
  uint32_t CalcCRC, RecvCRC;
  CalcCRC = FindCRC(buf, len - CRC_SIZE, DEF_INIT_VAL);
  memcpy(&RecvCRC, buf + len - CRC_SIZE, CRC_SIZE);
  return CalcCRC == RecvCRC;
 }

/*
 * *************************************************************************************************************
 **          Encoding Decoding Data Functions / Procedures
 * *************************************************************************************************************
 */

/*----------------------------------------------------------------------------------------------------------------------*/
/* This function encodes the packet received from network.                                                              */
ssize_t EncodeReqData(TestData_s const * const TestData, uint8_t TestPattern[], uint8_t **ReqData)
 {
  
  size_t PackSizeNetto, PackSizeFull;
  PackSizeNetto = sizeof(TestData_s) + TestData->Bit_Pattern_Length;
  PackSizeFull = PackSizeNetto + CRC_SIZE;
  *ReqData = calloc(PackSizeFull, sizeof(uint8_t));
  if(*ReqData)
   {
    memcpy(*ReqData, TestData, sizeof(TestData_s));
    memcpy(*ReqData + sizeof(TestData_s), TestPattern, TestData->Bit_Pattern_Length);
    Add_CRC(*ReqData, PackSizeFull);
    return PackSizeFull;
   }

  return 0;
 }

/*----------------------------------------------------------------------------------------------------------------------*/
/* This function decodes the packet received from network.                                                              */
/* Attention !!!                                                                                                        */
/* The last parameter "TestPattern" is given as pointer to pointer to dynamically allocated memory.                     */
/* That means that at the end of the program it must be freed by the procedure "FreeTestPattern()"                      */
/* to avoid the memory leakage.                                                                                         */
bool DecodeReqData(uint8_t Data[], size_t Len, TestData_s *TestData, uint8_t **TestPattern)
 {
  if(CRC_Correct(Data, Len))
   {
    memcpy(TestData, Data, sizeof(TestData_s));

    *TestPattern = calloc(TestData->Bit_Pattern_Length, sizeof(uint8_t));
    if(*TestPattern)
     {
      memcpy(*TestPattern, Data + sizeof(TestData_s),TestData->Bit_Pattern_Length);

      /* At the end of the usage with the returned data the "**TestPattern" must be freed by the "FreeTestPattern()" procedure. */
      /* free(*TestPattern); Not in use. the "**TestPattern" must be freed by the "FreeTestPattern()" procedure by the user. */
     }
    return true;
   }
  else
   return false;
 }

/*----------------------------------------------------------------------------------------------------------------------*/
/* This Procedure is used for freeing the "**TestPattern" reserved by the procedure "DecodeReqData()".                  */
/* No need to check anything before running it because it checks automatically inside if the memory                     */
/* is reserved and sets the pointer to NULL after freeing it.                                                           */
// void FreeTestPattern(uint8_t **TestPattern)
//  {
//   if(*TestPattern != NULL)
//    {
//     free(*TestPattern);
//     *TestPattern = NULL;
//    }
//  }

/*----------------------------------------------------------------------------------------------------------------------*/
/* Encodes data for response to the parameter "**RespData".                                                             */
/* Attention !!!                                                                                                        */
/* The procedure allocates dynamic memory for to which the parameter "**RespDtata" points.                              */
/* The freeing must be done by the "FreeRespData()".                                                                    */
size_t EncodeRespData(TestResult_s *TestResult, uint8_t **RespData)
 {
  size_t Len;
  Len = sizeof(TestResult_s) + CRC_SIZE;
  *RespData = calloc(Len, sizeof(uint8_t));
  if(*RespData)
   {
    memcpy(*RespData, TestResult, sizeof(TestResult_s));
    Add_CRC(*RespData, Len);
   }
  else
   Len = 0;

  return Len;
 }

/*----------------------------------------------------------------------------------------------------------------------*/
/* Decodes data for response to the parameter "**ResultData".                                                             */
bool DecodeRespData(uint8_t Data[], size_t Len, TestResult_s *ResultData)
 {
  if( (Len == sizeof(TestResult_s) + CRC_SIZE) && CRC_Correct(Data, Len) )  // Due to shortcirquit and method the function "CRC_Correct()" will not be done if the first condition is false in case of incorrect data length.
   {
    memcpy(ResultData, Data, sizeof(TestResult_s));
    return true;
   }
  return false;
 }

/*----------------------------------------------------------------------------------------------------------------------*/
/* Frees the "**RespData" allocated by the procedure "EncodeRespData()".                                                */
/* No need to check the condition. It checks if the pointer is not NULL and only in this case it frees the memory.      */
/* After freeng the memory it sets the pointer to NULL.                                                                 */
// void FreeRespData(uint8_t **RespData)
//  {
//   if(*RespData != NULL)
//    {
//     free(*RespData);
//     *RespData = NULL;
//    }
//  }

/*----------------------------------------------------------------------------------------------------------------------*/

void FreeData(uint8_t **Data)
 {
  if(*Data != NULL)
   {
    free(*Data);
    *Data = NULL;
   }
 }


/*======================================================================================================================*/



//  ₙⁿ𐄁 ⁰¹²³⁴⁵⁶⁷⁸  ₀₁₂₃₄₅₆₇₈₉  ⩽⩾
//  ₒᵤₜ ᵢₙ   
//  ≡≣≡≣
// ㎐㎑㎒㎓㎔
//          ------
//  ◀ ▶   ◀⸻ ⸺▶   ◀▬▬▬ ▬▬▬▶  <-- -->  🡄🬋🬋🡆  ⇶   ⇇⇉
//  ▤▥▦▧▨▩▪▫░▒▓
//  🔏🔐🔑🔒🔓 🚏  🚥🚦🚪


