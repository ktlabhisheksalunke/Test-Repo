/*
 * externs.h
 *
 *  Created on: Oct 19, 2023
 *      Author: abhisheks
 */

#ifndef INC_EXTERNS_H_
#define INC_EXTERNS_H_

#include <user_config.h>
#include "user_ApplicationDefines.h"
#include "user_UART.h"
#include "user_can.h"
#include "user_can_config.h"
#include "user_system.h"
#include "user_queue.h"
#include "user_modbus_rs485.h"
#include "user_Accelorometer.h"
#include "user_eeprom.h"
#include "user_HourMeter.h"
#include "user_rtc.h"

/*************************** Peripheral Handlers ******************************/
extern ADC_HandleTypeDef hadc1;
extern DMA_HandleTypeDef hdma_adc1;
extern CAN_HandleTypeDef hcan1;
extern I2C_HandleTypeDef hi2c1;
extern IWDG_HandleTypeDef hiwdg;
extern RTC_HandleTypeDef hrtc;
extern RTC_TimeTypeDef RTC_time;
extern RTC_DateTypeDef RTC_date;
extern TIM_HandleTypeDef htim1;
extern TIM_HandleTypeDef htim7;
//extern UART_HandleTypeDef huart1;
//extern UART_HandleTypeDef huart2;
//extern UART_HandleTypeDef huart3;

/********************************* System *************************************/
extern enmSYS_State SYS_State;
void CAN_Error_Handler(void);

/********************************** DPS ***************************************/

extern strctBuffQUEUE DPS_BuffQueue;
extern strctQUEUE DPS_LiveQueue;

extern volatile uint32_t DPS_HeartBeatTimer;
extern volatile uint32_t DPS_PwrCycleTimer;
extern volatile uint32_t DPS_IsModuleAlive;
extern volatile uint32_t DPS_ModuleWkup;
extern 					uint8_t DPS_NTPSyncFLag;
extern volatile uint32_t DPS_PwrCycleTimer;

extern CONFIG_PinState AI_IN1Config;
extern CONFIG_PinState AI_IN2Config;
extern CONFIG_PinState DI_IN1Config;
extern CONFIG_PinState DI_IN2Config;
extern CONFIG_PinState DO_DOConfig;
extern CONFIG_PinState RPM_IN1Config;

extern char DPS_ModuleIMEI[20];
//extern char DPS_UARTConfigBuff [DPS_MAX_CONFIG_SIZE];
extern char DPS_UART_Resp;
extern uint32_t DPS_UART_RxIndex;

extern enmUART_State UART_RxState;
extern volatile uint32_t UART_RxTimer;

extern uint8_t DI_IN1Flag;
extern uint8_t DI_IN2Flag;
extern uint8_t DO_DOFlag;

extern uint32_t AI_RawData[5];
extern volatile uint32_t AI_ADCRestartTimer;
extern float AI_BattVlt;
extern float AI_InputVlt;

extern char dinfo[50];

extern strctQUEUE DPS_LiveQueue;

extern volatile uint32_t DPS_PayloadQueueEnqueue;

extern uint8_t gu8CommType;

extern uint32_t Config_PayloadFreqON;
extern uint32_t Config_PayloadFreqOFF;
extern uint32_t Config_PayloadFreqIDLE;
extern uint32_t GPS_Latch_Status;
extern _Bool boolGSMPeriodicRestartFalg;

/****************************** Config ************************************/
extern strctDPSConfig 	DPS_ConfigStr;
extern char DPS_TempConfig	[DPS_MAX_CONFIG_SIZE];
extern char 						Config_SelStr[64];
extern HAL_StatusTypeDef Config_Status;

extern char Config_StrID [10];

extern uint8_t Config_CANEnable;
extern uint8_t Config_IOEnable;
extern uint8_t Config_MBEnable;
extern uint8_t Config_HRMEnable;
extern uint8_t Config_RPMHRMEnable;
extern uint8_t Config_AccEnable;

extern HAL_StatusTypeDef 	Config_IOStatus;
extern HAL_StatusTypeDef 	Config_CANStatus;
extern HAL_StatusTypeDef Config_MBStatus;
extern HAL_StatusTypeDef Config_HRMStatus;
extern HAL_StatusTypeDef Config_AccStatus;

extern uint8_t						Config_IORequest;
extern uint8_t						Config_CanRequest;
extern uint8_t						Config_MBRequest;
extern uint8_t						Config_HRMRequest;
extern uint8_t						Config_AccRequest;


extern uint8_t						Config_ReconfigReq;

extern uint8_t Config_Reload;
//extern uint8_t Config_MBWriteCmd;

/****************************** CAN ************************************/

extern char * cansof ;
extern char * caneof ;

/* CAN Config */
extern uint32_t gu32CANType;
extern uint32_t gu32CANCommType;
extern uint32_t gu32OperateCAN;
extern uint32_t gu32CANBitRate;
extern uint32_t gu32CANMode;
extern uint32_t gu32CANNoOfID;
extern uint32_t gu32TxCANNoOfID;

