/*
 * payload.c
 *
 *  Created on: Oct 23, 2023
 *      Author: abhisheks
 */

#include "payload.h"
#include "main.h"
#include "externs.h"
#include "user_ApplicationDefines.h"
#include "string.h"
#include <stdlib.h>
#include <stdio.h>
#include "user_queue.h"
#include "user_config.h"
#include "user_can_config.h"
#include "user_can.h"
#include "user_AI.h"
#include "user_rtc.h"
#include "user_modbus_config.h"
#include "user_modbus_rs485.h"
#include "user_RPM.h"
#include "user_HourMeter.h"

uint32_t Payload_BuffCounter = 0;
#define MAXERRORPACKET 16
char cnv_temp[20] = "";

char * DPS_GenPayload(void)
{
	  char *DPS_SysPayload = (char *)malloc(sizeof(char) * DPS_PAYLOAD_MAX_SIZE);
	  char payload_temp[20]="";

	if(DPS_SysPayload  != NULL)
	{
		/*
			1.	Start of frame
			2.	Hardware ID
			3.	PAYLOAD
			4.	IO Data
			5.	CAN data
			6.	MODBUS data
		*/
		memset(DPS_SysPayload,0x00,sizeof(char) * DPS_PAYLOAD_MAX_SIZE);
		/* Start of Frame */
		strcpy((char *)DPS_SysPayload,(char * )START_OF_FRAME);
		strcat((char *)DPS_SysPayload,(char * )",");

		/* Device IMEI */
		strcat((char *)DPS_SysPayload,(char * )DPS_ModuleIMEI);


		/* ID */
		strcat((char *)DPS_SysPayload,(char * )",PAYLOAD,");

		/*Firmware Version*/
		strcat((char *)DPS_SysPayload,FIRMWARE_VER);
		strcat((char *)DPS_SysPayload,(char * )",");

		/* Date Time */
		RTC_PaylodTime(DPS_SysPayload);

		/*******************Input Supply and Batt Voltage*******************/

		sprintf(payload_temp,"%.2f,",AI_InputVlt);
		strcat((char *)DPS_SysPayload,(char * )payload_temp);
		sprintf(payload_temp,"%.2f,",AI_BattVlt);
		strcat((char *)DPS_SysPayload,(char * )payload_temp);

		/*******************IO Data Start********************/
		if (DI_IN1Config == CONFIG_PIN_SET)
		{
			itoa((int)DI_IN1Flag, cnv_temp, 10);
			strcat((char *)DPS_SysPayload,cnv_temp);
		}
		strcat((char *)DPS_SysPayload,",");

		if (DI_IN2Config == CONFIG_PIN_SET)
		{
			itoa((int)DI_IN2Flag, cnv_temp, 10);
			strcat((char *)DPS_SysPayload,cnv_temp);
		}
		strcat((char *)DPS_SysPayload,",");

		/*******************IO Data End********************/

		/*******************AI Data Start********************/
		if (AI_IN1Config == CONFIG_PIN_SET)
		{
			itoa((int)AI_RawData[AI_AI1], cnv_temp, 10);
			strcat((char *)DPS_SysPayload,cnv_temp);
		}
		strcat((char *)DPS_SysPayload,",");

		if (AI_IN2Config == CONFIG_PIN_SET)
		{
			itoa((int)AI_RawData[AI_AI2], cnv_temp, 10);
			strcat((char *)DPS_SysPayload,cnv_temp);
		}
		strcat((char *)DPS_SysPayload,",");
		/*******************AI Data End********************/

		/*******************DO Data Start********************/
		if (DO_DOConfig == CONFIG_PIN_SET)
		{
			itoa((int)DO_DOFlag, cnv_temp, 10);
			strcat((char *)DPS_SysPayload,cnv_temp);
		}
		strcat((char *)DPS_SysPayload,",");
		/*******************DO Data End********************/

		/*******************RPM Data Start********************/
		if (RPM_IN1Config == CONFIG_PIN_SET)
		{
			itoa((int)RPM_frequency, cnv_temp, 10);
			strcat((char *)DPS_SysPayload,cnv_temp);
		}
		strcat((char *)DPS_SysPayload,",");
		/*******************RPM Data End********************/

		/*******************HR Meter Start**********************/

		if (HrMeter_Master[RPM_HRM_NUM-1].En)
		{
			itoa(HrMeter_Master[RPM_HRM_NUM-1].TotalHours, cnv_temp, 10);
			strcat((char *)DPS_SysPayload,cnv_temp);
			strcat((char *)DPS_SysPayload,",");
			itoa(HrMeter_Master[RPM_HRM_NUM-1].TotalMin, cnv_temp, 10);
			strcat((char *)DPS_SysPayload,cnv_temp);
			strcat((char *)DPS_SysPayload,",");
		}
		else
		{
			strcat((char *)DPS_SysPayload,",,");
		}
		/*******************HR Meter End**********************/

		/*******************Acc. Data Start******************/
		memset(cnv_temp, 0, sizeof(cnv_temp));
		sprintf((char * )cnv_temp,"%d,",(int)g_stDeviceAccLiveData.u8VheicleInMotionFlag);
		strcat((char *)DPS_SysPayload,(char * )cnv_temp);


		memset(cnv_temp, 0, sizeof(cnv_temp));
		sprintf((char * )cnv_temp,"%d,",(int)g_stDeviceAccLiveData.u8EngineStausFlag);
		strcat((char *)DPS_SysPayload,(char * )cnv_temp);


		memset(cnv_temp, 0, sizeof(cnv_temp));
		sprintf((char * )cnv_temp,"%d,",(int)g_stDeviceAccLiveData.enumDeviceOnSlopeFlag);
		strcat((char *)DPS_SysPayload,(char * )cnv_temp);

		memset(cnv_temp, 0, sizeof(cnv_temp));
		sprintf((char * )cnv_temp,"%d,",(int)g_stDeviceAccLiveData.enumSuddenAccFlag);
		strcat((char *)DPS_SysPayload,(char * )cnv_temp);

		memset(cnv_temp, 0, sizeof(cnv_temp));
		sprintf((char * )cnv_temp,"%d,",(int)g_stDeviceAccLiveData.enumSuddenBrakingFlag);
		strcat((char *)DPS_SysPayload,(char * )cnv_temp);

		memset(cnv_temp, 0, sizeof(cnv_temp));
		sprintf((char * )cnv_temp,"%d,",(int)g_stDeviceAccLiveData.u8DeviceImpactFlag);
		strcat((char *)DPS_SysPayload,(char * )cnv_temp);

		memset(cnv_temp, 0, sizeof(cnv_temp));
		sprintf((char * )cnv_temp,"%d,",(int)g_stDeviceAccLiveData.u8DeviceTamperingDetectedFlag);
		strcat((char *)DPS_SysPayload,(char * )cnv_temp);

		/*Turn Flag logic is not Implemented*/
		memset(cnv_temp, 0, sizeof(cnv_temp));
		sprintf((char * )cnv_temp,"%d,",(int)g_stDeviceAccLiveData.enumDeviceTrunFlag);
		strcat((char *)DPS_SysPayload,(char * )cnv_temp);

		/*G SensorPayload X,Y,Z, Min, Max, and Average Values*/
		/*Axis Minimum Values*/
		memset(cnv_temp, 0, sizeof(cnv_temp));
		sprintf((char * )cnv_temp,"%d,",(int)g_stDeviceAccLiveData.i32LongAxisMinValue);
		strcat((char *)DPS_SysPayload,(char * )cnv_temp);

		memset(cnv_temp, 0, sizeof(cnv_temp));
		sprintf((char * )cnv_temp,"%d,",(int)g_stDeviceAccLiveData.i32LatAxisMinValue);
		strcat((char *)DPS_SysPayload,(char * )cnv_temp);

		memset(cnv_temp, 0, sizeof(cnv_temp));
		sprintf((char * )cnv_temp,"%d,",(int)g_stDeviceAccLiveData.i32VerticleAxisMinValue);
		strcat((char *)DPS_SysPayload,(char * )cnv_temp);

		/*Axis Maximum Values*/
		memset(cnv_temp, 0, sizeof(cnv_temp));
		sprintf((char * )cnv_temp,"%d,",(int)g_stDeviceAccLiveData.i32LongAxisMaxValue);
		strcat((char *)DPS_SysPayload,(char * )cnv_temp);

		memset(cnv_temp, 0, sizeof(cnv_temp));
		sprintf((char * )cnv_temp,"%d,",(int)g_stDeviceAccLiveData.i32LatAxisMaxValue);
		strcat((char *)DPS_SysPayload,(char * )cnv_temp);

		memset(cnv_temp, 0, sizeof(cnv_temp));
		sprintf((char * )cnv_temp,"%d,",(int)g_stDeviceAccLiveData.i32VerticleAxisMaxValue);
		strcat((char *)DPS_SysPayload,(char * )cnv_temp);


		/*Axis Average Values*/

		memset(cnv_temp, 0, sizeof(cnv_temp));
		sprintf((char * )cnv_temp,"%d,",(int)g_stDeviceAccLiveData.i32LongAxisAvgValue);
		strcat((char *)DPS_SysPayload,(char * )cnv_temp);

		memset(cnv_temp, 0, sizeof(cnv_temp));
		sprintf((char * )cnv_temp,"%d,",(int)g_stDeviceAccLiveData.i32LatAxisAvgValue);
		strcat((char *)DPS_SysPayload,(char * )cnv_temp);

		memset(cnv_temp, 0, sizeof(cnv_temp));
		sprintf((char * )cnv_temp,"%d,",(int)g_stDeviceAccLiveData.i32VerticleAxisAvgValue);
		strcat((char *)DPS_SysPayload,(char * )cnv_temp);

		/*Along the moment of axis (Long axis) tilt angle */
		memset(cnv_temp, 0, sizeof(cnv_temp));
		sprintf((char * )cnv_temp,"%f,",g_stDeviceAccLiveData.f32GradientAlongAxis);
		strcat((char *)DPS_SysPayload,(char * )cnv_temp);

		/*Around the moment of axis(Long axis) tilt angle*/
		memset(cnv_temp, 0, sizeof(cnv_temp));
		sprintf((char * )cnv_temp,"%f,",g_stDeviceAccLiveData.f32GradientAroundAxis);
		strcat((char *)DPS_SysPayload,(char * )cnv_temp);
		/*G force Value*/
		memset(cnv_temp, 0, sizeof(cnv_temp));
		sprintf((char * )cnv_temp,"%f,",g_stDeviceAccLiveData.f32Gforce);
		strcat((char *)DPS_SysPayload,(char * )cnv_temp);

		/*******************Acc. Data End******************/

		/*******************CAN Data Start****************/
		if (Config_CANEnable)
		{
			for (int canid = 1; canid <= gu32CANNoOfID; ++canid)
			{
				if ((gu32CANCommType == 1) | (gu32CANCommType == 4)) {
					updateCanPayload(canid, DPS_SysPayload, gu32CanConfigurationArray, gu64CanMessageArrayTest);
				}
				if ((gu32CANCommType == 2) | (gu32CANCommType == 5)) {
					updateCanPayloadPGN(canid, DPS_SysPayload, gu32CanConfigurationArrayPGN, gu64CanMessageIDPGN, gu64CanMessageArrayTest);
				}
			}

			if(u32MultiFrameFlag)
			{
				char temp[20] = "";

				if(st_DeviceConfig.gau32BAMType == 1)
				{
					strcat((char *)DPS_SysPayload,"^,");
					itoa(u32MultiFrameRequestId,temp,CAN_PAYLOADSTRING_RADIX);
					strcat((char *)DPS_SysPayload,temp);
					strcat((char *)DPS_SysPayload,",");

					sprintf(temp,"%02X",MultiFrameRequest[7]);
					strcat((char *)DPS_SysPayload,temp);
					sprintf(temp,"%02X",MultiFrameRequest[6]);
					strcat((char *)DPS_SysPayload,temp);
					sprintf(temp,"%02X",MultiFrameRequest[5]);
					strcat((char *)DPS_SysPayload,temp);
					sprintf(temp,"%02X",MultiFrameRequest[4]);
					strcat((char *)DPS_SysPayload,temp);
					sprintf(temp,"%02X",MultiFrameRequest[3]);
					strcat((char *)DPS_SysPayload,temp);
					sprintf(temp,"%02X",MultiFrameRequest[2]);
					strcat((char *)DPS_SysPayload,temp);
					sprintf(temp,"%02X",MultiFrameRequest[1]);
					strcat((char *)DPS_SysPayload,temp);
					sprintf(temp,"%02X",MultiFrameRequest[0]);
					strcat((char *)DPS_SysPayload,temp);
					strcat((char *)DPS_SysPayload,",");

				}
				else if (st_DeviceConfig.gau32BAMType == 2)
				{
					strcat((char *)DPS_SysPayload,"^,");
					itoa(u32MultiFrameRequestId,temp,CAN_PAYLOADSTRING_RADIX);
					strcat((char *)DPS_SysPayload,temp);
					strcat((char *)DPS_SysPayload,",");

					sprintf(temp,"%02X",MultiFrameRequest[7]);
					strcat((char *)DPS_SysPayload,temp);
					sprintf(temp,"%02X",MultiFrameRequest[6]);
					strcat((char *)DPS_SysPayload,temp);
					sprintf(temp,"%02X",MultiFrameRequest[5]);
					strcat((char *)DPS_SysPayload,temp);
					sprintf(temp,"%02X",MultiFrameRequest[4]);
					strcat((char *)DPS_SysPayload,temp);
					sprintf(temp,"%02X",MultiFrameRequest[3]);
					strcat((char *)DPS_SysPayload,temp);
					sprintf(temp,"%02X",MultiFrameRequest[2]);
					strcat((char *)DPS_SysPayload,temp);
					sprintf(temp,"%02X",MultiFrameRequest[1]);
					strcat((char *)DPS_SysPayload,temp);
					sprintf(temp,"%02X",MultiFrameRequest[0]);
					strcat((char *)DPS_SysPayload,temp);
					strcat((char *)DPS_SysPayload,",");

				}
				else
				{
					memset(MultiFrameRequest,0x00,sizeof(MultiFrameRequest));
					if(!canCSV_Enabled)
					{
						strcat((char *)DPS_SysPayload,"^,0,0");
					}
					else
					{
						strcat((char *)DPS_SysPayload,"^,0,00,00,00,00,00,00,00,00");
					}
				}

				strcat((char *)DPS_SysPayload,":,");

				updateMultiFrameCanPayload(MultiFrameCAN_ID,DPS_SysPayload);

				memset(MultiFrameCAN_ID,0x00,sizeof(MultiFrameCAN_ID));
				memset(MultiFrameRequest,0x00,sizeof(MultiFrameRequest));

				u32MultiFrameCommandId = 0;
				MultiFramecounter = 0;
				u32MultiFrameFlag = FALSE;
				NumberMultiFrame = 0;

			}//end of if(u32MultiFrameFlag)
//			else
//			{
//				memset(MultiFrameRequest,0x00,sizeof(MultiFrameRequest));
//				if(!canCSV_Enabled)
//				{
//					strcat((char *)DPS_SysPayload,"^,0,0");
//				}
//				else
//				{
//					strcat((char *)DPS_SysPayload,"^,0,00,00,00,00,00,00,00,00");
//				}
//			}

//			strcat((char *)DPS_SysPayload,":,");
//
//			updateMultiFrameCanPayload(MultiFrameCAN_ID,DPS_SysPayload);
//
//			memset(MultiFrameCAN_ID,0x00,sizeof(MultiFrameCAN_ID));
//
//			u32MultiFrameCommandId = 0;
//			MultiFramecounter = 0;
//			u32MultiFrameFlag = FALSE;
//			NumberMultiFrame = 0;

			/* CAN Data */
			memset(u32CanIdsReceived_2,0x00,sizeof(u32CanIdsReceived_2));
			memset(gu64CanMessageArrayTest,0x00,sizeof(gu64CanMessageArrayTest));
			//memset(gu64CanMessageArray_2,0x00,sizeof(gu64CanMessageArray_2));

			u32MultiFrameRequestId = 0;
			u32MultiFrameCommandId = 0;
			memset(MultiFrameRequest,0x00,sizeof(MultiFrameRequest));
			memset(MultiFrameCAN_ID,0x00,sizeof(MultiFrameCAN_ID));

		}//end of if (Config_CANEnable)
		else{
			strcat((char *)DPS_SysPayload,",");
		}
/*******************CAN Data END****************/
/*******************MODBUS Data Start**********/
		if (Config_MBEnable) {
			if(st_DeviceConfig.u8ModbusMasterEnFlag != 0)
			{
				strcat((char *)DPS_SysPayload,MODBUS_SOF);
				MB_UpdateModbusPayload();
				strcat((char *)DPS_SysPayload,(char * )gu8MBRTUPayloadString);
				strcat((char *)DPS_SysPayload,MODBUS_EOF);
			}
		}
		else{
			strcat((char *)DPS_SysPayload,",");
		}
/*******************MODBUS Data End**********/
		/* End of Frame */
		strcat((char *)DPS_SysPayload,(char * )END_OF_FRAME);

		DPS_SysPayload = (char *)realloc(DPS_SysPayload, (strlen(DPS_SysPayload)+1));

		return DPS_SysPayload;
	}
	else
		return NULL;
}


