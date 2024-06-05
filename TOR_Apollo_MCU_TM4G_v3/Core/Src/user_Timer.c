/*
 * user_Timer.c
 *
 *  Created on: Oct 31, 2023
 *      Author: abhisheks
 */


#include "main.h"
#include "stm32l4xx_hal.h"
#include "externs.h"
#include "user_Timer.h"

/* Captured Values */
uint32_t								uwIC2Value1 = 0;
uint32_t								uwIC2Value2 = 0;
uint32_t								uwDiffCapture = 0;
uint32_t								AvgDiff = 0;

/* Capture index */
uint16_t               uhCaptureIndex = 0;

/* Frequency Value */
uint32_t               uwFrequency_Avg = 0;
uint32_t               uwFrequency_Counter = 0;
uint32_t               uwFrequency_Start = 0;
uint32_t				FreqCounts = 0;

/* Frequency timer's overflow counter */
uint32_t 				FreqOFCount = 0;
uint32_t 				OldFreVal = 0;
uint32_t 				PrescalerTim1 = 4800;
uint32_t				ReloadVal = 408000;

uint32_t 				gu32FreqCalc = FIVE_SEC;

uint32_t gu32Channel1IsFirstEdgeCaptured = RESET;
uint32_t gu32Channel2IsFirstEdgeCaptured = RESET;

uint32_t gu32icValue1Ch1 = 0;
uint32_t gu32icValue1Ch2 = 0;
uint32_t gu32icValue2Ch1 = 0;
uint32_t gu32icValue2Ch2 = 0;

uint32_t gu32Channel1DifferenceInEdge = 0;
uint32_t gu32Channel2DifferenceInEdge = 0;

uint32_t RPMFirstReading = SET;

volatile uint32_t DPS_PayloadQueueEnqueue = 0;
volatile uint32_t DPS_HeartBeatTimer = 0;
volatile uint32_t DPS_IsModuleAlive = 0;
volatile uint32_t DPS_ModuleWkup = 0;
volatile uint32_t DPS_PwrCycleTimer = 0;

volatile uint32_t gu32CANCommandResponseTimeout = 0;
volatile uint32_t UART_RxTimer = 0;
volatile uint32_t UART_TxTimer = 0;
volatile uint32_t AI_ADCRestartTimer = 0;
volatile uint32_t RTC_SyncTimer = 0;

uint8_t RTC_SyncModuleNTP = 0;

volatile uint32_t gu32GPIODelay = 0;

volatile uint32_t LED_CommLEDDelay = 0;

volatile uint32_t UART_DebugTxTmr = 0;

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	if(htim->Instance == TIM7){
		if(DPS_HeartBeatTimer) DPS_HeartBeatTimer --;
		if(DPS_IsModuleAlive) DPS_IsModuleAlive --;
		if(DPS_PayloadQueueEnqueue) DPS_PayloadQueueEnqueue --;
		if(DPS_ModuleWkup) DPS_ModuleWkup --;
		if(DPS_PwrCycleTimer) DPS_PwrCycleTimer --;

		if(AI_ADCRestartTimer) AI_ADCRestartTimer--;
		if(UART_RxTimer) UART_RxTimer--;
		if(UART_TxTimer) UART_TxTimer--;

		if(RTC_SyncTimer) RTC_SyncTimer--;

		if(gu32CANCommandResponseTimeout) gu32CANCommandResponseTimeout--;

		if(gu32GPIODelay) gu32GPIODelay--;

		if(gu16ModbusFrameEndTimer)
		{
			gu16ModbusFrameEndTimer--;
		}
		if(gu32ModbusPollDelay)
		{
			gu32ModbusPollDelay--;
		}
		if(gu32ModbusCycelRestartTmr)
	  {
		  gu32ModbusCycelRestartTmr--;
	  }
		if(gu32ModbusResponseTimeout)
		{
			gu32ModbusResponseTimeout--;
		}

		if(UART_DebugTxTmr) {
			UART_DebugTxTmr--;
		}

		if (LED_CommLEDDelay) {
			LED_CommLEDDelay--;
		}

		/*G SENSOR*/
		if(gu32SensorVheInMotionTmr){gu32SensorVheInMotionTmr--;}
		if(gu32SensorFsmMotionTmr){gu32SensorFsmMotionTmr--;}

		for (int hrmeter_num = 0; hrmeter_num < HOURMETER_COUNT; ++hrmeter_num) {
			if(HrMeter_Master[hrmeter_num].RunningStatus)
			{
				HrMeter_Master[hrmeter_num].MsCntr++;
				if(HrMeter_Master[hrmeter_num].MsCntr >= ONE_SEC)
				{
					HrMeter_Master[hrmeter_num].MsCntr = 0;
					HrMeter_Master[hrmeter_num].SecCntr++;

					if(HrMeter_Master[hrmeter_num].SecCntr >= (ONE_MIN/ONE_SEC))
					{
						HrMeter_Master[hrmeter_num].SecCntr = 0;
						HrMeter_Master[hrmeter_num].MinInFlag++;
					}
				}
			}
		}

		for(int i=1 ;i <= gau32TxConfigCANIDs_Count ;i++)
		{
			if(requestedCANID_Time[i].value)
			{
				requestedCANID_Time[i].value--;
			}
		}
	}

	if(htim->Instance == TIM1)
	{
		if(FreqOFCount < 100)
		{
			FreqOFCount++;
		}
		else
		{
			FreqOFCount = 0;
			uwFrequency_Counter	= 0;
			RPM_frequency = 0;
		}
		/* If timer 1 overflows, there is no valid frequency so make RPM 0 */

	}
}