extern uint32_t gau32ConfigCANIDs_Count;
extern uint32_t gau32TxConfigCANIDs_Count;

extern uint32_t Config_CANConfigErr;

extern uint32_t gau32ConfigCANIDs_Count;
extern uint32_t gau32TxConfigCANIDs_Count;
extern uint32_t gau32ConfigCANIDs[CONFIG_CAN_ID];
extern uint32_t gau32TxConfigCANIDs[CONFIG_CAN_ID_Tx];

extern uint32_t gu32CanConfigurationArray[(MAX_CAN_IDS_SUPPORTED + 1)];
extern uint32_t gu32CanConfigurationArrayPGN[(MAX_CAN_IDS_SUPPORTED + 1)];
extern volatile uint32_t gu32CANCommandResponseTimeout;

//extern RequestedCANID_t requestedCANIDs_Tx[MAX_Tx_CAN_IDS_SUPPORTED];
extern uint32_t u32CanIdsReceived_2[CAN_BUFFER_LENGTH];

/* CAN Configuration Update Variables */
extern char gau8ConfigCANIsEnabled[2];
extern char gau8ConfigCANMode[2];
extern char gau8ConfigCANbitRate[5];
extern char gau8ConfigCANNoOfIDs[3];
extern char gau8ConfigTxCANNoOfIDs[3];
extern char gau8ConfigCANType[2];

extern uint64_t gu64CanMessageArray[MAX_CAN_IDS_SUPPORTED];
extern uint64_t gu64CanMessageArrayTest[MAX_CAN_IDS_SUPPORTED];
extern uint64_t gu64CanMessageIDPGN[MAX_CAN_IDS_SUPPORTED];

extern RequestedCANIDTimer_t requestedCANID_Time[MAX_Tx_CAN_IDS_SUPPORTED];

extern char canCSV_Enabled;

extern RequestedCANID requestedCANIDs[MAX_Tx_CAN_IDS_SUPPORTED + 1];

extern strCanReceivedMsg unCanReceivedMsgs_2[MAX_CAN_IDS_SUPPORTED];
extern uint32_t u32CanIdsReceived_2[CAN_BUFFER_LENGTH];
extern uint32_t gau32ConfigBAMCANIDs[3];
extern unCan1939CommandId unCan1939ReceivedBAMId[3];
extern unCan1939CommandId unCan1939ReceivedId[CAN_BUFFER_LENGTH];




extern RequestedCANID requestedCANIDs[MAX_Tx_CAN_IDS_SUPPORTED + 1];

extern RequestedCANID_t requestedCANIDs_Tx[MAX_Tx_CAN_IDS_SUPPORTED + 1];;
//extern RequestedCANIDTimer_t requestedCANID_Time[MAX_Tx_CAN_IDS_SUPPORTED];

extern uint32_t gu32CANQueryCommandResponseReceivedFlag;

extern uint8_t    TxData[8];
extern uint8_t    RxData[8];
extern uint32_t	TxMailbox;

//extern volatile enmCanQueryState canCurrentState;

extern _Bool receivingCANDATA ;
extern _Bool u32MultiFrameFlag ;
extern _Bool u32MultiFramRecvFlag ;

extern uint16_t MultiFramecounter;
extern uint32_t NumberMultiFrame ;
extern uint32_t u32MultiFrameCommandId ;
extern uint32_t u32CanMsgID_1;
extern uint32_t multiNumBytes;

extern uint8_t MultiFrameCAN_ID[150];

extern uint32_t u32MultiFrameRequestId;
extern uint8_t MultiFrameRequest[8];

extern uint32_t gu32CanBAMArray[3];
extern uint32_t gu32PGNBAMArray[3];

//extern unCan1939CommandId unCan1939ReceivedBAMId[3];
//extern unCan1939CommandId unCan1939ReceivedId[MAX_CAN_IDS_SUPPORTED];

extern uint16_t MultiFrameReceivedCounter;

extern _Bool receivedAllBAMFrames;
extern _Bool boolOnIntBattCANErrorFlag;

extern uint32_t  TxMailbox;


/****************************** UART ************************************/
extern volatile uint8_t DPS_UART_TxCmplt;

extern volatile uint32_t UART_TxTimer;

extern struct UART_Buff *Buff_H;
extern struct UART_Buff *Buff_T;
extern struct UART_Buff *Buff_N;

extern uint16_t UART_DebugStrIdx;
extern char UART_DebugBuff[2048];
/****************************** RTC ************************************/
extern char RTC_Century [3];
extern volatile uint32_t RTC_SyncTimer;
extern uint8_t RTC_SyncModuleNTP;

extern RTC_TimeTypeDef RTC_temptime;
extern RTC_DateTypeDef RTC_tempdate;

/****************************** MODBUS ************************************/
#define CONFIG_MB485_MAXPARAMS  (255)

