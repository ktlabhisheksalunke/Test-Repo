
/*
  *****************************************************************************
  * @file    UserEeprom.c
  * @author  Anil More
  * @version
  * @date
  * @brief   EEPROM over I2C functions
*******************************************************************************
*/
/******************************************************************************

            Copyright (c) by KloudQ Technologies Limited.

  This software is copyrighted by and is the sole property of KloudQ
  Technologies Limited.
  All rights, title, ownership, or other interests in the software remain the
  property of  KloudQ Technologies Limited. This software may only be used in
  accordance with the corresponding license agreement. Any unauthorized use,
  duplication, transmission, distribution, or disclosure of this software is
  expressly forbidden.

  This Copyright notice may not be removed or modified without prior written
  consent of KloudQ Technologies Limited.

  KloudQ Technologies Limited reserves the right to modify this software
  without notice.

  KloudQ Technologies Limited

*******************************************************************************
*/

#include "user_eeprom.h"
#include "externs.h"
#include "error_handler.h"
#include "user_rtc.h"
#include "user_HourMeter.h"
#include "user_ApplicationDefines.h"

stEepromBuffMem g_stEepromBuffMemAdd;

HAL_StatusTypeDef HalMemWrStatus = HAL_ERROR;
uint32_t u32MemWriteCntr =0;

/* Global variable Definition End */


/* Private Functions Declaration Start */
//static uint32_t EEPROM_CalcCheckSum(uint8_t* pu8Data, uint16_t u16Size);
/* Private Functions Declaration End  */

/* Private Functions Definition Start */
//static uint32_t EEPROM_CalcCheckSum(uint8_t* pu8Data, uint16_t u16Size)
//{
//	uint32_t u32Checksum = 0;
//	uint16_t u16LoopCntr = 0;
//	for(u16LoopCntr =0;u16LoopCntr<(u16Size-4);u16LoopCntr++)
//	{
//		u32Checksum += (pu8Data[u16LoopCntr]);
//	}
//
//	//Increment it by one for Ignoring the  All Zeros data
//	u32Checksum++;
//	return u32Checksum;
//}

/*Hour meter Data write.
 * 1. 16-bit data Hours data.
 * 2. 8-bit Minutes data.
 * 3. 8-bit For Checksum
 *
 * At Power On Read the EEPROM ADD 0x00 to 0x3 total 4 bytes. for first copy of HR meter data
 * If First copy CRC is Invalid then read second copy 0x04 to 0x07 Second copy If Both Copy are corrupted the write default Value as 0.0
 *
 * If First Copy Hour Meter data is greater than or equal to 16000.( It means Fist Location Write cycles count are
 * completed or near about to complete.
 * Then read Second Location 0x08 to 0x0B first copy. if this is corrupted. then read second copy. 0x0C to 0x0f. if this is currptted then set this default.
 *
 *
 *
 * */

/* Private Functions Definition End */