char * DPS_MBWritePayload(void)
{
	 char *DPS_MBWritePayload = (char *)malloc(sizeof(char) * DPS_PAYLOAD_MAX_SIZE);// GSM_PAYLOAD_MAX_SIZE = 1500

	if(DPS_MBWritePayload  != NULL)
	{
		/*
			1.	Start of frame
			2.	Hardware ID
			3.	Model No.
			4.	UTC date time
			5.	Firmware Version
		*/
		memset(DPS_MBWritePayload,0x00,sizeof(char) * DPS_PAYLOAD_MAX_SIZE);
		/* Start of Frame */
		strcpy((char *)DPS_MBWritePayload,(char * )START_OF_FRAME);
		strcat((char *)DPS_MBWritePayload,(char * )",");

		/* Device UUID */
		strcat((char *)DPS_MBWritePayload,(char * )DPS_ModuleIMEI);

		strcat((char *)DPS_MBWritePayload,(char * )",PAYLOAD,MWR,");

		/* Firmware Version*/
		strcat((char *)DPS_MBWritePayload,(char * )FIRMWARE_VER);
		strcat((char *)DPS_MBWritePayload,(char * )",");

		/*System Time Stamp */
		RTC_PaylodTime(DPS_MBWritePayload);

		if(MBWritePayload)
		{
			strcat((char *)DPS_MBWritePayload,TempMbPayloadBuff1);
		}

		  /* End of Frame */
		strcat((char *)DPS_MBWritePayload,(char * )END_OF_FRAME);

		DPS_MBWritePayload = (char *)realloc(DPS_MBWritePayload, (strlen(DPS_MBWritePayload)+1));

		return DPS_MBWritePayload;
	}
	else
		return NULL;
}

