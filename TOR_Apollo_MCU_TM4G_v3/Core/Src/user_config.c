/*
 * config.c
 *
 *  Created on: Nov 2, 2023
 *      Author: abhisheks
 */

#include "user_config.h"
#include "user_can_config.h"
#include "user_modbus_config.h"
#include "user_modbus_rs485.h"
#include "user_rtc.h"
#include "externs.h"
#include "payload.h"
#include "user_Timer.h"
#include "user_ApplicationDefines.h"
#include "user_AI.h"

strctDPSConfig DPS_ConfigStr __attribute__((section(".sram2")));;
char 								DPS_TempConfig	[DPS_MAX_CONFIG_SIZE] = "";
char 								config_temp	[DPS_MAX_CONFIG_SIZE] __attribute__((section(".sram2")));;
char 								Config_SelStr[64] = "";
enmConfigOperation	Config_OprSel = 0;
uint8_t							Config_ReconfigReq = 0;
uint8_t							Config_EraseFlag = 0;

uint8_t						Config_IORequest;
uint8_t						Config_CanRequest;
uint8_t						Config_MBRequest;
uint8_t						Config_HRMRequest;
uint8_t						Config_AccRequest;

uint8_t						Config_SetGet;
char							Config_StrID [10];

HAL_StatusTypeDef Config_IOStatus;
HAL_StatusTypeDef Config_CANStatus;
HAL_StatusTypeDef Config_MBStatus;
HAL_StatusTypeDef Config_HRMStatus;
HAL_StatusTypeDef Config_AccStatus;

uint8_t Config_IOConfigErr = 0;
uint8_t Config_HRMConfigErr = 0;
uint8_t Config_AccConfigErr = 0;

uint8_t Config_CANEnable = 0;
uint8_t Config_IOEnable = 0;
uint8_t Config_HRMEnable = 0;
uint8_t Config_MBEnable = 0;

uint8_t Config_RPMHRMEnable = 0;
uint8_t Config_AccEnable = 0;

_Bool WriteQueryReceivedflag = FALSE;
uint8_t String_length_counter = 0;

uint32_t gu32ConfigModbus485ErrorDatabase = 0;
uint8_t MultiwriteFlag = 0;
uint8_t SinglewriteFlag = 0;
uint8_t Stringlencorrectflag = 0;

uint32_t Config_PayloadFreqON = 0;
uint32_t Config_PayloadFreqOFF = 0;
uint32_t Config_PayloadFreqIDLE = 0;
uint32_t GPS_Latch_Status = 0;
_Bool boolGSMPeriodicRestartFalg = 0;


uint8_t Config_Reload = 0;
uint8_t Config_MBWriteCmd = 0;

static void verifyModbus485ConfigData(void);

/****************************************************************************
 Function: Config_Routine
 Purpose: Configuration Routine
 Input: None
 Return value: None

 Note(s)(if-any) :
Can take one or more than one configuration strings.
Seperates the strings and writes each one into the respective configuration string.

 Change History:
 Author            	Date                Remarks
 100533        			29/11/2023					Initial Version
******************************************************************************/

void Config_Routine(void){

	if (strstr((char*)DPS_TempConfig,DPS_ModuleIMEI) != NULL) {					//tbr

		if (AI_InputVlt <= 10) {
			if (!Queue_IsQueueFull(&DPS_LiveQueue)) Queue_Enequeue(&DPS_LiveQueue,(char *)DPS_GenDiagnostic(enmDiagnostic_NOPWRSUPPLY_CNFDISC));
			SYS_State = SYS_Routine;
			return;
		}

		char* tempidx = strstr(DPS_TempConfig,(char*)"#");
		memset(tempidx+1,0x0,1);

		Config_IORequest = 0;
		Config_CanRequest = 0;
		Config_MBRequest = 0;
		Config_SetGet = 0;
		Config_OprSel = 0;
		Config_MBWriteCmd = 0;
		char *strid;
		char *nexttoken;
		size_t len;

		strcpy(config_temp,DPS_TempConfig);
		char *token;
		token = strtok((char*)config_temp, ",");						// SOF
		token = strtok(NULL, ",");													// IMEI
		token = strtok(NULL, ",");													// SET/GET

		if(strstr((char*)DPS_CONFIG_SET,token) != NULL){
			SYS_State = SYS_Init;
			token = strtok(NULL, ",");													// CONFIG
			if 			(strstr((char*)token,DPS_CONFIGID_IO) != NULL) Config_OprSel = enmCONFIG_SETIO;
			else if (strstr((char*)token,DPS_CONFIGID_CAN) != NULL) Config_OprSel = enmCONFIG_SETCAN;
			else if (strstr((char*)token,DPS_CONFIGID_MB) != NULL) Config_OprSel = enmCONFIG_SETMB;
			else if (strstr((char*)token,DPS_CONFIGID_HRM) != NULL) Config_OprSel = enmCONFIG_SETHRM;
			else if (strstr((char*)token,DPS_CONFIGID_ACC) != NULL) Config_OprSel = enmCONFIG_SETACC;
		}
		else if(strstr((char*)DPS_CONFIG_GET,token) != NULL){
			SYS_State = SYS_Routine;
			token = strtok(NULL, ",");													// CONFIG
			if 			(strstr((char*)token,DPS_CONFIGID_IO) != NULL) Config_OprSel = enmCONFIG_GETIO;
			else if (strstr((char*)token,DPS_CONFIGID_CAN) != NULL) Config_OprSel = enmCONFIG_GETCAN;
			else if (strstr((char*)token,DPS_CONFIGID_MB) != NULL) Config_OprSel = enmCONFIG_GETMB;
			else if (strstr((char*)token,DPS_CONFIGID_HRM) != NULL) Config_OprSel = enmCONFIG_GETHRM;
			else if (strstr((char*)token,DPS_CONFIGID_MCU) != NULL) Config_OprSel = enmCONFIG_GETCNF;
		}
		else if(strstr((char*)DPS_CONFIG_CMD,token) != NULL){
			SYS_State = SYS_Init;
			token = strtok(NULL, ",");													// CONFIG
			if 			(strstr((char*)token,DPS_CONFIGID_CLR) != NULL) Config_OprSel = enmCONFIG_CMDCLEAR;
			else if	(strstr((char*)token,DPS_CONFIGID_RST) != NULL) Config_OprSel = enmCONFIG_CMDRST;
			else if	(strstr((char*)token,DPS_CONFIGID_ACC) != NULL) Config_OprSel = enmCONFIG_CMDACCCALIB;
		}
		else{
			token = strtok(NULL, ",");													// CONFIG
			Config_OprSel = enmCONFIG_NONE;
			SYS_State = SYS_Routine;
		}

		nexttoken = token;
		while((strstr((char*)"#*",nexttoken) == NULL) || (strstr((char*)"#",nexttoken) == NULL)){
			strid = nexttoken;
			nexttoken = strtok(NULL, ",");
		}
		strcpy(Config_StrID,strid);
		len = strid - config_temp;

		switch (Config_OprSel) {
			case enmCONFIG_SETIO:
				memset(DPS_ConfigStr.DPS_IOConfig,0,DPS_MAX_CONFIG_SIZE);
				strlcpy(DPS_ConfigStr.DPS_IOConfig,DPS_TempConfig,len);
				strcat(DPS_ConfigStr.DPS_IOConfig,",#");
				Config_IORequest = 1;
				break;

			case enmCONFIG_SETCAN:
				memset(DPS_ConfigStr.DPS_CanConfig,0,DPS_MAX_CONFIG_SIZE);
				strlcpy(DPS_ConfigStr.DPS_CanConfig,DPS_TempConfig,len);
				strcat(DPS_ConfigStr.DPS_CanConfig,",#");
				Config_CanRequest = 1;
				break;

			case enmCONFIG_SETHRM:
				memset(DPS_ConfigStr.DPS_HRMConfig,0,DPS_MAX_CONFIG_SIZE);
				strlcpy(DPS_ConfigStr.DPS_HRMConfig,DPS_TempConfig,len);
				strcat(DPS_ConfigStr.DPS_HRMConfig,",#");
				Config_HRMRequest = 1;
				break;

			case enmCONFIG_SETMB:
				memset(DPS_ConfigStr.DPS_MBConfig,0,DPS_MAX_CONFIG_SIZE);
				strlcpy(DPS_ConfigStr.DPS_MBConfig,DPS_TempConfig,len);
				strcat(DPS_ConfigStr.DPS_MBConfig,",#");
				if ((strstr((char*)DPS_ConfigStr.DPS_MBConfig,(char*)DPS_MW_MB) != NULL) || (strstr((char*)DPS_ConfigStr.DPS_MBConfig,(char*)DPS_SW_MB) != NULL)) {
					Config_MBWriteCmd = 1;
				}
				else{
					Config_MBWriteCmd = 0;
				}
				Config_MBRequest = 1;
				break;

			case enmCONFIG_SETACC:
				memset(DPS_ConfigStr.DPS_AccConfig,0,DPS_MAX_CONFIG_SIZE);
				strlcpy(DPS_ConfigStr.DPS_AccConfig,DPS_TempConfig,len);
				strcat(DPS_ConfigStr.DPS_AccConfig,",#");
				Config_AccRequest = 1;
				break;

			case enmCONFIG_GETIO:
				if (!Queue_IsQueueFull(&DPS_LiveQueue)) Queue_Enequeue(&DPS_LiveQueue,(char *)DPS_GenDiagnostic(enmDiagnostic_GET_IOCONFIG));
				break;

			case enmCONFIG_GETCAN:
				if (!Queue_IsQueueFull(&DPS_LiveQueue)) Queue_Enequeue(&DPS_LiveQueue,(char *)DPS_GenDiagnostic(enmDiagnostic_GET_CANCONFIG));
				break;

			case enmCONFIG_GETMB:
				if (!Queue_IsQueueFull(&DPS_LiveQueue)) Queue_Enequeue(&DPS_LiveQueue,(char *)DPS_GenDiagnostic(enmDiagnostic_GET_MBCONFIG));
				break;

			case enmCONFIG_GETCNF:
				if (!Queue_IsQueueFull(&DPS_LiveQueue)) Queue_Enequeue(&DPS_LiveQueue,(char *)DPS_GenDiagnostic(enmDiagnostic_GET_CONFIG));
				break;

			case enmCONFIG_CMDCLEAR:
				memset(&DPS_ConfigStr,0x00,sizeof(DPS_ConfigStr));
				Config_EraseFlag = 1;
				uint8_t config_clearstatus = Config_Write(NULL, DPS_CONFIG_STARTPAGE, 0);
				if (config_clearstatus == HAL_OK) {
					if (!Queue_IsQueueFull(&DPS_LiveQueue)) Queue_Enequeue(&DPS_LiveQueue,(char *)DPS_GenDiagnostic(enmDiagnostic_CMD_CNFCLR));
				}
				break;

			case enmCONFIG_CMDACCCALIB:
			{
				char* rest;
				rest = DPS_TempConfig;
				strtok_r(rest, ",", &rest);
				strtok_r(rest, ",", &rest);
				strtok_r(rest, ",", &rest);
				strtok_r(rest, ",", &rest);
				g_u8AccCalibratemode = atoi(strtok_r(rest, ",", &rest));
				g_u8GSensCalibState = G_SESN_CALIB_START;
				break;
			}
			case enmCONFIG_CMDRST:
				HAL_NVIC_SystemReset();
				break;

			default:
				break;
		}

		memset(config_temp,0,DPS_MAX_CONFIG_SIZE);
	}
	else{
		SYS_State = SYS_Routine;
	}

	memset(DPS_TempConfig,0,DPS_MAX_CONFIG_SIZE);

}

