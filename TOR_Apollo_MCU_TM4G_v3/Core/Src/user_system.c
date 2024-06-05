/*
 * user_system.c
 *
 *  Created on: Nov 20, 2023
 *      Author: abhisheks
 */

#include "user_system.h"

#include "main.h"
#include "externs.h"
#include "user_queue.h"
#include "payload.h"
#include "user_Accelorometer.h"
#include "user_ApplicationDefines.h"
#include "user_AI.h"
#include "user_DI.h"
#include "user_DO.h"
#include "user_can.h"
#include "user_config.h"
#include "user_DeviceInfo.h"
#include "user_HourMeter.h"
#include "user_Timer.h"
#include "user_UART.h"
#include "user_rtc.h"
#include "user_modbus_rs485.h"
#include "user_modbus_config.h"
#include "user_LEDs.h"
#include "user_RPM.h"

enmSYS_State SYS_State = SYS_Boot;

char DPS_ModuleIMEI[20];

strctQUEUE DPS_LiveQueue;
strctBuffQUEUE DPS_BuffQueue;

uint8_t DPS_RebootReq = 0;

uint8_t DPS_NTPSyncFLag = 0;;

enmModule_PWRSTATE DPS_ModulePwrState = enmModule_PWRNOTSTARTED;

#if (UART_DEBUGENABLE == 1)
uint8_t DPS_DebugSend = 0;
#endif

/****************************************************************************
 Function: SYS_FSM
 Purpose: System FSM for DPS Vitesse
 Input: None
 Return value: None

 Note(s)(if-any) :
 Boot - Once
 Init - After bootup and reconfigure request
 Routine - Device function
 Config - Entered on reconfigure request

 Change History:
 Author            	Date                Remarks
 100533        			29/11/2023					Initial Version
******************************************************************************/