void updateMultiFrameCanPayload(uint8_t data[] ,char * systemPayload)
{
	char temp[20] = "";
//	uint8_t CANTempData = 0;
	uint8_t i;
	uint8_t frameCount = 0;

	for(i = 0 ; i <= (8 * MAXERRORPACKET) ;)
	{
		if(frameCount <= NumberMultiFrame )
		{
			if(i%8 == 0)
			{

				strcat((char *)systemPayload,"^,");
				itoa(u32MultiFrameCommandId,temp,CAN_PAYLOADSTRING_RADIX);
				strcat((char *)systemPayload,temp);
				strcat((char *)systemPayload,",");
				frameCount++;
			}
 			sprintf(temp,"%02X",data[i++]);
			strcat((char *)systemPayload,temp);
			sprintf(temp,"%02X",data[i++]);
			strcat((char *)systemPayload,temp);
			sprintf(temp,"%02X",data[i++]);
			strcat((char *)systemPayload,temp);
			sprintf(temp,"%02X",data[i++]);
			strcat((char *)systemPayload,temp);
			sprintf(temp,"%02X",data[i++]);
			strcat((char *)systemPayload,temp);
			sprintf(temp,"%02X",data[i++]);
			strcat((char *)systemPayload,temp);
			sprintf(temp,"%02X",data[i++]);
			strcat((char *)systemPayload,temp);
			sprintf(temp,"%02X",data[i++]);
			strcat((char *)systemPayload,temp);

		}
		else
		{
			if(!canCSV_Enabled)
			{
				strcat((char *)systemPayload,"^,0,0");
				i = i + 8;
			}
			else
			{
				strcat((char *)systemPayload,"^,0,0");
				i = i + 8;
			}
		}

		strcat((char *)systemPayload,",");
	}
	u32MultiFrameCommandId = 0;

}