/****************************************************************************
 Function: Config_Save
 Purpose: Save or revert recieved configuration
 Input: None
 Return value: None

 Note(s)(if-any) :

 Change History:
 Author            	Date                Remarks
 100533        			29/11/2023					Initial Version
******************************************************************************/

void Config_Save(void){
	/*
	 * 	IO Configuration
	 */
	if (Config_IORequest == 1) {
		strcpy((char *)Config_SelStr,(char * )DPS_CONFIGID_IO);
		strcat((char *)Config_SelStr,(char * )",");
		if ((Config_IOStatus == HAL_OK) && (AI_InputVlt > 10)) {
			Config_Write(DPS_ConfigStr.DPS_IOConfig, DPS_CONFIG_AIDIDO_PAGE, DPS_MAX_CONFIG_SIZE);
			strcat((char *)Config_SelStr,(char * )"OK,");
		}
		else {
			Config_Get(DPS_CONFIG_AIDIDO, DPS_ConfigStr.DPS_IOConfig, DPS_MAX_CONFIG_SIZE);
			strcat((char *)Config_SelStr,(char * )"NOT OK,");
			SYS_State = SYS_Init;
		}
//		itoa(Config_IOConfigErr, config_code, 10);
//		strcat((char *)Config_SelStr,(char * )config_code);
//		strcat((char *)Config_SelStr,(char * )",");
		if (!Queue_IsQueueFull(&DPS_LiveQueue)) Queue_Enequeue(&DPS_LiveQueue,(char *)DPS_GenDiagnostic(enmDiagnostic_CONFIG_RECV_ACK));
		Config_IORequest = 0;
	}

	/*
	 * 	CAN Configuration
	 */
	if (Config_CanRequest == 1) {
		strcpy((char *)Config_SelStr,(char * )DPS_CONFIGID_CAN);
		strcat((char *)Config_SelStr,(char * )",");
		if ((Config_CANStatus == HAL_OK) && (AI_InputVlt > 10)) {
			Config_Write(DPS_ConfigStr.DPS_CanConfig, DPS_CONFIG_CAN_PAGE, DPS_MAX_CONFIG_SIZE);
			strcat((char *)Config_SelStr,(char * )"OK,");
		}
		else {
			Config_Get(DPS_CONFIG_CAN, DPS_ConfigStr.DPS_CanConfig, DPS_MAX_CONFIG_SIZE);
			strcat((char *)Config_SelStr,(char * )"NOT OK,");
			SYS_State = SYS_Init;
		}
//		itoa(Config_CANConfigErr, config_code, 10);
//		strcat((char *)Config_SelStr,(char * )config_code);
//		strcat((char *)Config_SelStr,(char * )",");
		if (!Queue_IsQueueFull(&DPS_LiveQueue)) Queue_Enequeue(&DPS_LiveQueue,(char *)DPS_GenDiagnostic(enmDiagnostic_CONFIG_RECV_ACK));
		Config_CanRequest = 0;
	}

	/*
	 * 	Modbus Configuration
	 */
	if(Config_MBRequest == 1)  {
		strcpy((char *)Config_SelStr,(char * )DPS_CONFIGID_MB);
		strcat((char *)Config_SelStr,(char * )",");
		if ((Config_MBStatus == HAL_OK) && (AI_InputVlt > 10)) {
			if (!Config_MBWriteCmd) {
				Config_Write(DPS_ConfigStr.DPS_MBConfig, DPS_CONFIG_MB_PAGE, DPS_MAX_CONFIG_SIZE);
			}
			strcat((char *)Config_SelStr,(char * )"OK,");
		}
		else{
			Config_Get(DPS_CONFIG_MB, DPS_ConfigStr.DPS_MBConfig, DPS_MAX_CONFIG_SIZE);
			strcat((char *)Config_SelStr,(char * )"NOT OK,");
			SYS_State = SYS_Init;
		}
		if (!Queue_IsQueueFull(&DPS_LiveQueue)) Queue_Enequeue(&DPS_LiveQueue,(char *)DPS_GenDiagnostic(enmDiagnostic_CONFIG_RECV_ACK));
		Config_MBRequest = 0;
	}

	/*
	 * 	HRM Configuration
	 */
	if(Config_HRMRequest == 1)  {
		strcpy((char *)Config_SelStr,(char * )DPS_CONFIGID_HRM);
		strcat((char *)Config_SelStr,(char * )",");
		if ((Config_HRMStatus == HAL_OK) && (AI_InputVlt > 10)) {
			Config_Write(DPS_ConfigStr.DPS_HRMConfig, DPS_CONFIG_HRMETER_PAGE, DPS_MAX_CONFIG_SIZE);
			strcat((char *)Config_SelStr,(char * )"OK,");
		}
		else{
			Config_Get(DPS_CONFIG_HRMETER, DPS_ConfigStr.DPS_HRMConfig, DPS_MAX_CONFIG_SIZE);
			strcat((char *)Config_SelStr,(char * )"NOT OK,");
			SYS_State = SYS_Init;
		}
		if (!Queue_IsQueueFull(&DPS_LiveQueue)) Queue_Enequeue(&DPS_LiveQueue,(char *)DPS_GenDiagnostic(enmDiagnostic_CONFIG_RECV_ACK));
		Config_HRMRequest = 0;
	}

	/*
	 * 	ACC Configuration
	 */
	if(Config_AccRequest == 1)  {
		strcpy((char *)Config_SelStr,(char * )DPS_CONFIGID_ACC);
		strcat((char *)Config_SelStr,(char * )",");
		if ((Config_AccStatus == HAL_OK) && (AI_InputVlt > 10)) {
			Config_Write(DPS_ConfigStr.DPS_AccConfig, DPS_CONFIG_ACC_PAGE, DPS_MAX_CONFIG_SIZE);
			strcat((char *)Config_SelStr,(char * )"OK,");
		}
		else{
			Config_Get(DPS_CONFIG_ACC, DPS_ConfigStr.DPS_AccConfig, DPS_MAX_CONFIG_SIZE);
			strcat((char *)Config_SelStr,(char * )"NOT OK,");
			SYS_State = SYS_Init;
		}
		if (!Queue_IsQueueFull(&DPS_LiveQueue)) Queue_Enequeue(&DPS_LiveQueue,(char *)DPS_GenDiagnostic(enmDiagnostic_CONFIG_RECV_ACK));
		Config_AccRequest = 0;
	}

}

