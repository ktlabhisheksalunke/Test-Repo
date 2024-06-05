/*
 * user_modbus_config.c
 *
 *  Created on: Dec 4, 2023
 *      Author: pratikshaw
 */

/****************************************Includes****************************************/
#include "main.h"
#include "externs.h"
#include "user_modbus_rs485.h"
#include "user_modbus_config.h"
#include "user_UART.h"
#include "user_Timer.h"
#include <user_timer.h>

char * mb485sof = NULL;
char * mb485eof = NULL;
#define CONFIG_MB485_MAXPARAMS  (255)
/* Modbus 485 Configuration Update Variables */
uint32_t gau8ConfigModbus485SlaveID[CONFIG_MB485_MAXPARAMS] = {'0'};
uint32_t gau32ConfigModbus485Address[CONFIG_MB485_MAXPARAMS] = {'0'};
uint32_t gau32ConfigModbus485DFunctionCode[CONFIG_MB485_MAXPARAMS] = {'0'};
uint32_t gau32ConfigModbus485NoPoints[CONFIG_MB485_MAXPARAMS] = {'0'};
char gau8ConfigModbus485UARTBR[8] = "";
char gau8ConfigModbus485UARTParity[2] = "";
char gau8ConfigModbus485UARTStopBit[2] = "";
char gau8ConfigModbus485UARTDataBit[2] = "";
uint32_t Baudrate;
uint8_t Parity,Stopbit,Databit;
char gau8ConfigModbus485PollingTime[7] = "";
char gau8ConfigModbus485ResponseTime[7] = "";
uint32_t gu32Modbus485PollingTime = ONE_SEC;
uint32_t gu32Modbus485ResponseTime = ONE_SEC;
uint32_t gu32LoopCounter = 0;



