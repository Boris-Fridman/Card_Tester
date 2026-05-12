/*
 * Flash.c
 *
 *  Created on: 5 May 2026
 *      Author: boris
 */


#include "Flash.h"

#include "main.h"

#include "crc.h"
#include "stm32f7xx_hal_flash.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "SystemLib.h"


/*======================================================================================================================*/


HAL_StatusTypeDef ProgSegment(uint32_t StartAddress, uint8_t Segment[], uint8_t Length);

void ResetErasedSectorsStatuses();

uint8_t SectorToErase(uint32_t StartAddress, uint8_t Length);

uint8_t GetSectorNo(uint32_t Address);

uint32_t SectStartAddress(uint8_t SectNo);

/*======================================================================================================================*/

extern uint32_t _eidata;   /* End of data in RAM */
static uint32_t LastBLAddress = (uint32_t)&_eidata;
static volatile bool BurnEnded = false;
/*======================================================================================================================*/

void BurnData(TestData_s CodeSegInfo, uint8_t CodeSegment[], TestResult_s *BurnResult)
 {
  HAL_StatusTypeDef Result;
  uint8_t LastBLSector;
  uint8_t SectorForErasing;
  FLASH_EraseInitTypeDef pEraseInit = {0};
  uint32_t SectorError = 0;
  static BL_Conf_s BlConf;

  memset(BurnResult, 0, sizeof(TestResult_s));

  switch(CodeSegInfo.Num_Interations)  /* "Num_Interations" is used as Command Type. */
   {
    case OTA_START:
      Result = HAL_FLASH_Unlock();
      ResetErasedSectorsStatuses();
      BlConf.StartProgAddr = BlConf.EndProgAddr = NULL;
     break;
    case OTA_DATA:
      if(CodeSegInfo.Test_ID >= START_PROG_ADDRESS)
       {
        LastBLSector = GetSectorNo(LastBLAddress);
        SectorForErasing = SectorToErase(CodeSegInfo.Test_ID, CodeSegInfo.Bit_Pattern_Length);
        BlConf.StartProgAddr = (BlConf.StartProgAddr != NULL) ? BlConf.StartProgAddr : ((void*)CodeSegInfo.Test_ID);
        BlConf.EndProgAddr = (void*)MAX((uint32_t)BlConf.EndProgAddr, CodeSegInfo.Test_ID + CodeSegInfo.Bit_Pattern_Length);
        if((SectorForErasing != 0xFF) && (SectorForErasing > LastBLSector)) /* Protection against erasing flash page/sector with bootloader program. */
         {
          pEraseInit.NbSectors = 1;
          pEraseInit.Sector = (FLASH_SECTOR_0 + SectorForErasing);
          pEraseInit.TypeErase = FLASH_TYPEERASE_SECTORS;
          pEraseInit.VoltageRange = FLASH_VOLTAGE_RANGE_3;
          Result = HAL_FLASHEx_Erase(&pEraseInit, &SectorError);
         }
        if(CodeSegInfo.Test_ID > LastBLAddress)
         {
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
      BlConf.ProgCRC = 0;
      printf("%sThe program is located in the 0x%08lX : 0x%08lX%s\n\r", TermYello, (uint32_t)BlConf.StartProgAddr, (uint32_t)BlConf.EndProgAddr, TermColorsReset);
      printf("%sThe size is: 0x%08lX%s\n\r", TermYello, (uint32_t)BlConf.EndProgAddr - (uint32_t)BlConf.StartProgAddr, TermColorsReset);
      BlConf.ProgCRC = HAL_CRC_Calculate(&hcrc, (uint32_t*)BlConf.StartProgAddr, (uint32_t)BlConf.EndProgAddr - (uint32_t)BlConf.StartProgAddr);
      printf("%sThe CRC is: 0x%08lX%s\n\r",TermYello, BlConf.ProgCRC, TermColorsReset);
      SetBLConf(BlConf);

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
    Result = HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, StartAddress + StartPart, BlockParts[i]);
   }
  return Result;
 }






bool SectorNotErased[FLASH_SECTOR_TOTAL];

void ResetErasedSectorsStatuses()
 {
  for(uint8_t i = 0; i < FLASH_SECTOR_TOTAL; i++)
   SectorNotErased[i] = true;
 }


uint8_t SectorToErase(uint32_t StartAddress, uint8_t Length)
 {
  uint8_t i;
  uint32_t EndAddress = StartAddress + Length;
  for(i = 0; i < FLASH_SECTOR_TOTAL; i++)
   {
    if(   ( (StartAddress <= SectorsAddr[i].Start) && (EndAddress >= SectorsAddr[i].Start) ) ||
        ( ( (StartAddress >= SectorsAddr[i].Start) && (EndAddress <= SectorsAddr[i].End) ) && SectorNotErased[i] )   )
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
  for(i = 0; i < FLASH_SECTOR_TOTAL; i++)
   {
    if((Address >= SectorsAddr[i].Start) && (Address <= SectorsAddr[i].End))
     return i;
   }
  return 0xFF;
 }



inline uint32_t SectStartAddress(uint8_t SectNo)
 {
  return SectorsAddr[SectNo].Start;
 }


bool TheBurnIsFinished()
 {
  return BurnEnded;
 }


bool ApplicationExists()
 {
  BL_Conf_s BlConf;
  uint32_t FoundCRC;
  GetBLConf(&BlConf);
  MX_CRC_Init();  /* The initialization is required due to running this checking application function before all the initializations. */
  FoundCRC = HAL_CRC_Calculate(&hcrc, (uint32_t*)BlConf.StartProgAddr, (uint32_t)BlConf.EndProgAddr - (uint32_t)BlConf.StartProgAddr);
  return BlConf.ProgCRC == FoundCRC;
 }

