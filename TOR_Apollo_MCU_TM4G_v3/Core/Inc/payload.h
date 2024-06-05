/*
 * payload.h
 *
 *  Created on: Oct 23, 2023
 *      Author: abhisheks
 */

#ifndef INC_PAYLOAD_H_
#define INC_PAYLOAD_H_

#include <stdint.h>

#define START_OF_FRAME 				"*"
#define END_OF_FRAME	 				"#"
#define FIRMWARE_VER					"TM4G_v3_L433.1.24.06.04.0.0.6" //TorXXX.Ver.YY.MM.DD.<Rev>X.Y.Z
#define BOOT_REGION						"DPS"

#define MODBUS_SOF "MS,"
#define MODBUS_EOF "ME,"

typedef enum
{
	enmDiagnostic_SUCCESS_OK = 0, // FOTA SUCCESS
	enmDiagnostic_CAN_ID,
	enmDiagnostic_DEVICE_POWER_ON,
	enmDiagnostic_HW_ID_MISMATCH_ERROR, // Device HW ID Mismatch
	enmDiagnostic_FOTA_REQ_RX,
	enmDiagnostic_FOTA_REQ_RX_ERROR,
	enmDiagnostic_POWER_SUPPLY_ERROR,
	enmDiagnostic_RX_FILE_SIZE_ERROR, //Received File Size is Greater than (FLASH_SIZE-4k)/2
	enmDiagnostic_FILE_DONWLOAD_ERROR,
	enmDiagnostic_FLASH_ERRASE_ERROR,
	enmDiagnostic_FLASH_Write_ERROR,
	enmDiagnostic_BOOT_REGION_SELECTION_ERROR,
	enmDiagnostic_MEM_WR_CHK_SUM_ERROR, //MEM sector written, but downloaded DATA and Written data in MEM Mismatch
	enmDiagnostic_BOOT_REGION_JUMP_ERROR,
	enmDiagnostic_IMOBI_REQ_RX,
	enmDiagnostic_IMOBI_SUCCESS_OK,
	enmDiagnostic_IDLE = 99,
	enmDiagnostic_MB_WRITE_QUERY_SUCCESSFUL,
	enmDiagnostic_CONFIG_RECV_ACK,
	enmDiagnostic_CONFIG_REQUEST,
//	enmDiagnostic_CONFIG_FAIL,
	enmDiagnostic_GET_CONFIG,
	enmDiagnostic_GET_IOCONFIG,
	enmDiagnostic_GET_CANCONFIG,
	enmDiagnostic_GET_MBCONFIG,
	enmDiagnostic_GET_MCU,
	enmDiagnostic_CMD_CNFCLR,
	enmDiagnostic_NOPWRSUPPLY_CNFDISC,
//	enmDiagnostic_HEARTBEAT,

}enmDiagnosticStatus;

char* DPS_GenPayload(void);
char * DPS_GenDiagnostic(enmDiagnosticStatus DiagnosticStatus);
void DiagnosticString(char * systemPayload, enmDiagnosticStatus DiagnosticStatus);
void updateCanPayload(uint32_t data ,char * systemPayload, uint32_t * CanConfigurationArray, uint64_t * CanMessageArray);
void updateCanPayloadPGN(uint32_t data ,char * systemPayload, uint32_t * CanConfigurationArray, uint64_t * CanIDArray, uint64_t * CanMessageArray);
char * DPS_MBWritePayload(void);
void updateMultiFrameCanPayload(uint8_t data[] ,char * systemPayload);
#endif /* INC_PAYLOAD_H_ */
