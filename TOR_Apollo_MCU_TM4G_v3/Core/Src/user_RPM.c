/*
 * user_RPM.c
 *
 *  Created on: Apr 16, 2024
 *      Author: abhisheks
 */

#include "user_RPM.h"
#include "user_DI.h"
#include "main.h"
#include "externs.h"

CONFIG_PinState RPM_IN1Config = CONFIG_PIN_RESET;

volatile uint8_t RPM_HrMeterStart = 0;
volatile uint32_t RPM_frequency = 0;
uint32_t RPM_ThresholdFreq = 0;

void RPM_IntHrMtrProcess(void)
{
 	if(RPM_HrMeterStart == 0)
	{
		if((RPM_frequency >= RPM_ThresholdFreq) && (RPM_frequency != 0))
		{
			RPM_HrMeterStart = 1; //Start the Hour meter Counting.
		}
		else if(RPM_frequency == 0)
		{
			if(HAL_GPIO_ReadPin(RPM_IN1_GPIO_Port, RPM_IN1_Pin) == RPM_HRM_TRIGGER_PIN_STATE)
			{
				if(HAL_GPIO_ReadPin(RPM_IN1_GPIO_Port, RPM_IN1_Pin) == RPM_HRM_TRIGGER_PIN_STATE)
				{
					RPM_HrMeterStart = 1; //Start the Hour meter Counting.
				}
			}
		}
	}
	else
	{
		if((HAL_GPIO_ReadPin(RPM_IN1_GPIO_Port, RPM_IN1_Pin) != RPM_HRM_TRIGGER_PIN_STATE) && (RPM_frequency == 0))
		{
			if(HAL_GPIO_ReadPin(RPM_IN1_GPIO_Port, RPM_IN1_Pin) != RPM_HRM_TRIGGER_PIN_STATE)
			{
				RPM_HrMeterStart = 0; //Stop the Hour meter Counting.
			}
		}
	}

 	if (RPM_frequency >= RPM_ThresholdFreq) {
 		HrMeter_Master[RPM_HRM_NUM-1].Pin = RPM_HrMeterStart;
	}
 	else {
 		HrMeter_Master[RPM_HRM_NUM-1].Pin = 0;
	}

}
