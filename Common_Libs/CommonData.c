#include "CommonData.h"

#include <string.h>



#define POLYNOM  0x04C11DB7  
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