extern char * mb485sof ;
extern char * mb485eof ;

extern uint32_t gu32Modbus485RegisterFetch;
extern volatile uint32_t gu16ModbusFrameEndTimer;
//extern uint32_t gu32Modbus485SlaveID;
extern uint8_t u8MBQueryRegisterAddressCounter;
extern uint8_t u8MBWrQueryRegisterAddressCounter;
extern char gu8MBRTUPayloadString[];
extern volatile uint32_t gu32ModbusResponseTimeout;
extern volatile uint32_t gu32ModbusPollDelay;
extern volatile uint32_t gu32ModbusCycelRestartTmr;
extern uint8_t MultiwriteFlag;
extern uint8_t SinglewriteFlag;
extern _Bool WriteQueryReceivedflag;
extern uint8_t TempBuff[8];
extern char TempMbPayloadBuff[35];
extern char TempMbPayloadBuff1[1000];
extern uint8_t MBWritePayload;

/* Modbus 485 Configuration Update Variables */
extern char gau8ConfigModbus485IsEnabled[2];
extern char gau8ConfigModbus485Termination[2];
extern char gau8ConfigModbus485DataPoints[3];
extern char gau8ConfigModbus485UARTBR[8];
extern uint32_t Baudrate;
extern uint8_t Parity,Stopbit,Databit;
extern char gau8ConfigModbus485UARTStartBit[2];
extern char gau8ConfigModbus485UARTStopBit[2];
extern char gau8ConfigModbus485UARTDataBit[2];
extern char gau8ConfigModbus485UARTParity[2];
extern char gau8ConfigModbus485PollingTime[7];
extern char gau8ConfigModbus485ResponseTime[7];
extern uint32_t gau8ConfigModbus485SlaveID[CONFIG_MB485_MAXPARAMS];
extern uint32_t gau32ConfigModbus485Address[CONFIG_MB485_MAXPARAMS];
extern uint32_t gau32ConfigModbus485DFunctionCode[CONFIG_MB485_MAXPARAMS];
extern uint32_t gau32ConfigModbus485NoPoints[CONFIG_MB485_MAXPARAMS];
/* Modbus 485 Config */
extern uint32_t gu32OperateModbus485;
extern uint32_t gu32Modbus485UARTBR;
extern uint32_t gu32Modbus485PollingTime;
extern uint32_t gu32Modbus485ResponseTime;
extern uint32_t gu32Modbus485RegisterFetch;

extern uint32_t gu32Modbus485PollingTime;
extern uint32_t gu32Modbus485SlaveID[255];
extern uint32_t gu32MBRTUClientFuncCode[255];
extern uint32_t gu32MBRTUClientAddress[255];
extern uint32_t gu32MBRTUClientNoofPoints[255];

/****************************** ACC ************************************/
extern volatile uint32_t gu32SensorVheInMotionTmr;
extern volatile uint32_t gu32SensorFsmMotionTmr;
extern stAccData_t g_stDeviceAccLiveData;

extern uint8_t g_u8AccCalibratemode;
extern uint8_t g_u8GSensCalibState;
extern st_DeviceConfigCalibData g_DeviceConfigCalibData;

extern char Config_AccCaliberation [128];

extern int32_t ACC_VehInMotionLimit_MPS;								//1
extern int32_t ACC_SuddenAccLimit_Max;									//2
extern int32_t ACC_SuddenAccLimit_Extreme;							//3
extern int32_t ACC_SuddenAccLimit_High;									//4
extern int32_t ACC_SuddenBrakeLimit_Max;								//5
extern int32_t ACC_SuddenBrakeLimit_Extreme;						//6
extern int32_t ACC_SuddenBrakeLimit_High;								//7
extern int32_t ACC_SuddenImpactLimit;										//8
extern float_t ACC_DeviceTamperAngle;										//9
extern float_t ACC_XYVehGradientLimit_Normal;						//10
extern float_t ACC_XYVehGradientLimit_Extreme;					//11
extern float_t ACC_XYVehGradientMaxVal;									//12

/**************************** HR METER *********************************/
extern HrMeter_MStruct HrMeter_Master[HOURMETER_COUNT];
extern uint16_t HrMeter_DataWrCycle[HOURMETER_COUNT];
extern volatile uint32_t gu32GPIODelay;

extern GPIO_TypeDef* HrMeter_IOPort[HOURMETER_COUNT];
extern uint16_t HrMeter_IOPin[HOURMETER_COUNT];
extern uint16_t HrMeter_EEPROMloc[HOURMETER_COUNT];

extern volatile uint32_t LED_CommLEDDelay;

/****************************** RPM ***********************************/
extern volatile uint8_t RPM_HrMeterStart;
extern volatile uint32_t RPM_frequency;
extern uint32_t RPM_ThresholdFreq;
extern enmRTC_State RTC_SyncState;

/****************************** DBUG **********************************/
extern volatile uint32_t UART_DebugTxTmr;

#endif /* INC_EXTERNS_H_ */
