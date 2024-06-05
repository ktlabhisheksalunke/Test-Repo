/*
 * user_DO.c
 *
 *  Created on: Oct 19, 2023
 *      Author: abhisheks
 */

#include <user_config.h>
#include "user_DO.h"
#include "main.h"
#include "externs.h"
#include "user_ApplicationDefines.h"

CONFIG_PinState DO_DOConfig = CONFIG_PIN_RESET;

uint8_t DO_DOFlag;

void DO_Routine(void){
	if (DO_DOConfig) {
		HAL_GPIO_WritePin(DO_uC_GPIO_Port, DO_uC_Pin, CONFIG_PIN_SET);
		DO_DOFlag = 1;
	}
	else{
		HAL_GPIO_WritePin(DO_uC_GPIO_Port, DO_uC_Pin, CONFIG_PIN_RESET);
		DO_DOFlag = 0;
	}
	return;
}
