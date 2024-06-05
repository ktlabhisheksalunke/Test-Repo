/*
 * user_LEDs.c
 *
 *  Created on: Apr 11, 2024
 *      Author: abhisheks
 */

#include "user_LEDs.h"
#include "main.h"
#include "externs.h"
#include "user_Timer.h"

void LED_Routine(void){
	if (LED_CommLEDDelay == 0) {
		if ((Config_MBEnable) && (Config_CANEnable)) {
			HAL_GPIO_TogglePin(COMM_LED_uC_GPIO_Port, COMM_LED_uC_Pin);
			LED_CommLEDDelay = FIVEHUNDRED_MS;
		}
		else {
			HAL_GPIO_TogglePin(COMM_LED_uC_GPIO_Port, COMM_LED_uC_Pin);
			LED_CommLEDDelay = THREE_SEC;
		}
	}
	if(GPS_Latch_Status == TRUE)
	{
		HAL_GPIO_WritePin(GPS_LED_uC_GPIO_Port, GPS_LED_uC_Pin, GPIO_PIN_SET);
	}
	else
	{
		HAL_GPIO_WritePin(GPS_LED_uC_GPIO_Port, GPS_LED_uC_Pin, GPIO_PIN_RESET);
	}
}