/****************************************************************************
 Function: Config_Reset
 Purpose: Configuration Reset
 Input: None
 Return value: None

 Note(s)(if-any) :
Resets the exisiting configurations for all the peripherals.

 Change History:
 Author            	Date                Remarks
 100533        			29/11/2023					Initial Version
******************************************************************************/
void Config_Reset(void){
	Config_IOEnable = 0;
	Config_CANEnable = 0;
	Config_MBEnable = 0;
	AI_IN1Config = CONFIG_PIN_RESET;
	AI_IN2Config = CONFIG_PIN_RESET;
	DI_IN1Config = CONFIG_PIN_RESET;
	DI_IN2Config = CONFIG_PIN_RESET;
	DO_DOConfig  = CONFIG_PIN_RESET;
	memset(gu32CanConfigurationArray,0,4*(MAX_CAN_IDS_SUPPORTED + 1));
	memset(gu32CanConfigurationArrayPGN,0,4*(MAX_CAN_IDS_SUPPORTED + 1));
	memset(requestedCANIDs_Tx,0,(sizeof(RequestedCANID_t))*(MAX_CAN_IDS_SUPPORTED + 1));
}

/****************************************************************************
 Function: Config_Get
 Purpose: Read saved configuration
 Input:
 DPS_ConfigAddr - Address in the flash where the configuration is saved
 DPS_ConfigStr - string to which the configuration string is to be copied into

 Return value: None

 Note(s)(if-any) :
Function takes in the configuration address,
reads the saved configuration in the flash
and copies the configuration string in the DPS_ConfigStr

 Change History:
 Author            	Date                Remarks
 100533        			29/11/2023					Initial Version
******************************************************************************/
void Config_Get(int DPS_ConfigAddr, char* DPS_ConfigStr, uint32_t length){
	uint32_t temp;
	for (int i = 0; i < (length/4); i++) {
		temp = *(__IO uint32_t*)(DPS_ConfigAddr + i * 4);
		DPS_ConfigStr[i*4 + 3] = (temp >> 24) & 0xFF;
		DPS_ConfigStr[i*4 + 2] = (temp >> 16) & 0xFF;
		DPS_ConfigStr[i*4 + 1] = (temp >> 8) 	& 0xFF;
		DPS_ConfigStr[i*4 + 0] = (temp) 			& 0xFF;
	}
}

/****************************************************************************
 Function: Config_Write
 Purpose: write new configuration in flash
 Input:
 data - data to be written in the flash
 page - page number of the flash where the data is to be written
 length - length of the data

 Return value: returns HAL_StatusTypedef value

 Note(s)(if-any) :
Takes in the new configuration string
Erases the respective page of configuration
Writes the new configuration in the page

 Change History:
 Author            	Date                Remarks
 100533        			29/11/2023					Initial Version
******************************************************************************/
HAL_StatusTypeDef Config_Write(char* data, uint32_t page, uint32_t length) {
	FLASH_EraseInitTypeDef eraseInit;
	eraseInit.TypeErase = FLASH_TYPEERASE_PAGES;
	eraseInit.Page = page;
	if (Config_EraseFlag) {
		eraseInit.NbPages = 5;
	}
	else{
		eraseInit.NbPages = 1;
	}
	eraseInit.Banks = FLASH_BANK_BOTH;

	uint32_t pageError = 0;

	HAL_FLASH_Unlock();
	__HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_ALL_ERRORS);
	if (HAL_FLASHEx_Erase(&eraseInit, &pageError) != HAL_OK) {
		return HAL_ERROR;
	}

	if (Config_EraseFlag) {
		Config_EraseFlag = 0;
		return HAL_OK;
	}

	uint32_t flashAddress = FLASH_BASE + (page * FLASH_PAGE_SIZE);

	for (int i = 0; i < length / 8; i++) {
		uint64_t doubleWord = *((uint64_t*)&data[i * 8]);
		if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, flashAddress, doubleWord) != HAL_OK) {
			return HAL_ERROR;
		}
		flashAddress += 8;
	}
	HAL_FLASH_Lock();
	return HAL_OK;
}

