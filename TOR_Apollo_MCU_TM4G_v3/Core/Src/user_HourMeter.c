/*
 * user_HourMeter..c
 *
 *  Created on: Feb 29, 2024
 *      Author: abhisheks
 */


#include "main.h"
#include "stm32l4xx_hal.h"
#include "externs.h"
#include <string.h>
#include <stdlib.h>
#include "user_rtc.h"
#include "user_eeprom.h"
#include "user_HourMeter.h"


HrMeter_MStruct HrMeter_Master[HOURMETER_COUNT];


uint8_t HrMeter_En = 0;
uint16_t HrMeter_DataWrCycle[HOURMETER_COUNT];
uint32_t gu8ScanStatus = 0;

GPIO_TypeDef* HrMeter_IOPort[HOURMETER_COUNT] = {
		DI_IN_1_uC_GPIO_Port,
		DI_IN_2_uC_GPIO_Port,
		RPM_IN1_GPIO_Port
};

uint16_t HrMeter_IOPin[HOURMETER_COUNT] = {
		DI_IN_1_uC_Pin,
		DI_IN_2_uC_Pin,
		RPM_IN1_Pin
};

uint16_t HrMeter_EEPROMloc[HOURMETER_COUNT] = {
		EEPROM_PAGE3_ADD,
		EEPROM_PAGE4_ADD,
		EEPROM_PAGE5_ADD
};

void HrMeter_ScanDigitalInputs(void)
{
	if(gu8ScanStatus == 0 && (gu32GPIODelay == 0))
	{
		for (int hrmeter_num = 0; hrmeter_num < HOURMETER_NUM_DI; ++hrmeter_num) {
			if (!HrMeter_Master[hrmeter_num].En) continue;
			HrMeter_Master[hrmeter_num].PinState = HAL_GPIO_ReadPin(HrMeter_Master[hrmeter_num].GPIOPort, HrMeter_Master[hrmeter_num].GPIOPin);
		}
		gu32GPIODelay = ONE_SEC;
		gu8ScanStatus = 1;
	}

	else if((gu8ScanStatus == 1) && (gu32GPIODelay == 0))
	{
		for (int hrmeter_num = 0; hrmeter_num < HOURMETER_NUM_DI; ++hrmeter_num) {
			if (!HrMeter_Master[hrmeter_num].En) continue;
			if(HAL_GPIO_ReadPin(HrMeter_Master[hrmeter_num].GPIOPort, HrMeter_Master[hrmeter_num].GPIOPin) == HrMeter_Master[hrmeter_num].PinState){
				HrMeter_Master[hrmeter_num].Pin = (HAL_GPIO_ReadPin(HrMeter_Master[hrmeter_num].GPIOPort, HrMeter_Master[hrmeter_num].GPIOPin) ^ 0x1);
			}
		}

		gu8ScanStatus = 0;
		gu32GPIODelay = ONE_SEC;
	}
}

void HrMeter_Routine(void)
{
	static uint32_t HrMeter_OldSecCntr[HOURMETER_COUNT] = {0};

	for (int hrmeter_num = 0; hrmeter_num < HOURMETER_COUNT; ++hrmeter_num) {
		if (!HrMeter_Master[hrmeter_num].En) continue;
		HrMeter_Master[hrmeter_num].RunningStatus  = HrMeter_Master[hrmeter_num].Pin;
		if(HrMeter_Master[hrmeter_num].MinInFlag)
		{
			HrMeter_Master[hrmeter_num].TotalMin += HrMeter_Master[hrmeter_num].MinInFlag;
			if(HrMeter_Master[hrmeter_num].TotalMin >= 60)
			{
				HrMeter_Master[hrmeter_num].TotalMin -= 60;
				HrMeter_Master[hrmeter_num].TotalHours++;
			}
			//Save the Hr Meter Data.
			HrMeter_Master[hrmeter_num].MinInFlag =0;
			EEPROM_WriteHrMeterData(hrmeter_num, HrMeter_Master[hrmeter_num].EEPROMloc);
		}

		/*Update Second in Backup Reg*/
		if(HrMeter_OldSecCntr[hrmeter_num] != HrMeter_Master[hrmeter_num].SecCntr)
		{
			HrMeter_OldSecCntr[hrmeter_num] = HrMeter_Master[hrmeter_num].SecCntr;
			if(HrMeter_OldSecCntr[hrmeter_num] > 60) HrMeter_OldSecCntr[hrmeter_num] = 60;
			RTC_BackupHRMSec(hrmeter_num,(uint8_t)HrMeter_Master[hrmeter_num].SecCntr);
		}
	}
}

