/*
 * user_UART.c
 *
 *  Created on: Nov 2, 2023
 *      Author: abhisheks
 */
#include "main.h"
#include "externs.h"
#include "user_ApplicationDefines.h"
#include "user_UART.h"
#include "user_Timer.h"
#include "user_rtc.h"
#include "payload.h"
#include "user_config.h"

/****************************************************************************/

char UART_RxBuff[DPS_UART_BUFF_SIZE] = "";
char UART_RxResp;
uint32_t UART_RxIndex = 0;
enmUART_State UART_RxState = UART_IDLE;

volatile uint8_t UART_TxCmplt = FALSE;
static char* UART_TxBuff;
static uint32_t UART_TxCounter;
static uint32_t UART_TxLen;

struct UART_Buff *UART_Buff_H;
struct UART_Buff *UART_Buff_T;
struct UART_Buff *UART_Buff_N;

uint8_t UART_IsBuffData = 0;

#if (UART_DEBUGENABLE)
char UART_DebugBuff[2048] = {'0'};
uint16_t UART_DbCharCounter = 0;
uint16_t UART_DebugStrIdx = 0;
#endif
/****************************************************************************/

/****************************************************************************
 Function: DPS_ConfigRcvCallback
 Purpose: Callback function to save the incoming data on UART
 Input: None
 Return value: None

 Note(s)(if-any) :

 Change History:
 Author            	Date                Remarks
 100533        			29/11/2023					Initial Version
******************************************************************************/

void DPS_ConfigRcvCallback(void){
	if(UART_RxState == UART_IDLE){
		UART_RxIndex = 0;
		UART_RxState = UART_INPROGRESS;
	}
	UART_RxResp = LL_USART_ReceiveData8(USART1);
	if (UART_RxIndex < DPS_UART_BUFF_SIZE) {
		UART_RxBuff[UART_RxIndex++] = UART_RxResp;
		UART_RxTimer = ONEHUNDRED_MS;
	}
	else{
		UART_RxTimer = 0;
	}
}

/****************************************************************************
 Function: UART_Transmit
 Purpose: Transmit data on UART
 Input: None
 Return value: None

 Note(s)(if-any) :

 Change History:
 Author            	Date                Remarks
 100533        			29/11/2023					Initial Version
******************************************************************************/

void UART_Transmit()
{

	if((Queue_IsQueueEmpty(&DPS_LiveQueue) == FALSE )&& (DPS_LiveQueue.data[DPS_LiveQueue.tail] != NULL) && ( UART_TxCmplt == FALSE) && (boolGSMPeriodicRestartFalg == 0))
	 {
		UART_TxBuff = DPS_LiveQueue.data[DPS_LiveQueue.tail];
		UART_TxLen = strlen((const char *)UART_TxBuff);
		LL_USART_TransmitData8(USART1,UART_TxBuff[UART_TxCounter++]);
		UART_TxCmplt = TRUE;
	 }
	else if((Queue_IsBuffQEmpty(&DPS_BuffQueue) == FALSE )&& (DPS_BuffQueue.data[DPS_BuffQueue.tail] != NULL) && ( UART_TxCmplt == FALSE) && (boolGSMPeriodicRestartFalg == 0))
	 {
		UART_TxBuff = DPS_BuffQueue.data[DPS_BuffQueue.tail];
		UART_TxLen = strlen((const char *)UART_TxBuff);
		LL_USART_TransmitData8(USART1,UART_TxBuff[UART_TxCounter++]);
		UART_TxCmplt = TRUE;
		UART_IsBuffData = 1;
	 }

	if((!LL_USART_IsActiveFlag_TXE(USART1) && ( UART_TxCmplt == TRUE) ))
	{
		/*Do Nothing . Wait For Previous Character Transmission */
	}
	else if( UART_TxCmplt == TRUE)
	{
		if((UART_TxCounter < UART_TxLen) && (UART_TxCmplt == TRUE))
		{
			LL_USART_TransmitData8(USART1,UART_TxBuff[UART_TxCounter++]);
		}
		else if(UART_TxCmplt == TRUE)
		{
			UART_TxCounter = 0;
			UART_TxLen = 0;
			UART_TxCmplt = FALSE;
			UART_TxTimer = FIVEHUNDRED_MS;
			if (UART_IsBuffData) {
				Queue_BuffQDequeue(&DPS_BuffQueue);
				UART_IsBuffData = 0;
			}
			else{
				Queue_Dequeue(&DPS_LiveQueue);
			}
		}
		else{}
	}
	else{}
}