char * DPS_GenDiagnostic(enmDiagnosticStatus DiagnosticStatus)
{

	char * systemPayload = (char*)malloc(sizeof(char) * DPS_DIAGNOSTIC_MAX_SIZE);
	if(systemPayload != NULL)
	{
		/*
			1.	Start of frame
			2.	Hardware ID
			3.	Model No.
			4.	UTC date time
			5.	Firmware Version
		*/
		memset(systemPayload,0x00,sizeof(char) * DPS_DIAGNOSTIC_MAX_SIZE);
		/* Start of Frame */
		strcpy((char *)systemPayload,(char * )START_OF_FRAME);
		strcat((char *)systemPayload,(char * )",");

		/* Device UUID */
		strcat((char *)systemPayload,(char * )DPS_ModuleIMEI);
		strcat((char *)systemPayload,(char * )",");

//		RTC_PaylodTime(systemPayload);

		DiagnosticString(systemPayload,DiagnosticStatus);

		/* End of Frame */
		strcat((char *)systemPayload,(char * )END_OF_FRAME);

		return systemPayload;
	}
	else
		return NULL; // malloc Error . Memory Allocation Failure
}

void DiagnosticString(char * systemPayload, enmDiagnosticStatus DiagnosticStatus)
{
//	char cnv_temp[20] = "";
	uint8_t diag_GetConfigflag = 0;
	switch(DiagnosticStatus)
	{

//		case	enmDiagnostic_HEARTBEAT:
//			strcat((char *)systemPayload,(char * )"HEARTBEAT,OK,1,");
//			break;

		case	enmDiagnostic_CONFIG_RECV_ACK:
			strcat((char *)systemPayload,(char * )Config_SelStr);
			strcat((char *)systemPayload,(char * )Config_StrID);
			memset(Config_StrID, 0, sizeof(Config_StrID));
			strcat((char *)systemPayload,(char * )",");
			break;

		case enmDiagnostic_CMD_CNFCLR:
			strcat((char *)systemPayload,(char * )"CNFCLR,OK,");
			strcat((char *)systemPayload,(char * )Config_StrID);
			memset(Config_StrID, 0, sizeof(Config_StrID));
			strcat((char *)systemPayload,(char * )",");
			break;

		case  enmDiagnostic_GET_CONFIG:
			diag_GetConfigflag = 1;

		case  enmDiagnostic_GET_IOCONFIG:
			if (Config_IOEnable) {
				char *rest =  DPS_ConfigStr.DPS_IOConfig;
				strtok_r(rest, ",", &rest);
				strtok_r(rest, ",", &rest);
				strtok_r(rest, ",", &rest);
				strtok_r(rest, ",", &rest);
				strcat((char *)systemPayload,(char * )DPS_CONFIGID_GETIO);
				strcat((char *)systemPayload,(char * )",");
				strcat(systemPayload, rest);
				strcpy(systemPayload + strlen(systemPayload) - 1, Config_StrID);
				strcat((char *)systemPayload,(char * )",");
				Config_Get(DPS_CONFIG_AIDIDO, DPS_ConfigStr.DPS_IOConfig, DPS_MAX_CONFIG_SIZE);
			}
			else{
				strcat((char *)systemPayload,(char * )DPS_CONFIGID_GETIO);
				strcat((char *)systemPayload,",NULL,");
				strcat((char *)systemPayload,Config_StrID);
				strcat((char *)systemPayload,",");
			}
			memset(Config_StrID, 0, sizeof(Config_StrID));
			if (diag_GetConfigflag == 0) {
				break;
			}

		case  enmDiagnostic_GET_CANCONFIG:
			if (Config_CANEnable) {
				char *rest =  DPS_ConfigStr.DPS_CanConfig;
				strtok_r(rest, ",", &rest);
				strtok_r(rest, ",", &rest);
				strtok_r(rest, ",", &rest);
				strtok_r(rest, ",", &rest);
				strcat((char *)systemPayload,(char * )DPS_CONFIGID_GETCAN);
				strcat((char *)systemPayload,(char * )",");
				strcat(systemPayload, rest);
				strcpy(systemPayload + strlen(systemPayload) - 1, Config_StrID);
				strcat((char *)systemPayload,(char * )",");
				Config_Get(DPS_CONFIG_CAN, DPS_ConfigStr.DPS_CanConfig, DPS_MAX_CONFIG_SIZE);
			}
			else{
				strcat((char *)systemPayload,(char * )DPS_CONFIGID_GETCAN);
				strcat((char *)systemPayload,",NULL,");
				strcat((char *)systemPayload,Config_StrID);
				strcat((char *)systemPayload,",");
			}
			memset(Config_StrID, 0, sizeof(Config_StrID));
			if (diag_GetConfigflag == 0) {
				break;
			}

		case  enmDiagnostic_GET_MBCONFIG:
			if (Config_MBEnable) {
				char *rest =  DPS_ConfigStr.DPS_MBConfig;
				strtok_r(rest, ",", &rest);
				strtok_r(rest, ",", &rest);
				strtok_r(rest, ",", &rest);
				strtok_r(rest, ",", &rest);
				strcat((char *)systemPayload,(char * )DPS_CONFIGID_GETMB);
				strcat((char *)systemPayload,(char * )",");
				strcat(systemPayload, rest);
				strcpy(systemPayload + strlen(systemPayload) - 1, Config_StrID);
				strcat((char *)systemPayload,(char * )",");
				Config_Get(DPS_CONFIG_CAN, DPS_ConfigStr.DPS_MBConfig, DPS_MAX_CONFIG_SIZE);
			}
			else{
				strcat((char *)systemPayload,(char * )DPS_CONFIGID_MB);
				strcat((char *)systemPayload,",NULL,");
				strcat((char *)systemPayload,Config_StrID);
				strcat((char *)systemPayload,",");
			}
			memset(Config_StrID, 0, sizeof(Config_StrID));
			if (diag_GetConfigflag == 0) {
				break;
			}

		case  enmDiagnostic_GET_MCU:
			strcat((char *)systemPayload,FIRMWARE_VER);
			strcat((char *)systemPayload,",");
			strcat((char *)systemPayload,dinfo);
			strcat((char *)systemPayload,",");
			strcat((char *)systemPayload,Config_StrID);
			strcat((char *)systemPayload,",");
			memset(Config_StrID, 0, sizeof(Config_StrID));
			break;

		case enmDiagnostic_IDLE:
			strcat((char *)systemPayload,"IDLE");
			strcat((char *)systemPayload,(char * )",");
			break;

		case enmDiagnostic_MB_WRITE_QUERY_SUCCESSFUL:
			strcat((char *)systemPayload,TempMbPayloadBuff1);
			strcat((char *)systemPayload,(char * )",");
			break;

		case enmDiagnostic_NOPWRSUPPLY_CNFDISC:
		{
			char* rest;
			char* str_key;
			char* cnf_id;

			char volt_temp[10]="";
			rest = DPS_TempConfig;
			strtok_r(rest, ",", &rest);																	//*
			strtok_r(rest, ",", &rest);																	//IMEI
			str_key = strtok_r(rest, ",", &rest);
			cnf_id = strtok_r(rest, ",", &rest);
			strcat((char *)systemPayload,str_key);
			strcat((char *)systemPayload,(char*)",");
			strcat((char *)systemPayload,cnf_id);
			strcat((char *)systemPayload,(char*)",NOT OK,ON BATT");

			memset(volt_temp, 0, sizeof(volt_temp));
			strcat((char *)systemPayload,(char*)",InputVlt : ");
			sprintf(volt_temp,"%.2f,",AI_InputVlt);
			strcat((char *)systemPayload,(char * )volt_temp);

			memset(volt_temp, 0, sizeof(volt_temp));
			strcat((char *)systemPayload,(char*)"Internal Batt : ");
			sprintf(volt_temp,"%.2f,",AI_BattVlt);
			strcat((char *)systemPayload,(char * )volt_temp);

			char* nexttoken = rest;
			char* strid;
			while((strstr((char*)"#*",nexttoken) == NULL) || (strstr((char*)"#",nexttoken) == NULL)){
				strid = nexttoken;
				nexttoken = strtok_r(rest, ",", &rest);
			}
			strcat(systemPayload,strid);
			strcat((char *)systemPayload,(char*)",");


////			 while ((cnf_id = strtok_r(rest, ",", &rest)) && strcmp(cnf_id, "#*") != 0 && strcmp(cnf_id, "*") != 0)
//			while ((cnf_id = strtok_r(rest, ",", &rest)) && strstr(cnf_id, "#*") == NULL && strstr(cnf_id, "*") == NULL)
//			{
//				if(strcmp(cnf_id, "#*") != 0 || strcmp(cnf_id, "*") != 0)
//				{
//				   memset(volt_temp, 0, sizeof(volt_temp));
//				   strcpy(volt_temp,cnf_id);
//				}
//
//			}
//
//			strcat((char *)systemPayload,cnf_id);
//			strcat((char *)systemPayload,",");

		}
			break;

		default:
			strcat((char *)systemPayload,"IDLE");
			strcat((char *)systemPayload,(char * )",");
			break;
	}//end of switch(DiagnosticStatus)

}//end of void DiagnosticString(char * systemPayload,enmDiagnosticStatus DiagnosticStatus)