/****************************************************************************
 Function: DPS_ConfigSystem
 Purpose: Parse and apply the System configuration from Heartbeat
 Input: None
 Return value: HAL_StatusTypeDef

 Note(s)(if-any) :
Parses the DPS_IOConfig configuration string and applies the configuration

 Change History:
 Author            	Date                Remarks
 100533        			29/11/2023					Initial Version
******************************************************************************/
void DPS_ConfigSystem(char* config_str){

	char * token;
	char * rest = config_str;
	char * datetime;
	char * temp_d;
	char * temp_t;
	char * imei;

	strtok_r(rest, ",", &rest);																				// *
	imei = strtok_r(rest, ",", &rest);																// IMEI
	strcpy(DPS_ModuleIMEI,imei);

	strtok_r(rest, ",", &rest);																				// HEARTBEAT

	DPS_NTPSyncFLag = atoi(strtok_r(rest, ",", &rest));

	datetime = strtok_r(rest, ",", &rest);														// DATE TIME
	temp_d = strtok_r(datetime, " ", &datetime);
	temp_t = datetime;

	token = strtok_r(temp_d, "/", &temp_d);														// DAY
	RTC_tempdate.Date =  atoi(token);
	token = strtok_r(temp_d, "/", &temp_d);														// MONTH
	RTC_tempdate.Month = atoi(token);
	strncpy(RTC_Century,temp_d,2);																		// CENTURY
	RTC_tempdate.Year =  atoi(temp_d+2);																	// YEAR

	token = strtok_r(temp_t, ":", &temp_t);
	RTC_temptime.Hours = atoi(token);																			// HOUR
	token = strtok_r(temp_t, ":", &temp_t);
	RTC_temptime.Minutes =  atoi(token);																	// MINUTES
	RTC_temptime.Seconds = atoi(temp_t);																	// SECONDS

	if (RTC_SyncModuleNTP) {
		HAL_RTC_SetTime(&hrtc,&RTC_temptime,RTC_FORMAT_BIN);
		HAL_RTC_SetDate(&hrtc,&RTC_tempdate,RTC_FORMAT_BIN);
		RTC_SyncModuleNTP = 0;
	}
//	else{
//		strtok_r(rest, ",", &rest);
//	}
	Config_PayloadFreqON = atoi(strtok_r(rest, ",", &rest));
	Config_PayloadFreqOFF = atoi(strtok_r(rest, ",", &rest));
	Config_PayloadFreqIDLE = atoi(strtok_r(rest, ",", &rest));
	GPS_Latch_Status = atoi(strtok_r(rest, ",", &rest));
	boolGSMPeriodicRestartFalg = atoi(strtok_r(rest, ",", &rest));
}

/****************************************************************************
 Function: DPS_ConfigHRM
 Purpose: Parse and apply the HRM configuration
 Input: None
 Return value: HAL_StatusTypeDef

 Note(s)(if-any) :
Parses the DPS_HRMConfig configuration string and applies the configuration

 Change History:
 Author            	Date                Remarks
 100533        			29/11/2023					Initial Version
******************************************************************************/
HAL_StatusTypeDef DPS_ConfigHRM(){
	memcpy(config_temp,DPS_ConfigStr.DPS_HRMConfig,DPS_MAX_CONFIG_SIZE);
	Config_HRMConfigErr = 0;
	Config_HRMEnable = 0;
	memset(HrMeter_Master,0x00,(sizeof(HrMeter_MStruct)*NUM_HR_METER));

	if ((strstr(config_temp,DPS_CONFIG_START) == NULL) | (strstr(config_temp,DPS_CONFIG_END) == NULL)) {
		return HAL_ERROR;
	}

	uint16_t config_iotemp [6] = {0};
	uint8_t i = 0;

	char *token = strtok(config_temp, ",");
	token = strtok(NULL, ",");
	token = strtok(NULL, ",");
	token = strtok(NULL, ",");
	token = strtok(NULL, ",");

	while (token != NULL) {
		config_iotemp[i] = atoi(token);
		if ((config_iotemp[i] <= 0) && (config_iotemp[i] > DPS_NUMOFDI)){
			Config_HRMConfigErr |= (1 << enmCONFIG_CONFIGVAL);
			Config_HRMEnable = 0;
			return HAL_ERROR;
		}
		i++;
		token = strtok(NULL, ",");
		if (strstr((char *)"#", token)) {
			break;
		}
	}

	if (i != (NUM_HR_METER+1)) {
		Config_HRMConfigErr |= (1 << enmCONFIG_CONFIGLEN);
		Config_HRMEnable = 0;
		return HAL_ERROR;
	}

	for (int hrmeter_num = 0; hrmeter_num < NUM_HR_METER; ++hrmeter_num) {
		if (config_iotemp[hrmeter_num] == 0) continue;
		HrMeter_Master[hrmeter_num].En = 1;
//		HrMeter_Master[hrmeter_num].GPIOPin = HrMeter_IOPin[(config_iotemp[hrmeter_num] -1)];
//		HrMeter_Master[hrmeter_num].GPIOPort = HrMeter_IOPort[(config_iotemp[hrmeter_num] -1)];
//		HrMeter_Master[hrmeter_num].EEPROMloc = HrMeter_EEPROMloc[(config_iotemp[hrmeter_num] -1)];
		HrMeter_Master[hrmeter_num].GPIOPin = HrMeter_IOPin[hrmeter_num];
		HrMeter_Master[hrmeter_num].GPIOPort = HrMeter_IOPort[hrmeter_num];
		HrMeter_Master[hrmeter_num].EEPROMloc = HrMeter_EEPROMloc[hrmeter_num];
	}

	for (int hrmeter_num = 0; hrmeter_num < NUM_HR_METER; ++hrmeter_num){
		if (!HrMeter_Master[hrmeter_num].En) continue;
		if (!Config_HRMRequest) {
			EEPROM_RetriveHrMeterData(hrmeter_num, HrMeter_Master[hrmeter_num].EEPROMloc);
			HrMeter_Master[hrmeter_num].SecCntr = RTC_GetHRMSecBkup(hrmeter_num);
		}else{
			EEPROM_WriteHrMeterData(hrmeter_num, HrMeter_EEPROMloc[hrmeter_num]);
		}
	}

	Config_RPMHRMEnable = config_iotemp[RPM_HRM_CONFIGIDX];
	RPM_ThresholdFreq = config_iotemp[RPM_HRM_THRESHIDX];

	Config_HRMEnable = 1;
	return HAL_OK;
}

/****************************************************************************
 Function: DPS_ConfigAIDIDO
 Purpose: Parse and apply the IO configuration
 Input: None
 Return value: HAL_StatusTypeDef

 Note(s)(if-any) :
Parses the DPS_IOConfig configuration string and applies the configuration

 Change History:
 Author            	Date                Remarks
 100533        			29/11/2023					Initial Version
******************************************************************************/
HAL_StatusTypeDef DPS_ConfigAIDIDO(){
	memcpy(config_temp,DPS_ConfigStr.DPS_IOConfig,DPS_MAX_CONFIG_SIZE);
	Config_IOConfigErr = 0;
	Config_IOEnable = 0;
	if ((strstr(config_temp,DPS_CONFIG_START) == NULL) | (strstr(config_temp,DPS_CONFIG_END) == NULL)) {
		return HAL_ERROR;
	}

	uint8_t config_iotemp [6];
	uint8_t i = 0;

	char *token = strtok(config_temp, ",");
	token = strtok(NULL, ",");
	token = strtok(NULL, ",");
	token = strtok(NULL, ",");
	token = strtok(NULL, ",");
	while (token != NULL) {
		if(strcmp(token, "1") == 0){
			config_iotemp[i] = CONFIG_PIN_SET;
		}
		else if(strcmp(token, "0") == 0){
			config_iotemp[i] = CONFIG_PIN_RESET;
		}
		else{
			Config_IOConfigErr |= (1 << enmCONFIG_CONFIGVAL);
			return HAL_ERROR;
		}
		i++;
		token = strtok(NULL, ",");
		if (strstr((char *)"#", token)) {
			break;
		}
	}
	if (i != 6) {
		Config_IOConfigErr |= (1 << enmCONFIG_CONFIGLEN);
		return HAL_ERROR;
	}
	AI_IN1Config = config_iotemp[0];
	AI_IN2Config = config_iotemp[1];
	DI_IN1Config = config_iotemp[2];
	DI_IN2Config = config_iotemp[3];
	DO_DOConfig  = config_iotemp[4];
	RPM_IN1Config = config_iotemp[5];

	Config_IOEnable = 1;
	return HAL_OK;
}

