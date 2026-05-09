/*
 * Flash.c
 *
 *  Created on: 5 May 2026
 *      Author: boris
 */


#include "Flash.h"

#include "main.h"

#include "stm32f7xx_hal_flash.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>




/*======================================================================================================================*/


HAL_StatusTypeDef ProgSegment(uint32_t StartAddress, uint8_t Segment[], uint8_t Length);

void ResetErasedSectorsStatuses();

uint8_t SectorToErase(uint32_t StartAddress, uint8_t Length);

uint8_t GetSectorNo(uint32_t Address);

uint32_t SectStartAddress(uint8_t SectNo);

/*======================================================================================================================*/

extern uint32_t _eidata;   /* End of data in RAM */
static uint32_t LastProgAddress = (uint32_t)&_eidata;
static volatile bool BurnEnded = false;
/*======================================================================================================================*/

void BurnData(TestData_s CodeSegInfo, uint8_t CodeSegment[], TestResult_s *BurnResult)
 {
  HAL_StatusTypeDef Result;
  uint8_t LastProgSector;
  uint8_t SectorForErasing;
  FLASH_EraseInitTypeDef pEraseInit = {0};
  uint32_t SectorError = 0;

  memset(BurnResult, 0, sizeof(TestResult_s));

  switch(CodeSegInfo.Num_Interations)  /* "Num_Interations" is used as Command Type. */
   {
    case OTA_START:
      Result = HAL_FLASH_Unlock();
      ResetErasedSectorsStatuses();
     break;
    case OTA_DATA:
      if(CodeSegInfo.Test_ID >= START_PROG_ADDRESS)
       {
        LastProgSector = GetSectorNo(LastProgAddress);
        SectorForErasing = SectorToErase(CodeSegInfo.Test_ID, CodeSegInfo.Bit_Pattern_Length);
        if((SectorForErasing != 0xFF) && (SectorForErasing > LastProgSector))
         {
          Result = 0;
          //  --  FLASH_Erase_Sector(SectStartAddress(SectorForErasing), FLASH_VOLTAGE_RANGE_4);
          //FLASH_Erase_Sector(SectorForErasing, FLASH_VOLTAGE_RANGE_4);
          pEraseInit.NbSectors = 1;
          pEraseInit.Sector = (FLASH_SECTOR_0 + SectorForErasing);
          pEraseInit.TypeErase = FLASH_TYPEERASE_SECTORS;
          pEraseInit.VoltageRange = FLASH_VOLTAGE_RANGE_3;
//          SCB_CleanDCache();
//          SCB_DisableDCache();
//          SCB_DisableICache();
//          __HAL_FLASH_ART_DISABLE(); // Disable ART Accelerator
//          uint32_t primask_bit = __get_PRIMASK();
          //__disable_irq();
          //__HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_OPERR);
          Result = HAL_FLASHEx_Erase(&pEraseInit, &SectorError);
          //__enable_irq();
//          __set_PRIMASK(primask_bit);
//          __HAL_FLASH_ART_ENABLE();
//          SCB_EnableICache();
//          SCB_EnableDCache();
         }
        if(CodeSegInfo.Test_ID > LastProgAddress)
         {
          //Result = 0;
          Result = ProgSegment(CodeSegInfo.Test_ID, CodeSegment, CodeSegInfo.Bit_Pattern_Length);
         }

        else
         Result = HAL_ERROR;
       }
      else
       {
        Result = HAL_ERROR;
       }
     break;
    case OTA_END:
      Result = HAL_FLASH_Lock();
      BurnEnded = true;
     break;
   }


  BurnResult->Test_ID = CodeSegInfo.Test_ID;
  BurnResult->Results_B_F.OTA_UPDATE_bf = 1;
  BurnResult->TestResult = ((Result == HAL_OK) ? E_TEST_SUCCEEDED : E_TEST_FAILED);

 }

HAL_StatusTypeDef ProgSegment(uint32_t StartAddress, uint8_t Segment[], uint8_t Length)
 {
  HAL_StatusTypeDef Result;
  uint8_t i;
  uint8_t StartPart;
  uint8_t NumParts;
  const uint8_t PART_SIZE = sizeof(uint32_t);
  const uint8_t TOTAL_NUM_PARTS = MAX_TEST_PATTERN_SIZE / PART_SIZE;

  uint32_t BlockParts[TOTAL_NUM_PARTS];
  memset(&BlockParts, 0xFF, sizeof(BlockParts));
  memcpy(&BlockParts, Segment, Length);
  NumParts = DIV_RND_UP(Length, PART_SIZE);

  for(i = 0; i < NumParts; i++)
   {
    Result = 0;
    StartPart = i * PART_SIZE;
    //Result = HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, StartAddress + StartPart, i);  // For test only.
    Result = HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, StartAddress + StartPart, BlockParts[i]);
   }
  return Result;
 }



typedef struct SectorAddr_s
 {
  uint32_t Start;
  uint32_t End;
 }SectorAddr_s;


#define NUM_SECTORS 8
SectorAddr_s const Sectors[NUM_SECTORS] =
  {
    {0x08000000 , 0x08007FFF},
    {0x08008000 , 0x0800FFFF},
    {0x08010000 , 0x08017FFF},
    {0x08018000 , 0x0801FFFF},
    {0x08020000 , 0x0803FFFF},
    {0x08040000 , 0x0807FFFF},
    {0x08080000 , 0x080BFFFF},
    {0x080C0000 , 0x080FFFFF}
  };

bool SectorNotErased[NUM_SECTORS];

void ResetErasedSectorsStatuses()
 {
  for(uint8_t i = 0; i < NUM_SECTORS; i++)
   SectorNotErased[i] = true;
 }


uint8_t SectorToErase(uint32_t StartAddress, uint8_t Length)
 {
  uint8_t i;
  uint32_t EndAddress = StartAddress + Length;
  for(i = 0; i < NUM_SECTORS; i++)
   {
    if(   ( (StartAddress <= Sectors[i].Start) && (EndAddress >= Sectors[i].Start) ) ||
        ( ( (StartAddress >= Sectors[i].Start) && (EndAddress <= Sectors[i].End) ) && SectorNotErased[i] )   )
     {
      SectorNotErased[i] = false;
      return i;
     }
   }
  return 0xFF;
 }

uint8_t GetSectorNo(uint32_t Address)
 {
  uint8_t i;
  for(i = 0; i < NUM_SECTORS; i++)
   {
    if((Address >= Sectors[i].Start) && (Address <= Sectors[i].End))
     return i;
   }
  return 0xFF;
 }



inline uint32_t SectStartAddress(uint8_t SectNo)
 {
  return Sectors[SectNo].Start;
 }


bool TheBurnIsFinished()
 {
  return BurnEnded;
 }