void EEPROM_RetriveHrMeterData(uint8_t HrMeter_Sel, uint16_t HrMeter_eepromPage)
{
	uint8_t u8TempArr[6] = {0};
	uint8_t u8TmepCrc = 0;
	uint16_t u16TempHr = 0;
	HrMeter_Master[HrMeter_Sel].TotalHours = HrMeter_Master[HrMeter_Sel].TotalHours;
	HrMeter_Master[HrMeter_Sel].TotalMin = HrMeter_Master[HrMeter_Sel].TotalMin;

	uint16_t HrMeter_1stCycleDataCopy1Add = HrMeter_eepromPage + 0x00;
	uint16_t HrMeter_1stCycleDataCopy2Add = HrMeter_eepromPage + 0x04;
	uint16_t HrMeter_2ndCycleDataCopy1Add = HrMeter_eepromPage + 0x08;
	uint16_t HrMeter_2ndCycleDataCopy2Add = HrMeter_eepromPage + 0x0C;

	HrMeter_Master[HrMeter_Sel].TotalMin = 0;
	HrMeter_Master[HrMeter_Sel].TotalHours = 0;

	/*Read First Copy*/
	if(HAL_I2C_Mem_Read(&hi2c1, EEPROM_ADDRESS, HrMeter_1stCycleDataCopy1Add ,I2C_MEMADD_SIZE_16BIT,	&u8TempArr[0],4,1000) != HAL_OK)
		assertError(enmTORERRORS_MEM_I2C,enmERRORSTATE_ACTIVE);
	else
		assertError(enmTORERRORS_MEM_I2C,enmERRORSTATE_NOERROR);

	u8TmepCrc = u8TempArr[0]+u8TempArr[1]+u8TempArr[2]+1;
	if(u8TmepCrc == u8TempArr[3])
	{
		//First copy read successfully.
		u16TempHr = ((((uint16_t)u8TempArr[0]<<8)&0xFF00)|((uint16_t)u8TempArr[1]&0x00FF));
		HrMeter_Master[HrMeter_Sel].TotalHours = (uint32_t)u16TempHr;
		HrMeter_Master[HrMeter_Sel].TotalMin = (uint32_t)u8TempArr[2];
	}
	else
	{
		memset(u8TempArr,0x00,sizeof(u8TempArr));
		/*Read Second Copy*/
		if(HAL_I2C_Mem_Read(&hi2c1, EEPROM_ADDRESS, HrMeter_1stCycleDataCopy2Add ,I2C_MEMADD_SIZE_16BIT,	&u8TempArr[0],4,1000) != HAL_OK)
			assertError(enmTORERRORS_MEM_I2C,enmERRORSTATE_ACTIVE);
		else
			assertError(enmTORERRORS_MEM_I2C,enmERRORSTATE_NOERROR);

		u8TmepCrc = u8TempArr[0]+u8TempArr[1]+u8TempArr[2]+1;
		if(u8TmepCrc == u8TempArr[3])
		{
			//Second copy read successfully.
			u16TempHr = ((((uint16_t)u8TempArr[0]<<8)&0xFF00)|((uint16_t)u8TempArr[1]&0x00FF));
			HrMeter_Master[HrMeter_Sel].TotalHours = (uint32_t)u16TempHr;
			HrMeter_Master[HrMeter_Sel].TotalMin = (uint32_t)u8TempArr[2];
		}
		else
		{
			//Both Copies are corrupted
			HrMeter_Master[HrMeter_Sel].TotalHours =0;
			HrMeter_Master[HrMeter_Sel].TotalMin =0;
//			EEPROM_WriteHrMeter1Data();
		}
	}

	/*If Current Hour Value is Greater than   16000:59*/
	if(HrMeter_Master[HrMeter_Sel].TotalHours  > (uint32_t)HOUR_METER_FIRST_LOC_MAX_CNT)
	{
			HrMeter_DataWrCycle[HrMeter_Sel] = HR_METER_2ND_WR_CYCLE;
			memset(u8TempArr,0x00,sizeof(u8TempArr));
		/*Read First Copy*/
			if(HAL_I2C_Mem_Read(&hi2c1, EEPROM_ADDRESS, HrMeter_2ndCycleDataCopy1Add ,I2C_MEMADD_SIZE_16BIT,	&u8TempArr[0],4,1000) != HAL_OK)
				assertError(enmTORERRORS_MEM_I2C,enmERRORSTATE_ACTIVE);
			else
				assertError(enmTORERRORS_MEM_I2C,enmERRORSTATE_NOERROR);

			u8TmepCrc = u8TempArr[0]+u8TempArr[1]+u8TempArr[2]+1;
			if(u8TmepCrc == u8TempArr[3])
			{
				//First copy read successfully.
				u16TempHr = ((((uint16_t)u8TempArr[0]<<8)&0xFF00)|((uint16_t)u8TempArr[1]&0x00FF));
				HrMeter_Master[HrMeter_Sel].TotalHours = (uint32_t)HOUR_METER_FIRST_LOC_MAX_CNT+ (uint32_t)u16TempHr;
				HrMeter_Master[HrMeter_Sel].TotalMin = (uint32_t)u8TempArr[2];
			}
			else
			{
				memset(u8TempArr,0x00,sizeof(u8TempArr));
				/*Read Second Copy*/
				if(HAL_I2C_Mem_Read(&hi2c1, EEPROM_ADDRESS, HrMeter_2ndCycleDataCopy2Add ,I2C_MEMADD_SIZE_16BIT,	&u8TempArr[0],4,1000) != HAL_OK)
					assertError(enmTORERRORS_MEM_I2C,enmERRORSTATE_ACTIVE);
				else
					assertError(enmTORERRORS_MEM_I2C,enmERRORSTATE_NOERROR);

				u8TmepCrc = u8TempArr[0]+u8TempArr[1]+u8TempArr[2]+1;
				if(u8TmepCrc == u8TempArr[3])
				{
					//Second copy read successfully.
					u16TempHr = ((((uint16_t)u8TempArr[0]<<8)&0xFF00)|((uint16_t)u8TempArr[1]&0x00FF));
					HrMeter_Master[HrMeter_Sel].TotalHours = (uint32_t)HOUR_METER_FIRST_LOC_MAX_CNT+ (uint32_t)u16TempHr;
					HrMeter_Master[HrMeter_Sel].TotalMin = (uint32_t)u8TempArr[2];
				}
				else
				{
					//Both Copies are corrupted
					HrMeter_Master[HrMeter_Sel].TotalHours = (uint32_t)HOUR_METER_FIRST_LOC_MAX_CNT+0;
					HrMeter_Master[HrMeter_Sel].TotalMin = 0;
		//			EEPROM_WriteHrMeter1Data();
				}
			}
	}
	else
	{
		HrMeter_DataWrCycle[HrMeter_Sel] = HR_METER_1ST_WR_CYCLE;
	}
}