/****************************************************************************
 Function: DPS_ConfigACC
 Purpose: Parse and apply the ACC configuration
 Input: None
 Return value: HAL_StatusTypeDef

 Note(s)(if-any) :
Parses the DPS_ACCConfig configuration string and applies the configuration

 Change History:
 Author            	Date                Remarks
 100533        			29/11/2023					Initial Version
******************************************************************************/
HAL_StatusTypeDef DPS_ConfigACC(){
	memcpy(config_temp,DPS_ConfigStr.DPS_AccConfig,DPS_MAX_CONFIG_SIZE);
	Config_AccConfigErr = 0;
	Config_AccEnable = 0;
	if ((strstr(config_temp,DPS_CONFIG_START) == NULL) | (strstr(config_temp,DPS_CONFIG_END) == NULL)) {
		return HAL_ERROR;
	}

	uint32_t config_acctemp_int [8] = {0};
	float_t config_acctemp_float [4] = {0};
	uint8_t i = 0;

	char *token = strtok(config_temp, ",");
	token = strtok(NULL, ",");
	token = strtok(NULL, ",");
	token = strtok(NULL, ",");
	token = strtok(NULL, ",");
	while (token != NULL) {
		if (i<8) {
			config_acctemp_int[i] = atoi(token);
		}
		else{
			config_acctemp_float[i-8] = atof(token);
		}
		i++;
		token = strtok(NULL, ",");
		if (strstr((char *)"#", token)) {
			break;
		}
	}
	if (i != 12) {
		Config_IOConfigErr |= (1 << enmCONFIG_CONFIGLEN);
		return HAL_ERROR;
	}

	ACC_VehInMotionLimit_MPS = config_acctemp_int[0];
	ACC_SuddenAccLimit_Max = config_acctemp_int[1];
	ACC_SuddenAccLimit_Extreme = config_acctemp_int[2];
	ACC_SuddenAccLimit_High = config_acctemp_int[3];
	ACC_SuddenBrakeLimit_Max = config_acctemp_int[4];
	ACC_SuddenBrakeLimit_Extreme = config_acctemp_int[5];
	ACC_SuddenBrakeLimit_High = config_acctemp_int[6];
	ACC_SuddenImpactLimit = config_acctemp_int[7];

	ACC_XYVehGradientLimit_Normal = config_acctemp_float[0];
	ACC_XYVehGradientLimit_Extreme = config_acctemp_float[1];
	ACC_XYVehGradientMaxVal = config_acctemp_float[2];
	ACC_DeviceTamperAngle = config_acctemp_float[3];

	Config_AccEnable = 1;
	return HAL_OK;
}

/****************************************************************************
 Function: DPS_ConfigAccelorometer
 Purpose: Parse and apply the IO configuration
 Input: None
 Return value: HAL_StatusTypeDef

 Note(s)(if-any) :
Parses the DPS_IOConfig configuration string and applies the configuration

 Change History:
 Author            	Date                Remarks
 100533        			29/11/2023					Initial Version
******************************************************************************/
HAL_StatusTypeDef DPS_LoadAccCaliberation(){
	Config_Get(DPS_CONFIG_ACCALIB, Config_AccCaliberation,sizeof(Config_AccCaliberation));
	memcpy(config_temp,Config_AccCaliberation,sizeof(Config_AccCaliberation));
	char *rest = config_temp;

	g_DeviceConfigCalibData.e_DeviceMountingPos = atoi(strtok_r(rest, ",", &rest));
	g_DeviceConfigCalibData.i32LatAxisOffset = atoi(strtok_r(rest, ",", &rest));
	g_DeviceConfigCalibData.i32LatAxisMinValue = atoi(strtok_r(rest, ",", &rest));
	g_DeviceConfigCalibData.i32LatAxisMaxValue = atoi(strtok_r(rest, ",", &rest));
	g_DeviceConfigCalibData.i32LongAxisOffset = atoi(strtok_r(rest, ",", &rest));
	g_DeviceConfigCalibData.i32LongAxisMinValue = atoi(strtok_r(rest, ",", &rest));
	g_DeviceConfigCalibData.i32LongAxisMaxValue = atoi(strtok_r(rest, ",", &rest));
	g_DeviceConfigCalibData.i32VerticleAxisOffset = atoi(strtok_r(rest, ",", &rest));
	g_DeviceConfigCalibData.i32VerticleAxisMinValue = atoi(strtok_r(rest, ",", &rest));
	g_DeviceConfigCalibData.i32VerticleAxisMaxValue = atoi(strtok_r(rest, ",", &rest));

	return HAL_OK;
}

