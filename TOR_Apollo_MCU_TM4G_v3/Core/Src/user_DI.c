/*
 * user_DI.c
 *
 *  Created on: Oct 19, 2023
 *      Author: abhisheks
 */

#include <user_config.h>
#include "user_DI.h"
#include "main.h"
#include "externs.h"
#include "user_ApplicationDefines.h"

CONFIG_PinState DI_IN1Config = CONFIG_PIN_RESET;
CONFIG_PinState DI_IN2Config = CONFIG_PIN_RESET;

uint8_t DI_IN1Flag;
uint8_t DI_IN2Flag;

void DI_Routine(void){
	if (DI_IN1Config == CONFIG_PIN_SET) {
		DI_IN1Flag = !(HAL_GPIO_ReadPin(DI_IN_1_uC_GPIO_Port, DI_IN_1_uC_Pin));
	}
	else {
		DI_IN1Flag = 0;
	}
	if (DI_IN2Config == CONFIG_PIN_SET) {
		DI_IN2Flag = !(HAL_GPIO_ReadPin(DI_IN_2_uC_GPIO_Port, DI_IN_2_uC_Pin));
	}
	else {
		DI_IN2Flag = 0;
	}
}