void EEPROM_WriteHrMeterData(uint8_t HrMeter_Sel, uint16_t HrMeter_eepromPage)
{
	uint8_t u8TempWrArra[5] = {0};
	uint16_t u16TempHr;
	uint8_t u8Tempmin = 0;
	uint16_t u16FirstCpyAdd=0,u16SecondCpyAdd = 0;

	stHourMeterTime HrMeter_Val;
	HrMeter_Val.u32TotalHours = HrMeter_Master[HrMeter_Sel].TotalHours;
	HrMeter_Val.u32TotalMin = HrMeter_Master [HrMeter_Sel].TotalMin;

	uint16_t HrMeter_1stCycleDataCopy1Add = HrMeter_eepromPage + 0x00;
	uint16_t HrMeter_1stCycleDataCopy2Add = HrMeter_eepromPage + 0x04;
	uint16_t HrMeter_2ndCycleDataCopy1Add = HrMeter_eepromPage + 0x08;
	uint16_t HrMeter_2ndCycleDataCopy2Add = HrMeter_eepromPage + 0x0C;

	if(HR_METER_1ST_WR_CYCLE  == HrMeter_DataWrCycle[HrMeter_Sel])
	{
		u16FirstCpyAdd = HrMeter_1stCycleDataCopy1Add;
		u16SecondCpyAdd = HrMeter_1stCycleDataCopy2Add;

		if(HrMeter_Val.u32TotalHours > HOUR_METER_FIRST_LOC_MAX_CNT)
		{
			// Write 16000:59 in First Region
			// And 16001:00 will be in First region
			//After That all in Second Region
			u16TempHr = HOUR_METER_FIRST_LOC_MAX_CNT+1;
			u8Tempmin = 0;
			u8TempWrArra[0] = (uint8_t)((u16TempHr&0xFF00)>>8);
			u8TempWrArra[1] = (uint8_t)((u16TempHr&0x00FF));
			u8TempWrArra[2] = u8Tempmin;
			u8TempWrArra[3] = u8TempWrArra[0]+u8TempWrArra[1]+u8TempWrArra[2]+1; //CRC

			HrMeter_DataWrCycle[HrMeter_Sel] = HR_METER_2ND_WR_CYCLE;

//			u16FirstCpyAdd = HR_METER1_2ND_CYCYLE_DATA_COPY_1_ADD;
//			u16SecondCpyAdd = HR_METER1_2ND_CYCYLE_DATA_COPY_2_ADD;
			/*Write in 1st region Only. for Next cycle it will */
		}
		else
		{

			u16TempHr = (uint16_t)HrMeter_Val.u32TotalHours;
			if(HrMeter_Val.u32TotalMin > 59)
					HrMeter_Val.u32TotalMin = 59;

			u8Tempmin = HrMeter_Val.u32TotalMin;

			u8TempWrArra[0] = (uint8_t)((u16TempHr&0xFF00)>>8);
			u8TempWrArra[1] = (uint8_t)((u16TempHr&0x00FF));
			u8TempWrArra[2] = u8Tempmin;
			u8TempWrArra[3] = u8TempWrArra[0]+u8TempWrArra[1]+u8TempWrArra[2]+1; //CRC
		}
	}
	else if(HR_METER_2ND_WR_CYCLE == HrMeter_DataWrCycle[HrMeter_Sel])
	{
		if(HrMeter_Val.u32TotalHours > HOUR_METER_FIRST_LOC_MAX_CNT)
		{
			u16TempHr =  (uint16_t)(HrMeter_Val.u32TotalHours - (uint32_t)HOUR_METER_FIRST_LOC_MAX_CNT);
		}
		else
		{
			/*This case will never happens but we added for safe purpose*/
			u16TempHr = 0;
		}

		if(HrMeter_Val.u32TotalMin > 59)
			HrMeter_Val.u32TotalMin = 59;

		u8Tempmin = HrMeter_Val.u32TotalMin;


		u8TempWrArra[0] = (uint8_t)((u16TempHr&0xFF00)>>8);
		u8TempWrArra[1] = (uint8_t)((u16TempHr&0x00FF));
		u8TempWrArra[2] = u8Tempmin;
		u8TempWrArra[3] = u8TempWrArra[0]+u8TempWrArra[1]+u8TempWrArra[2]+1; //CRC

		u16FirstCpyAdd = HrMeter_2ndCycleDataCopy1Add;
		u16SecondCpyAdd = HrMeter_2ndCycleDataCopy2Add;
	}



	HalMemWrStatus = HAL_ERROR;
	u32MemWriteCntr =0;
	do
	{
		HalMemWrStatus = HAL_I2C_Mem_Write(&hi2c1, EEPROM_ADDRESS, u16FirstCpyAdd ,I2C_MEMADD_SIZE_16BIT,&u8TempWrArra[0],4,1000);
		HAL_Delay(30);
		HAL_IWDG_Refresh(&hiwdg); //we are Retrying 5 times, it may reset device due to integral watchdog, so Refresh here
		u32MemWriteCntr++;
		if(u32MemWriteCntr>5)
			break;
	}while(HalMemWrStatus !=  HAL_OK);

	if(u32MemWriteCntr>5)
		assertError(enmTORERRORS_MEM_I2C,enmERRORSTATE_ACTIVE);
	else
		assertError(enmTORERRORS_MEM_I2C,enmERRORSTATE_NOERROR);

//	if(HAL_I2C_Mem_Write(&hi2c1, EEPROM_ADDRESS, u16FirstCpyAdd ,I2C_MEMADD_SIZE_16BIT,&u8TempWrArra[0],4,1000) != HAL_OK)
//		assertError(enmTORERRORS_MEM_I2C,enmERRORSTATE_ACTIVE);
//	else
//		assertError(enmTORERRORS_MEM_I2C,enmERRORSTATE_NOERROR);


		HalMemWrStatus = HAL_ERROR;
		u32MemWriteCntr =0;
		do
		{
			HalMemWrStatus = HAL_I2C_Mem_Write(&hi2c1, EEPROM_ADDRESS, u16SecondCpyAdd ,I2C_MEMADD_SIZE_16BIT,&u8TempWrArra[0],4,1000);
			HAL_Delay(30);
			HAL_IWDG_Refresh(&hiwdg); //we are Retrying 5 times, it may reset device due to integral watchdog, so Refresh here
			u32MemWriteCntr++;
			if(u32MemWriteCntr>5)
				break;
		}while(HalMemWrStatus !=  HAL_OK);

		if(u32MemWriteCntr>5)
			assertError(enmTORERRORS_MEM_I2C,enmERRORSTATE_ACTIVE);
		else
			assertError(enmTORERRORS_MEM_I2C,enmERRORSTATE_NOERROR);

//	if(HAL_I2C_Mem_Write(&hi2c1, EEPROM_ADDRESS, u16SecondCpyAdd ,I2C_MEMADD_SIZE_16BIT,&u8TempWrArra[0],4,1000) != HAL_OK)
//		assertError(enmTORERRORS_MEM_I2C,enmERRORSTATE_ACTIVE);
//	else
//		assertError(enmTORERRORS_MEM_I2C,enmERRORSTATE_NOERROR);

}