/****************************************************************************
 Function: DPS_ConfigCAN
 Purpose: Parse and apply the CAN configuration
 Input: None
 Return value: HAL_StatusTypeDef

 Note(s)(if-any) :
Parses the DPS_CANConfig configuration string and applies the configuration
supports following configuration types
1. broadcast
2. PGN
3. REquest response
4. broadcast + request response
5. PGN + request response

 Change History:
 Author            	Date                Remarks
 100533        			29/11/2023					Initial Version
******************************************************************************/
HAL_StatusTypeDef DPS_ConfigCAN(){

	memcpy(config_temp,DPS_ConfigStr.DPS_CanConfig,DPS_MAX_CONFIG_SIZE);
	uint32_t gu32LoopCounter = 0;
	Config_CANEnable = 0;
	char *rest = config_temp;

	cansof = strstr(config_temp, DPS_CONFIG_START);
	caneof = strstr(config_temp, DPS_CONFIG_END);
	if ((cansof == NULL) | (caneof == NULL)){
		return HAL_ERROR;
	}

	strtok_r(rest, ",", &rest); 																		// SOF
	strtok_r(rest, ",", &rest); 																		// Device ID
	strtok_r(rest, ",", &rest); 																		// SET
	strtok_r(rest, ",", &rest); 																		// CANCONFIG
	gu8CommType = atoi(strtok_r(rest, ",", &rest));									// COMM Type
	strtok_r(rest, ",", &rest);																			// CAN
	gu32CANCommType = atoi(strtok_r(rest, ",", &rest));							// CAN Config Type
	gu32CANMode = atoi(strtok_r(rest, ",", &rest));									// CAN Mode
	gu32CANBitRate = atoi(strtok_r(rest, ",", &rest));
	cansof = rest;
	parseCANID();
	verifyCANConfigData();

	if (Config_CANConfigErr == 0) {
		switch (gu32CANCommType) {
			case 1:
				gu32CanConfigurationArray[0] = gu32CANBitRate;
				gu32CANNoOfID = gau32ConfigCANIDs_Count;
				gu32LoopCounter = 0;
				for(gu32LoopCounter = 0; gu32LoopCounter < gu32CANNoOfID; gu32LoopCounter++)
				{
					/* This loop iterates "gu32CANNoOfID" times. It's used to copy CAN ID's. */
					gu32CanConfigurationArray[gu32LoopCounter + 1] = gau32ConfigCANIDs[gu32LoopCounter];
				}

				for(;gu32LoopCounter <= MAX_CAN_IDS_SUPPORTED; ++gu32LoopCounter)
				{
					/* This loop iterates  ("MAX_CAN_IDS_SUPPORTED" - "gu32CANNoOfID" )times. It's used to copy fill 2 to remaining array of CAN ID. */
					gu32CanConfigurationArray[gu32LoopCounter + 1] = 2;
				}
				gu32CANType = 1;
				Config_CANEnable = 1;
				break;
			case 2:
				gu32CanConfigurationArrayPGN[0] = gu32CANBitRate;
				gu32CANNoOfID = gau32ConfigCANIDs_Count;
				for(gu32LoopCounter = 0; gu32LoopCounter < gu32CANNoOfID; gu32LoopCounter++)
				{
					/* This loop iterates "gu32CANNoOfID" times. It's used to copy CAN ID's. */
					gu32CanConfigurationArrayPGN[gu32LoopCounter + 1] = gau32ConfigCANIDs[gu32LoopCounter];
				}
				for(;gu32LoopCounter <= MAX_CAN_IDS_SUPPORTED; ++gu32LoopCounter)
				{
					/* This loop iterates  ("MAX_CAN_IDS_SUPPORTED"  - "gu32CANNoOfID" )times. It's used to copy fill 2 to remaining array of CAN ID. */
					gu32CanConfigurationArrayPGN[gu32LoopCounter + 1] = 2;
				}
				gu32CANType = 2;
				Config_CANEnable = 1;
				break;
			case 3:
				requestedCANIDs_Tx[0].gu32TxCANId = gu32CANBitRate;
				gu32TxCANNoOfID = gau32TxConfigCANIDs_Count;
				gu32LoopCounter = 0;
				for(gu32LoopCounter = 0; gu32LoopCounter < gu32TxCANNoOfID; gu32LoopCounter++)
				{
					 /* This loop iterates "gu32TxCANNoOfID" times. It's used to copy CAN ID's. */
					requestedCANIDs_Tx[gu32LoopCounter + 1].gu32TxCANTime =  requestedCANIDs[gu32LoopCounter].time;
					requestedCANIDs_Tx[gu32LoopCounter + 1].gu32TxCANId =  requestedCANIDs[gu32LoopCounter].id;
					requestedCANIDs_Tx[gu32LoopCounter + 1].dataLength =  requestedCANIDs[gu32LoopCounter].dataLength;
					for (int j = 0; j < requestedCANIDs[gu32LoopCounter].dataLength; j++) {
						requestedCANIDs_Tx[gu32LoopCounter + 1].data[j] = requestedCANIDs[gu32LoopCounter].data[j];
					}
				}
				for(;gu32LoopCounter <= MAX_Tx_CAN_IDS_SUPPORTED; ++gu32LoopCounter)
				{
					/* This loop iterates  ("MAX_CAN_IDS_SUPPORTED"  - "gu32TxCANNoOfID" )times. It's used to copy fill 2 to remaining array of CAN ID. */
					requestedCANIDs_Tx[gu32LoopCounter + 1].gu32TxCANId = 2;
				}
				gu32CANType = 1;
				Config_CANEnable = 1;
				break;
			case 4:
				gu32CanConfigurationArray[0] = gu32CANBitRate;
				gu32CANNoOfID = gau32ConfigCANIDs_Count;
				for(gu32LoopCounter = 0; gu32LoopCounter < gu32CANNoOfID; gu32LoopCounter++)
				{
					 /* This loop iterates "gu32CANNoOfID" times. It's used to copy CAN ID's. */
					gu32CanConfigurationArray[gu32LoopCounter + 1] = gau32ConfigCANIDs[gu32LoopCounter];
				}
				for(;gu32LoopCounter <= MAX_CAN_IDS_SUPPORTED; ++gu32LoopCounter)
				{
					/* This loop iterates  ("MAX_CAN_IDS_SUPPORTED"  - "gu32CANNoOfID" )times. It's used to copy fill 2 to remaining array of CAN ID. */
					gu32CanConfigurationArray[gu32LoopCounter + 1] = 2;
				}
				gu32TxCANNoOfID = gau32TxConfigCANIDs_Count;
				gu32LoopCounter = 0;
				for(gu32LoopCounter = 0; gu32LoopCounter < gu32TxCANNoOfID; gu32LoopCounter++)
				{
					 /* This loop iterates "gu32TxCANNoOfID" times. It's used to copy CAN ID's. */
					requestedCANIDs_Tx[gu32LoopCounter + 1].gu32TxCANTime =  requestedCANIDs[gu32LoopCounter].time;
					requestedCANIDs_Tx[gu32LoopCounter + 1].gu32TxCANId =  requestedCANIDs[gu32LoopCounter].id;
					requestedCANIDs_Tx[gu32LoopCounter + 1].dataLength =  requestedCANIDs[gu32LoopCounter].dataLength;
					for (int j = 0; j < requestedCANIDs[gu32LoopCounter].dataLength; j++) {
						requestedCANIDs_Tx[gu32LoopCounter + 1].data[j] = requestedCANIDs[gu32LoopCounter].data[j];
					}
				}
				for(;gu32LoopCounter <= MAX_Tx_CAN_IDS_SUPPORTED; ++gu32LoopCounter)
				{
					/* This loop iterates  ("MAX_CAN_IDS_SUPPORTED"  - "gu32TxCANNoOfID" )times. It's used to copy fill 2 to remaining array of CAN ID. */
					requestedCANIDs_Tx[gu32LoopCounter + 1].gu32TxCANId = 2;
				}
				gu32CANType = 1;
				Config_CANEnable = 1;
				break;
			case 5:
				gu32CanConfigurationArrayPGN[0] = gu32CANBitRate;
				gu32CANNoOfID = gau32ConfigCANIDs_Count;
				gu32LoopCounter = 0;
				for(gu32LoopCounter = 0; gu32LoopCounter < gu32CANNoOfID; gu32LoopCounter++)
				{
					/* This loop iterates "gu32CANNoOfID" times. It's used to copy CAN ID's. */
					gu32CanConfigurationArrayPGN[gu32LoopCounter + 1] = gau32ConfigCANIDs[gu32LoopCounter];
				}
				for(;gu32LoopCounter <= MAX_CAN_IDS_SUPPORTED; ++gu32LoopCounter)
				{
					/* This loop iterates  ("MAX_CAN_IDS_SUPPORTED"  - "gu32CANNoOfID" )times. It's used to copy fill 2 to remaining array of CAN ID. */
					gu32CanConfigurationArrayPGN[gu32LoopCounter + 1] = 2;
				}
				gu32TxCANNoOfID = gau32TxConfigCANIDs_Count;
				gu32LoopCounter = 0;

				for(gu32LoopCounter = 0; gu32LoopCounter < gu32TxCANNoOfID; gu32LoopCounter++)
				{
					/* This loop iterates "gu32TxCANNoOfID" times. It's used to copy CAN ID's. */
					requestedCANIDs_Tx[gu32LoopCounter + 1].gu32TxCANTime =  requestedCANIDs[gu32LoopCounter].time;
					requestedCANIDs_Tx[gu32LoopCounter + 1].gu32TxCANId =  requestedCANIDs[gu32LoopCounter].id;
					requestedCANIDs_Tx[gu32LoopCounter + 1].dataLength =  requestedCANIDs[gu32LoopCounter].dataLength;
					for (int j = 0; j < requestedCANIDs[gu32LoopCounter].dataLength; j++) {
						requestedCANIDs_Tx[gu32LoopCounter + 1].data[j] = requestedCANIDs[gu32LoopCounter].data[j];
					}
				}

				for(;gu32LoopCounter <= MAX_Tx_CAN_IDS_SUPPORTED; ++gu32LoopCounter)
				{
					/* This loop iterates  ("MAX_CAN_IDS_SUPPORTED"  - "gu32TxCANNoOfID" )times. It's used to copy fill 2 to remaining array of CAN ID. */
					requestedCANIDs_Tx[gu32LoopCounter + 1].gu32TxCANId = 2;
				}
				gu32CANType = 2;
				Config_CANEnable = 1;
				break;
			default:
				return HAL_ERROR;
				break;
		}

		if(st_DeviceConfig.gau32BAMType == 1) {
			for( int canBAMcount = 0 ; canBAMcount < 2 ; canBAMcount++ ) {
				gu32CanBAMArray[canBAMcount] = st_DeviceConfig.gau32ConfigBAMCANIDs[canBAMcount];
			}
		}
		else if(st_DeviceConfig.gau32BAMType == 2) {
			for( int canBAMcount = 0 ; canBAMcount < 2 ; canBAMcount++ ) {
				gu32PGNBAMArray[canBAMcount] = st_DeviceConfig.gau32ConfigBAMCANIDs[canBAMcount];
			}
		}
		else {
		}

	}
	else {
		return HAL_ERROR;
	}

	return HAL_OK;
}