/******************************************************************************
 Function Name: HAL_TIM_IC_CaptureCallback
 Purpose: Input Capture Interrupt
 Input:	TIM_HandleTypeDef timer instance
 Return value: None.

 Change History:
 Author           	Date                Remarks
 System Generated  	9-02-19
 100138			   	26-02-19			Falling / Ceiling Logic for RPM
 100138			   	27-02-19			Falling / Ceiling Logic for RPM with 80 base
 100138				28-02-19			Changed falling - ceiling in 2 parts, 0-50
 	 	 	 	 	 	 	 	 	 	and 51-100 as required by Manitou
******************************************************************************/
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
	if((htim->Instance == TIM1))
	{
		if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1)
		{
			/* If fist edge captured, get data for current timer*/
			if(gu32Channel2IsFirstEdgeCaptured == RESET)
			{
				gu32icValue1Ch2 = HAL_TIM_ReadCapturedValue(htim,TIM_CHANNEL_1);
				gu32Channel2IsFirstEdgeCaptured = SET;
			}
			/* else if second edge is captured, Calculate frequency */
			else if(gu32Channel2IsFirstEdgeCaptured == SET)
			{
				/* Disable IT */
				if(HAL_TIM_IC_Stop_IT(&htim1, TIM_CHANNEL_1) != HAL_OK)
				{
					/* Starting Error */
					Error_Handler();
				}

				gu32icValue2Ch2 = HAL_TIM_ReadCapturedValue(htim,TIM_CHANNEL_1);
				if(gu32icValue2Ch2 > gu32icValue1Ch2)
				{
					gu32Channel2DifferenceInEdge = (gu32icValue2Ch2 - gu32icValue1Ch2) + (FreqOFCount * 65536);
				}
				else if(gu32icValue2Ch2 < gu32icValue1Ch2)
				{
					gu32Channel2DifferenceInEdge = (((0xFFFF - gu32icValue1Ch2) + gu32icValue2Ch2) + 1) + (FreqOFCount * 65536);
				}
				else
				{
					/* Unexpected Interrupt */
					/* Manage an error for robust application */
				}
				uwFrequency_Counter = ((HAL_RCC_GetPCLK1Freq() / (htim->Instance->PSC + 1)) / gu32Channel2DifferenceInEdge);

//				uwFrequency_Counter =  (4.2043 * uwFrequency_Counter) - 48.281;

//				uwFrequency_Counter =  (-0.0024 * uwFrequency_Counter * uwFrequency_Counter ) +(5.8022 * uwFrequency_Counter)- 299.8;


				if(uwFrequency_Counter <= 3000 )
				{

					/* If first reading then take it as it is */
					if(RPMFirstReading != SET)
					{
//						uwFrequency_Counter = (uwFrequency_Counter * FilterCoeff) + (RPM_frequency * (1 - FilterCoeff));
					}
					else
					{
						RPMFirstReading = RESET;
					}

					RPM_frequency = uwFrequency_Counter;
				}

				uwFrequency_Counter = 0;
				FreqOFCount = 0;
				gu32Channel2IsFirstEdgeCaptured = RESET;

				__HAL_TIM_SET_COUNTER(htim, 0);

				/* Enable IT */
				if(HAL_TIM_IC_Start_IT(&htim1, TIM_CHANNEL_1) != HAL_OK)
				{
					/* Starting Error */
					Error_Handler();
				}

			}
		}
	}
}
