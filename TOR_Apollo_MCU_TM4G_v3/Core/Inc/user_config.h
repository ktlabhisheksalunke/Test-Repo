/*
 * config.h
 *
 *  Created on: Oct 30, 2023
 *      Author: abhisheks
 */

#ifndef INC_USER_CONFIG_H_
#define INC_USER_CONFIG_H_

#include "main.h"
#include "string.h"
#include <stdlib.h>

#define DPS_MAX_CONFIG_SIZE 2048
#define DPS_NUM_OF_CONFIG

#define DPS_CONFIG_STARTADDR		0x0803D000
#define DPS_CONFIG_STARTPAGE		122

#define DPS_CONFIG_ACCALIB			0x0803C800
#define DPS_CONFIG_ACCALIB_PAGE	121

#define DPS_CONFIG_HRMETER			0x0803D000
#define DPS_CONFIG_HRMETER_PAGE	122

#define DPS_CONFIG_AIDIDO				0x0803D800
#define DPS_CONFIG_AIDIDO_PAGE	123

#define DPS_CONFIG_CAN					0x0803E000
#define DPS_CONFIG_CAN_PAGE			124

#define DPS_CONFIG_MB						0x0803E800
#define DPS_CONFIG_MB_PAGE			125

#define DPS_CONFIG_ACC					0x0803F000
#define DPS_CONFIG_ACC_PAGE			126

#define DPS_CONFIG_START				"*"
#define DPS_CONFIG_END					"#"

#define DPS_CONFIG_SET					"SET"
#define DPS_CONFIG_GET					"GET"
#define DPS_CONFIG_CMD					"CMD"

#define DPS_CONFIGID_CAN				"CANMCU"
#define DPS_CONFIGID_IO					"IOMCU"
#define DPS_CONFIGID_MB					"MODBUSMCU"
#define DPS_CONFIGID_HRM				"HRMMCU"
#define DPS_CONFIGID_ACC				"GSENSOR"
#define DPS_CONFIGID_GETMB			"MODBUSMCUDATA"
#define DPS_CONFIGID_GETCAN			"CANMCUDATA"
#define DPS_CONFIGID_GETIO			"IOMCUDATA"
#define DPS_CONFIGID_MCU				"MCU"

#define DPS_CONFIGID_CLR				"CNFCLR"
#define DPS_CONFIGID_RST				"RST"

#define DPS_SW_MB					"SW"
#define DPS_MW_MB					"MW"

typedef struct
{
	char DPS_IOConfig		[DPS_MAX_CONFIG_SIZE];
	char DPS_CanConfig	[DPS_MAX_CONFIG_SIZE];
	char DPS_MBConfig	[DPS_MAX_CONFIG_SIZE];
	char DPS_MBWriteConfig	[DPS_MAX_CONFIG_SIZE];
	char DPS_HRMConfig	[DPS_MAX_CONFIG_SIZE];
	char DPS_AccConfig	[DPS_MAX_CONFIG_SIZE];
}strctDPSConfig;

typedef enum
{
	enmCONFIG_CONFIGLEN = 1,
	enmCONFIG_CONFIGVAL,
}enmIOConfigErrors;

typedef enum
{
	enmCONFIG_SET = 0,
	enmCONFIG_SETIO,
	enmCONFIG_SETCAN,
	enmCONFIG_SETMB,
	enmCONFIG_SETHRM,
	enmCONFIG_SETACC,
	enmCONFIG_GET,
	enmCONFIG_GETIO,
	enmCONFIG_GETCAN,
	enmCONFIG_GETMB,
	enmCONFIG_GETHRM,
	enmCONFIG_GETCNF,
	enmCONFIG_CMD,
	enmCONFIG_CMDCLEAR,
	enmCONFIG_CMDRST,
	enmCONFIG_CMDACCCALIB,
	enmCONFIG_NONE,
}enmConfigOperation;

void Config_Routine(void);
void Config_Reset(void);
void Config_Save(void);
void Config_Get(int DPS_ConfigAddr, char* DPS_ConfigStr, uint32_t length);
HAL_StatusTypeDef Config_Write(char* data, uint32_t page, uint32_t length);

HAL_StatusTypeDef DPS_ConfigAIDIDO();
HAL_StatusTypeDef DPS_ConfigCAN();
HAL_StatusTypeDef DPS_ConfigMB();
HAL_StatusTypeDef DPS_ConfigMBWrite();
HAL_StatusTypeDef DPS_ConfigAccelorometer();
HAL_StatusTypeDef DPS_ConfigHRM();
HAL_StatusTypeDef DPS_ConfigACC();
void parseConfigString();
void parseConfigMultiWriteString();
void DPS_ConfigSystem(char* config_str);
HAL_StatusTypeDef DPS_LoadAccCaliberation();

#endif /* INC_USER_CONFIG_H_ */