HAL_StatusTypeDef DPS_ConfigMB()
{
	memcpy(config_temp,DPS_ConfigStr.DPS_MBConfig,sizeof(DPS_ConfigStr.DPS_MBConfig));
	Config_MBEnable = 0;
	WriteQueryReceivedflag = FALSE;

	mb485sof = strstr(config_temp, DPS_CONFIG_START);
	mb485eof = strstr(config_temp, DPS_CONFIG_END);
	if ((mb485sof == NULL) | (mb485eof == NULL))
	{
		return HAL_ERROR;
	}

	int containsMS = strstr(config_temp, "MS") != NULL;
	if(containsMS)
	{
		int Issinglewrite;
		int IsMultiwrite;

		Issinglewrite = strstr(config_temp, "SW") != NULL;
		IsMultiwrite = strstr(config_temp, "MW") != NULL;

		if(Issinglewrite == 1 || IsMultiwrite == 1 )//true if found
		{
			parseConfigMultiWriteString();

		}
		else//false if not found
		{
			parseConfigString();
		}

		if (gu32ConfigModbus485ErrorDatabase == 0) {
			Config_MBEnable = 1;
			return HAL_OK;
		}

	}
	return HAL_ERROR;
}




void parseConfigString()
{
	int containsMS = strstr(config_temp, "MS") != NULL;
	int containsME = strstr(config_temp, "ME") != NULL;
	if(containsMS && containsME)
	 {
	  char *token,*configptr;
    char* rest = config_temp;
			strtok_r(rest, ",", &rest); 																		// SOF(*)
			strtok_r(rest, ",", &rest); 																		// Device ID
			strtok_r(rest, ",", &rest); 																		// SET
			strtok_r(rest, ",", &rest); 																		// MODBUS
			gu8CommType = atoi(strtok_r(rest, ",", &rest));									// COMM Type(2)
			strtok_r(rest, ",", &rest);																			// Config string Type(6)
	    strtok_r(rest, ",", &rest); 																		// MS

	  	token = strtok_r(rest, ",", &rest);
	  	char totalqueries[]="";
	  	strcpy(totalqueries, token);
	  	st_DeviceConfig.u16MbTotalNoOfQuerys = 0;
	  	st_DeviceConfig.u16MbTotalNoOfQuerys = atoi(totalqueries);
      if((st_DeviceConfig.u16MbTotalNoOfQuerys <= 255) && (st_DeviceConfig.u16MbTotalNoOfQuerys != 0))///max 255 queries are allowed
	    {
				for (int counter = 0; counter < st_DeviceConfig.u16MbTotalNoOfQuerys; counter++)
				{
					uint16_t data[4];
					memset(data,0x00,sizeof(data));
					for (int para =0; para < 4; para++ )
					{
						token = strtok_r(rest, ",", &rest);
						if(token != 0)
						{
							int TempQuery = atoi(token);
							data[para]= (uint16_t)TempQuery;
							String_length_counter++;
						}
					}
					st_DeviceConfig.stMbMasterQuerysArr[counter].u8MbSlaveID =  data[0];
					st_DeviceConfig.stMbMasterQuerysArr[counter].u8MbFunctionCode =  data[1];
					st_DeviceConfig.stMbMasterQuerysArr[counter].u16MbStartingAddress=  data[2];
					st_DeviceConfig.stMbMasterQuerysArr[counter].u16MbNoOfPoints =  data[3];
				}
	    }
  configptr = strtok_r(rest, ",", &rest);
  strcpy(gau8ConfigModbus485UARTBR,configptr);                            /* Baudrate */
  Baudrate = atoi(gau8ConfigModbus485UARTBR);

  configptr = strtok_r(rest, ",", &rest);
	strcpy(gau8ConfigModbus485UARTParity,configptr);                        /*Parity*/
	Parity = atoi(gau8ConfigModbus485UARTParity);

	configptr = strtok_r(rest, ",", &rest);
	strcpy(gau8ConfigModbus485UARTStopBit,configptr);                       /* Stop Bit*/
	Stopbit = atoi(gau8ConfigModbus485UARTStopBit);

	configptr = strtok_r(rest, ",", &rest);
	strcpy(gau8ConfigModbus485UARTDataBit,configptr);                       /* Data Bit */
	Databit = atoi(gau8ConfigModbus485UARTDataBit);

	configptr = strtok_r(rest, ",", &rest);
	strcpy(gau8ConfigModbus485PollingTime,configptr);                       /* Polling Time */
//	gu32Modbus485PollingTime = atoi(gau8ConfigModbus485PollingTime);

	configptr = strtok_r(rest, ",", &rest);
	strcpy(gau8ConfigModbus485ResponseTime,configptr);                      /* Response Time */
//	gu32ModbusResponseTimeout = atoi(gau8ConfigModbus485ResponseTime);

	strtok_r(rest, ",", &rest);                         //ME
	strtok_r(rest, ",", &rest);                         //#
	verifyModbus485ConfigData();
	Config_MBEnable = 1;
	SinglewriteFlag = 0;
	MultiwriteFlag = 0;

	}
}