void updateCanPayload(uint32_t data ,char * systemPayload, uint32_t * CanConfigurationArray, uint64_t * CanMessageArray)
{
//	char cnv_temp[20] = "";
	uint8_t CANTempData = 0;

	if(CanConfigurationArray[data]!=0)
	{
		strcat((char *)systemPayload,"^,0x");
		itoa(CanConfigurationArray[data],cnv_temp,CAN_PAYLOADSTRING_RADIX);
		strcat((char *)systemPayload,cnv_temp);
		strcat((char *)systemPayload,",");
	}
	else
	{
		strcat((char *)systemPayload,"^,0x00000000,");
	}


	if(CanMessageArray[data] != 0 )
	{

		CANTempData = (((uint64_t)CanMessageArray[data] >> 56) & 255);
		sprintf(cnv_temp,"%02X",CANTempData);
		CANTempData = 0;
		strcat((char *)systemPayload,cnv_temp);

		CANTempData = (((uint64_t)CanMessageArray[data] >> 48) & 255);
		sprintf(cnv_temp,"%02X",CANTempData);
		CANTempData = 0;
		strcat((char *)systemPayload,cnv_temp);

		CANTempData = (((uint64_t)CanMessageArray[data] >> 40) & 255);
		sprintf(cnv_temp,"%02X",CANTempData);
		CANTempData = 0;
		strcat((char *)systemPayload,cnv_temp);

		CANTempData = (((uint64_t)CanMessageArray[data] >> 32) & 255);
		sprintf(cnv_temp,"%02X",CANTempData);
		CANTempData = 0;
		strcat((char *)systemPayload,cnv_temp);

		CANTempData = (((uint64_t)CanMessageArray[data] >> 24) & 255);
		sprintf(cnv_temp,"%02X",CANTempData);
		CANTempData = 0;
		strcat((char *)systemPayload,cnv_temp);

		CANTempData = (((uint64_t)CanMessageArray[data] >> 16) & 255);
		sprintf(cnv_temp,"%02X",CANTempData);
		CANTempData = 0;
		strcat((char *)systemPayload,cnv_temp);

		CANTempData = (((uint64_t)CanMessageArray[data] >> 8) & 255);
		sprintf(cnv_temp,"%02X",CANTempData);
		CANTempData = 0;
		strcat((char *)systemPayload,cnv_temp);

		CANTempData = (((uint64_t)CanMessageArray[data]) & 255);
		sprintf(cnv_temp,"%02X",CANTempData);
		CANTempData = 0;
		strcat((char *)systemPayload,cnv_temp);
		strcat((char *)systemPayload,",");

		/*Clear Payload Array*/
		/* Removed after discussion with team that we should keep previous data in case of CAN communication not available */
		CanMessageArray[data] = 0;
	}
	else if(CanMessageArray[data] == 0 )
	{
		/* Added constant string in place of multiple strcat - 18/02/2021 Milind Vaze*/
//		strcat((char *)systemPayload,"00,00,00,00,00,00,00,00,");
		strcat((char *)systemPayload,"0,");

	}
}//end of void updateCanPayload(uint32_t data ,char * systemPayload )


