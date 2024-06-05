/*
 * user_AI.c
 *
 *  Created on: Oct 19, 2023
 *      Author: abhisheks
 */

#include "user_AI.h"
#include "main.h"
#include "externs.h"
#include "user_Timer.h"

CONFIG_PinState AI_IN1Config = CONFIG_PIN_RESET;
CONFIG_PinState AI_IN2Config = CONFIG_PIN_RESET;

enmAI_ADC_State AI_ADCState = AI_ADC_READY;

uint32_t AI_RawData[5] = {0};
uint32_t AI_AdcConvCpltFlag = 0;

float AI_BattVlt = 0;
float AI_InputVlt = 0;

/****************************************************************************
 Function: AI_Routine
 Purpose: ADC data(raw) accumulation
 Input: None
 Return value: None

 Note(s)(if-any) :

 Change History:
 Author            	Date                Remarks
 100533        			29/11/2023					Initial Version
******************************************************************************/
void AI_Routine(void){

	if(AI_ADCRestartTimer){
		return;
	}

	switch (AI_ADCState) {
		case AI_ADC_READY:
			AI_AdcConvCpltFlag = 0;
			HAL_ADC_Start_DMA(&hadc1, (uint32_t*)AI_RawData, 5);
			AI_ADCState = AI_ADC_DATA;
			break;

		case AI_ADC_DATA:
			if(AI_AdcConvCpltFlag != 0){
				AI_AdcConvCpltFlag = 0;
				AI_ADCState = AI_ADC_READY;
				AI_ADCRestartTimer = FIVEHUNDRED_MS;
				AI_InputVlt = (((((((AI_RawData[AI_INPUTVLT] +80.1 ) * 3.3) / 4095)* (110/10)))));//* (308.2/8.2))))); //
				AI_BattVlt = (((((AI_RawData[AI_BATTVLT])*3.3)/4095)*3)/2);
			}
			break;

		case AI_ADC_TIMEOUT:
			break;

		default:
			AI_ADCState = AI_ADC_READY;
			break;
	}

}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc) {
	AI_AdcConvCpltFlag = 1;
	HAL_ADC_Stop_DMA(&hadc1);
}