/****************************************************************************
 Function: UART_Routine
 Purpose: Routine to transmit and receive data on UART
 Input: None
 Return value: None

 Note(s)(if-any) :
Jumps to SYS_Config if received data consists of * and #

 Change History:
 Author            	Date                Remarks
 100533        			29/11/2023					Initial Version
******************************************************************************/
void UART_Routine(void){
	if ((UART_TxTimer == 0) && DPS_IsModuleAlive) {
		UART_Transmit();
	}

#if(UART_DEBUGENABLE == TRUE)
	UART_DbCharCounter=strlen(UART_DebugBuff);

	if(UART_DbCharCounter>1){
		if(UART_DebugStrIdx<UART_DbCharCounter) {
			if(!LL_USART_IsActiveFlag_TXE(USART3)) {
				/*Do Nothing . Wait For Previous Character Transmission */
			}
			else {
				LL_USART_TransmitData8(USART3, UART_DebugBuff[UART_DebugStrIdx++]);
			}
		}
		else if(UART_DebugStrIdx==UART_DbCharCounter) {
			memset(UART_DebugBuff, 0x00,  sizeof(UART_DebugBuff)); /* Clear  Buffer */
			UART_DebugStrIdx=0;
			UART_DbCharCounter=0;
		}
		else{}
	}
#endif

	if(UART_RxState == UART_INPROGRESS && UART_RxTimer == 0){
		size_t len;
		char *soconfig = UART_RxBuff;
		char *eoconfig;

		while ((strstr(soconfig, DPS_CONFIG_START) != NULL) && (strstr(soconfig, DPS_CONFIG_END) != NULL)){
			soconfig = strstr(soconfig, DPS_CONFIG_START);
			eoconfig = strstr(soconfig, DPS_CONFIG_END);
			len = eoconfig-soconfig+2;

			struct UART_Buff *Buff_N = malloc(sizeof(struct UART_Buff));

			if (Buff_N != NULL) {
				memset(Buff_N,0,sizeof(struct UART_Buff));
				Buff_N->data = (char*)malloc(len);
				if (Buff_N->data != NULL) {
					memset(Buff_N->data,0,len);
					strncpy(Buff_N->data, soconfig, len);
					if (UART_Buff_H == NULL) {
						UART_Buff_H = Buff_N;
						UART_Buff_T = Buff_N;
					}
					else {
						UART_Buff_T->next = Buff_N;
						UART_Buff_T = Buff_N;
					}
					soconfig = eoconfig + 1;
				}
				else{
					free(Buff_N);
					break;
				}

			}
			else{
				break;
			}
		}
		memset(UART_RxBuff,0,DPS_MAX_CONFIG_SIZE);
		UART_RxState = UART_IDLE;
		UART_RxIndex = 0;
	}

	if (UART_Buff_H != NULL) {
		UART_Buff_N = UART_Buff_H;
		UART_Buff_H = UART_Buff_H->next;

		if ((strstr(UART_Buff_N->data, DPS_ModuleIMEI) != NULL) | (strlen(DPS_ModuleIMEI) == 0)) {
			if (strstr(UART_Buff_N->data, "HEARTBEAT") != NULL) {
				DPS_IsModuleAlive = ONE_MIN;
				DPS_ConfigSystem(UART_Buff_N->data);
			}
			else if ((strstr(UART_Buff_N->data, DPS_CONFIG_SET) != NULL)
					|| (strstr(UART_Buff_N->data, DPS_CONFIG_GET) != NULL)
					|| (strstr(UART_Buff_N->data, DPS_CONFIG_CMD) != NULL)) {
				SYS_State = SYS_Config;
				strcpy(DPS_TempConfig,UART_Buff_N->data);
				Config_ReconfigReq = 1;
			}
		}

		free(UART_Buff_N->data);
		free(UART_Buff_N);
		UART_Buff_N = NULL;
		UART_Buff_N->data = NULL;

		if (UART_Buff_H == NULL) {
			UART_Buff_T = NULL;
		}

	}

}