void updateCanPayloadPGN(uint32_t data ,char * systemPayload, uint32_t * CanConfigurationArray, uint64_t * CanIDArray, uint64_t * CanMessageArray)
{
//	char cnv_temp[20] = "";
	uint8_t CANTempData = 0;

	if(CanMessageArray[data] != 0 )
	{

		strcat((char *)systemPayload,"^,0x");
		itoa(CanIDArray[data],cnv_temp,CAN_PAYLOADSTRING_RADIX);
		strcat((char *)systemPayload,cnv_temp);
		strcat((char *)systemPayload,",");

		CANTempData = (((uint64_t)CanMessageArray[data] >> 56) & 255);
		sprintf(cnv_temp,"%02X",CANTempData);
		CANTempData = 0;
		strcat((char *)systemPayload,cnv_temp);

		CANTempData = (((uint64_t)CanMessageArray[data] >> 48) & 255);
		sprintf(cnv_temp,"%02X",CANTempData);
		CANTempData = 0;
		strcat((char *)systemPayload,cnv_temp);

		CANTempData = (((uint64_t)CanMessageArray[data] >> 40) & 255);
		sprintf(cnv_temp,"%02X",CANTempData);
		CANTempData = 0;
		strcat((char *)systemPayload,cnv_temp);

		CANTempData = (((uint64_t)CanMessageArray[data] >> 32) & 255);
		sprintf(cnv_temp,"%02X",CANTempData);
		CANTempData = 0;
		strcat((char *)systemPayload,cnv_temp);

		CANTempData = (((uint64_t)CanMessageArray[data] >> 24) & 255);
		sprintf(cnv_temp,"%02X",CANTempData);
		CANTempData = 0;
		strcat((char *)systemPayload,cnv_temp);

		CANTempData = (((uint64_t)CanMessageArray[data] >> 16) & 255);
		sprintf(cnv_temp,"%02X",CANTempData);
		CANTempData = 0;
		strcat((char *)systemPayload,cnv_temp);

		CANTempData = (((uint64_t)CanMessageArray[data] >> 8) & 255);
		sprintf(cnv_temp,"%02X",CANTempData);
		CANTempData = 0;
		strcat((char *)systemPayload,cnv_temp);

		CANTempData = (((uint64_t)CanMessageArray[data]) & 255);
		sprintf(cnv_temp,"%02X",CANTempData);
		CANTempData = 0;
		strcat((char *)systemPayload,cnv_temp);
		strcat((char *)systemPayload,",");

		/*Clear Payload Array*/
		/* Removed after discussion with team that we should keep previous data in case of CAN communication not available */
		CanMessageArray[data] = 0;
	}
	else if(CanMessageArray[data] == 0 )
	{
		/* Added constant string in place of multiple strcat - 18/02/2021 Milind Vaze*/
		strcat((char *)systemPayload,"^,0,0,");
	}//

}//end of void updateCanPayload(uint32_t data ,char * systemPayload )