void SYS_FSM(void){

	switch (SYS_State) {
		case SYS_Boot:
			updateDeviceSignature();
			Queue_InitQ(&DPS_LiveQueue);
			Queue_BuffQInit(&DPS_BuffQueue);

			Config_Get(DPS_CONFIG_AIDIDO, DPS_ConfigStr.DPS_IOConfig, DPS_MAX_CONFIG_SIZE);
			Config_Get(DPS_CONFIG_CAN, DPS_ConfigStr.DPS_CanConfig, DPS_MAX_CONFIG_SIZE);
			Config_Get(DPS_CONFIG_MB, DPS_ConfigStr.DPS_MBConfig, DPS_MAX_CONFIG_SIZE);
			Config_Get(DPS_CONFIG_HRMETER, DPS_ConfigStr.DPS_HRMConfig, DPS_MAX_CONFIG_SIZE);
			Config_Get(DPS_CONFIG_ACC, DPS_ConfigStr.DPS_AccConfig, DPS_MAX_CONFIG_SIZE);

			DPS_LoadAccCaliberation();

		  LL_USART_EnableIT_RXNE(USART1);
			SYS_State = SYS_Init;
			Config_PayloadFreqON = FIVE_SEC;
			DPS_PayloadQueueEnqueue = Config_PayloadFreqON;
			DPS_IsModuleAlive = THIRTY_SEC;

#if (UART_DEBUGENABLE == 1)
			DPS_DebugSend = 1;
#endif

			break;

		case SYS_Init:
			if(Config_ReconfigReq && (AI_InputVlt < 10)){
				SYS_State = SYS_Routine;
				return;
			}
			Config_Reset();
			Config_IOStatus = DPS_ConfigAIDIDO();
			Config_CANStatus = DPS_ConfigCAN();
			Config_MBStatus = DPS_ConfigMB();
			Config_HRMStatus = DPS_ConfigHRM();
			Config_AccStatus = DPS_ConfigACC();

		  UserAppl_FillDefaultparameters(&st_DeviceConfig);
		  if(st_DeviceConfig.u8ModbusMasterEnFlag != 0)	{
		  	setupModbus();
		  }

		  if(boolOnIntBattCANErrorFlag == FALSE)
		  {
			  CAN_Init();
			  CAN_FilterConfig();
		  }

		  RTC_SyncTimer = 10;
			DPS_IsModuleAlive = THIRTY_SEC;

			if(Config_ReconfigReq){
				Config_Save();
				Config_ReconfigReq = 0;
				break;
			}
			else{

			}

			SYS_State = SYS_Routine;
			break;

		case SYS_Routine:
			if (Config_IOEnable) {
				DI_Routine();
				DO_Routine();
			}
			AI_Routine();
			if((boolOnIntBattCANErrorFlag == TRUE) && (AI_InputVlt > 10))
			{
				boolOnIntBattCANErrorFlag = FALSE;
				CAN_Init();
				CAN_FilterConfig();
			}

			if (Config_CANEnable) {
				CAN_Routine();
			}
			if (Config_MBEnable) {
				if ((SinglewriteFlag)||(MultiwriteFlag)) {
					ModbusMasterMultiWrite_FSM();
				}
				else {
				 	 ModbusMaster_FSM();
				}
			}

			if (Config_HRMEnable) {
				HrMeter_ScanDigitalInputs();
				HrMeter_Routine();
			}

			if (Config_RPMHRMEnable) {
				RPM_IntHrMtrProcess();
			}

			UART_Routine();
			RTC_Routine();

			if ((DPS_PayloadQueueEnqueue == 0) && (Config_PayloadFreqON != 0)){
				if (DPS_IsModuleAlive) {
					if (!Queue_IsQueueFull(&DPS_LiveQueue)) {
						Queue_Enequeue(&DPS_LiveQueue,(char *)DPS_GenPayload());
#if (UART_DEBUGENABLE)
						strcpy(UART_DebugBuff,DPS_LiveQueue.data[DPS_LiveQueue.tail]);
						strcat(UART_DebugBuff,(char*)"\n");
#endif
					}
				}
				else{
					if (Queue_IsBuffQFull(&DPS_BuffQueue)) Queue_BuffQDequeue(&DPS_BuffQueue);
					Queue_BuffQEnqueue(&DPS_BuffQueue, (char *)DPS_GenPayload());
				}
				DPS_PayloadQueueEnqueue = Config_PayloadFreqON;
			}

			if ((Config_Reload) && (Queue_IsQueueEmpty(&DPS_LiveQueue))) {
				SYS_State = SYS_Init;
				Config_Reload = 0;
			}

			LED_Routine();

			G_SensorFSM();

			if (DPS_IsModuleAlive == 0) {
				SYS_State = SYS_ModulePwrCycle;
				DPS_ModulePwrState = enmModule_PWRNOTSTARTED;
			}

#if (UART_DEBUGENABLE)
			if ((UART_DebugTxTmr == 0) && (UART_DebugStrIdx == 0)) {
				if (DPS_DebugSend == 1) {
					strcpy(UART_DebugBuff,DPS_ConfigStr.DPS_IOConfig);
					strcat(UART_DebugBuff,(char*)"\n");
					DPS_DebugSend = 2;
					UART_DebugTxTmr = ONEHUNDRED_MS;
				}
				else if (DPS_DebugSend == 2) {
					strcpy(UART_DebugBuff,DPS_ConfigStr.DPS_CanConfig);
					strcat(UART_DebugBuff,(char*)"\n");
					DPS_DebugSend = 3;
					UART_DebugTxTmr = ONEHUNDRED_MS;
				}
				else if (DPS_DebugSend == 3) {
					strcpy(UART_DebugBuff,DPS_ConfigStr.DPS_MBConfig);
					strcat(UART_DebugBuff,(char*)"\n");
					DPS_DebugSend = 4;
					UART_DebugTxTmr = ONEHUNDRED_MS;
				}
				else if (DPS_DebugSend == 4) {
					strcpy(UART_DebugBuff,DPS_ConfigStr.DPS_HRMConfig);
					strcat(UART_DebugBuff,(char*)"\n");
					DPS_DebugSend = 0;
					UART_DebugTxTmr = ONEHUNDRED_MS;
				}
				else{
				}
			}
#endif

			break;

		case SYS_Config:
			Config_Routine();
			break;

		case SYS_ModulePwrCycle:
			if (DPS_PwrCycleTimer == 0) {
				if(DPS_ModulePwrState == enmModule_PWRNOTSTARTED)
				{
					HAL_GPIO_WritePin(GSM_PWR_GPIO_Port,GSM_PWR_Pin,GPIO_PIN_SET);
					DPS_PwrCycleTimer =  FOUR_SEC;
					DPS_ModulePwrState = enmModule_PWRSTARTED;
				}
				else if(DPS_ModulePwrState == enmModule_PWRSTARTED)
				{
					HAL_GPIO_WritePin(GSM_PWR_GPIO_Port,GSM_PWR_Pin,GPIO_PIN_RESET);
					DPS_PwrCycleTimer = FOUR_SEC;
					DPS_ModulePwrState = enmModule_PWRCOMPLETED;
				}
				else if(DPS_ModulePwrState == enmModule_PWRCOMPLETED)
				{
					SYS_State = SYS_Routine;
					DPS_IsModuleAlive = ONE_MIN;
				}
				else{
					SYS_State = SYS_Routine;
				}
			}
			break;

		default:
			HAL_GPIO_WritePin(GPS_LED_uC_GPIO_Port, GPS_LED_uC_Pin, GPIO_PIN_SET);
 			HAL_NVIC_SystemReset();
			break;
	}

}