void parseConfigMultiWriteString()
{
	int containsMS = strstr(config_temp, "MS") != NULL;
	int containsME = strstr(config_temp, "ME") != NULL;
	if(containsMS && containsME)
	 {
	  char *token,*configptr;
    char* rest = config_temp;
			strtok_r(rest, ",", &rest); 																		// SOF(*)
			strtok_r(rest, ",", &rest); 																		// Device ID
			strtok_r(rest, ",", &rest); 																		// SET
			strtok_r(rest, ",", &rest); 																		// MODBUS
			gu8CommType = atoi(strtok_r(rest, ",", &rest));									// COMM Type(2)
			strtok_r(rest, ",", &rest);																			// Config string Type(6)
	    strtok_r(rest, ",", &rest); 																		// MS
	  	token = strtok_r(rest, ",", &rest);
	  	if(strstr(token,"SW" )!= NULL)
	  	{
	  		SinglewriteFlag = 1;
	  		MultiwriteFlag = 0;
	  	}
	  	else if(strstr(token,"MW" )!= NULL)
	  	{
	  		MultiwriteFlag = 1;
	  		SinglewriteFlag = 0;
	  	}
	  	else
	  	{
	  		//Need to handle later
	  	}

	  	if(SinglewriteFlag == 1)
	  	{
					token = strtok_r(rest, ",", &rest);
					char totalqueries[]="";
					strcpy(totalqueries, token);
					st_DeviceConfig.u16MbTotalNoOfQuerys = 0;
					st_DeviceConfig.u16MbTotalNoOfQuerys = atoi(totalqueries);
					if((st_DeviceConfig.u16MbTotalNoOfQuerys <= 255) && (st_DeviceConfig.u16MbTotalNoOfQuerys != 0))///max 255 queries are allowed
					{
						for (int counter = 0; counter < st_DeviceConfig.u16MbTotalNoOfQuerys; counter++)
						{
							uint16_t data[4];
							memset(data,0x00,sizeof(data));
							for (int para =0; para < 4; para++ )
							{
								token = strtok_r(rest, ",", &rest);
								if(token != 0)
								{
									int TempQuery = atoi(token);
									data[para]= (uint16_t)TempQuery;
									String_length_counter++;
								}
							}
							st_DeviceConfig.stMbMasterQuerysArr[counter].u8MbSlaveID =  data[0];
							st_DeviceConfig.stMbMasterQuerysArr[counter].u8MbFunctionCode =  data[1];
							st_DeviceConfig.stMbMasterQuerysArr[counter].u16MbStartingAddress=  data[2];
							st_DeviceConfig.stMbMasterQuerysArr[counter].u16MbNoOfPoints =  data[3];
						}
					}
	  	  }
	  	if(MultiwriteFlag == 1)
	  	{
					token = strtok_r(rest, ",", &rest);
					char totalqueries[]="";
					strcpy(totalqueries, token);
					st_DeviceConfig.u16MbTotalNoOfWrQuerys = 0;
					st_DeviceConfig.u16MbTotalNoOfWrQuerys = atoi(totalqueries);
					if((st_DeviceConfig.u16MbTotalNoOfWrQuerys <= 255) && (st_DeviceConfig.u16MbTotalNoOfWrQuerys != 0))///max 255 queries are allowed
					{
						for (int counter = 0; counter < st_DeviceConfig.u16MbTotalNoOfWrQuerys; counter++)
						{
							uint16_t data[5];
							memset(data,0x00,sizeof(data));
							for (int para =0; para < 5; para++ )
							{
								token = strtok_r(rest, ",", &rest);
								if(token != 0)
								{
									int TempQuery = atoi(token);
									data[para]= (uint16_t)TempQuery;
									String_length_counter++;
								}
							}
							st_DeviceConfig.stMbMasterMultiWriteQuerysArr[counter].u8MbSlaveID =  data[0];
							st_DeviceConfig.stMbMasterMultiWriteQuerysArr[counter].u8MbFunctionCode =  data[1];
							st_DeviceConfig.stMbMasterMultiWriteQuerysArr[counter].u16MbStartingAddress=  data[2];
							st_DeviceConfig.stMbMasterMultiWriteQuerysArr[counter].u16MbLengthtowr =  data[3];
							st_DeviceConfig.stMbMasterMultiWriteQuerysArr[counter].u8MbBytetofollow  = data[4];
							for(int databyte = 0; databyte < (st_DeviceConfig.stMbMasterMultiWriteQuerysArr[counter].u8MbBytetofollow/2); databyte++)//MultiWriteQuery[counter].lengthtowr
							{
								token = strtok_r(rest, ",", &rest);
								st_DeviceConfig.stMbMasterMultiWriteQuerysArr[counter].u16MbDatatowr[databyte] = atoi(token);
							}
						}
					}
	  	}
			configptr = strtok_r(rest, ",", &rest);
			strcpy(gau8ConfigModbus485UARTBR,configptr);                            /* Baudrate */
			Baudrate = atoi(gau8ConfigModbus485UARTBR);

			configptr = strtok_r(rest, ",", &rest);
			strcpy(gau8ConfigModbus485UARTParity,configptr);                        /*Parity*/
			Parity = atoi(gau8ConfigModbus485UARTParity);

			configptr = strtok_r(rest, ",", &rest);
			strcpy(gau8ConfigModbus485UARTStopBit,configptr);                       /* Stop Bit*/
			Stopbit = atoi(gau8ConfigModbus485UARTStopBit);

			configptr = strtok_r(rest, ",", &rest);
			strcpy(gau8ConfigModbus485UARTDataBit,configptr);                       /* Data Bit */
			Databit = atoi(gau8ConfigModbus485UARTDataBit);

			configptr = strtok_r(rest, ",", &rest);
			strcpy(gau8ConfigModbus485PollingTime,configptr);                       /* Polling Time */
			//	gu32Modbus485PollingTime = atoi(gau8ConfigModbus485PollingTime);

			configptr = strtok_r(rest, ",", &rest);
			strcpy(gau8ConfigModbus485ResponseTime,configptr);                      /* Response Time */
			//	gu32ModbusResponseTimeout = atoi(gau8ConfigModbus485ResponseTime);

			strtok_r(rest, ",", &rest);                         //ME
			strtok_r(rest, ",", &rest);                         //#
			verifyModbus485ConfigData();
			WriteQueryReceivedflag = TRUE;

			}
  }



/******************************************************************************
* Function : verifyModbus485ConfigData()
*//**
* \b Description:
*
* This function is used to verify MB485 config data as defined in configuration doc.
* This function verifies whether the data is appropriate else updates error database
* variable w.r.t found error.
*
* PRE-CONDITION:
*
* POST-CONDITION: None
*
* @return 		None.
*
* \b Example Example:
* @code
*
* 	verifyModbus485ConfigData();
*
* @endcode
*
* @see
*
* <br><b> - HISTORY OF CHANGES - </b>
*
* <table align="left" style="width:800px">
* <tr><td> Date       </td><td> Software Version </td><td> Initials </td><td> Description </td></tr>
* <tr><td> 08/03/2022 </td><td> 0.0.1            </td><td> HL100133 </td><td> Interface Created </td></tr>
* </table><br><br>
* <hr>
*
*******************************************************************************/
static void verifyModbus485ConfigData(void)
{
   /* Log Errors Related to Modbus485 Configuration */
//	uint32_t u32LoopCounter = 0;
	gu32ConfigModbus485ErrorDatabase = 0;

  if((st_DeviceConfig.u16MbTotalNoOfQuerys <= CONFIG_MB485_MAXPARAMS) && (st_DeviceConfig.u16MbTotalNoOfQuerys != 0))
  	gu32ConfigModbus485ErrorDatabase &= ~(1 << enmCONFIG_MB485DATAPOINTS);
	else
		gu32ConfigModbus485ErrorDatabase |= (1 << enmCONFIG_MB485DATAPOINTS);

  if((st_DeviceConfig.u16MbTotalNoOfQuerys * 4) == String_length_counter)
  {
  		for(int FC_index = 0;FC_index < st_DeviceConfig.u16MbTotalNoOfQuerys; FC_index++)
  		{
  			if((st_DeviceConfig.stMbMasterQuerysArr[FC_index].u8MbFunctionCode == 1)||(st_DeviceConfig.stMbMasterQuerysArr[FC_index].u8MbFunctionCode == 2)||
  				 (st_DeviceConfig.stMbMasterQuerysArr[FC_index].u8MbFunctionCode == 3)||(st_DeviceConfig.stMbMasterQuerysArr[FC_index].u8MbFunctionCode == 4))
  		  {
  				gu32ConfigModbus485ErrorDatabase &= ~(1 << enmCONFIG_MB485FUCNTIONCODE);
  		  }
  			else
					gu32ConfigModbus485ErrorDatabase |= (1 << enmCONFIG_MB485FUCNTIONCODE);
  		}

   }


	if(Baudrate != 0)
		gu32ConfigModbus485ErrorDatabase &= ~(1 << enmCONFIG_MB485UARTBR);
	else
		gu32ConfigModbus485ErrorDatabase |= (1 << enmCONFIG_MB485UARTBR);

	if(Parity == 0 || Parity == 1 || Parity == 2)
		gu32ConfigModbus485ErrorDatabase &= ~(1 << enmCONFIG_MB485UARTPARITY);
	else
		gu32ConfigModbus485ErrorDatabase |= (1 << enmCONFIG_MB485UARTPARITY);

	if(Stopbit == 1 || Stopbit == 2)
		gu32ConfigModbus485ErrorDatabase &= ~(1 << enmCONFIG_MB485UARTSTOPBIT);
	else
		gu32ConfigModbus485ErrorDatabase |= (1 << enmCONFIG_MB485UARTSTOPBIT);

	if(Databit == 8)
		gu32ConfigModbus485ErrorDatabase &= ~(1 << enmCONFIG_MB485UARTDATABIT);
	else
		gu32ConfigModbus485ErrorDatabase |= (1 << enmCONFIG_MB485UARTDATABIT);

	if(atoi(gau8ConfigModbus485PollingTime) > TEN_SEC)
		gu32ConfigModbus485ErrorDatabase |= (1 << enmCONFIG_MB485POLLTIME);
	else
	{
		gu32Modbus485PollingTime = atoi(gau8ConfigModbus485PollingTime);
		gu32ConfigModbus485ErrorDatabase &= ~(1 << enmCONFIG_MB485POLLTIME);
	}

	if(atoi(gau8ConfigModbus485ResponseTime) > TEN_SEC)
		gu32ConfigModbus485ErrorDatabase |= (1 << enmCONFIG_MB485RESPTIME);
	else
	{	gu32ModbusResponseTimeout = atoi(gau8ConfigModbus485ResponseTime);
		gu32ConfigModbus485ErrorDatabase &= ~(1 << enmCONFIG_MB485RESPTIME);
	}

}
