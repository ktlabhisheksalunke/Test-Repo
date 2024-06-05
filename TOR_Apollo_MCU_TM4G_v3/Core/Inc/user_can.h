/*
 * PGN Code
 */

/*******************************************************************************
* Title                 :   CAN peripheral Interface Header
* Filename              :   user_can.c
* Author                :   VEDANT JOSHI
* Origin Date           :   1/6/2023
* Version               :   1.0.0
* Compiler              :
* Target                :   STM32L433 - TorMini
* Notes                 :   None
*
* Copyright (c) by KloudQ Technologies Limited.

  This software is copyrighted by and is the sole property of KloudQ
  Technologies Limited.
  All rights, title, ownership, or other interests in the software remain the
  property of  KloudQ Technologies Limited. This software may only be used in
  accordance with the corresponding license agreement. Any unauthorized use,
  duplication, transmission, distribution, or disclosure of this software is
  expressly forbidden.

  This Copyright notice may not be removed or modified without prior written
  consent of KloudQ Technologies Limited.

  KloudQ Technologies Limited reserves the right to modify this software
  without notice.
*
*
*******************************************************************************/
/*************** FILE REVISION LOG *****************************************
*
*    Date    Version   Author         	  Description
*  01/06/20   1.0.0    VEDANT JOSHI   Initial Release.
*
*******************************************************************************/

/** @file  user_can.h
 *  @brief Utilities for CAN interface
 */

#ifndef INC_USER_CAN_H_
#define INC_USER_CAN_H_

#include "main.h"
#include "string.h"
#include <stdlib.h>

/******************************************************************************
* Includes
*******************************************************************************/
/******************************************************************************
* Module Preprocessor Constants
*******************************************************************************/
/**
 * \brief Defines Max CAN Ids supported \a .
   \param None.
   \returns None \a .
 */

#define MAX_CAN_IDS_SUPPORTED (75)

#define MAX_Tx_CAN_IDS_SUPPORTED (35)
/**
 * \brief CAN buffer array length \a .
   \param None.
   \returns None \a .
 */
#define CAN_BUFFER_LENGTH     (200)


#define MAX_DATA_LENGTH 8

/******************************************************************************
* Module Preprocessor Macros
*******************************************************************************/

/******************************************************************************
* Module Typedefs
*******************************************************************************/
/**
 * This union stores Command IDs and its parsed info.
 * This is used to filter PGNs from incomming IDs
 */
typedef union{
	struct{
		uint8_t u16J1939SA : 8;		// bit 0 to 8
		uint32_t u16J1939PGN : 16;  // bit 9 to 24
		uint8_t u16J1939DataPage:1; // bit 25
		uint8_t u16J1939Reserved:1; // bit 26
		uint8_t u16J1939Priority:3; // bit 27,28,29
	};
	uint32_t u32J1939CommandId;
}unCan1939CommandId;

/**
 * This structure stores Command Msgs bytes.
 *
 */
typedef struct
{
	uint8_t u8CanMsgByte7;
	uint8_t u8CanMsgByte6;
	uint8_t u8CanMsgByte5;
	uint8_t u8CanMsgByte4;
	uint8_t u8CanMsgByte3;
	uint8_t u8CanMsgByte2;
	uint8_t u8CanMsgByte1;
	uint8_t u8CanMsgByte0;
}strCanReceivedMsg;

typedef enum
{
	enmCANQUERY_IDLE = 0,
	enmCANQUERY_UPDATEQUERY,
	enmCANQUERY_SENDQUERY,
	enmCANQUERY_AWAITRESPONSE,
	enmCANQUERY_PASRERESPONSE,
	enmCANQUERY_RESPONSETIMEOUT
}enmCanQueryState;


typedef struct {
	uint32_t gu32TxCANTime;
    uint32_t gu32TxCANId;
    int dataLength;
    unsigned char data[MAX_DATA_LENGTH];
} RequestedCANID_t;

typedef struct {

    uint32_t period;
    uint32_t value;

} RequestedCANIDTimer_t;

typedef struct {

    uint8_t arryDTC[4];

} strctDTC;



typedef struct {

	uint32_t spn ;
	uint32_t fmi ;
	uint8_t conversionModeAndOccurrence;
	uint8_t conversionMode ;
	uint8_t occurrence;

} strctDTC_2;

/******************************************************************************
* Module Variable Definitions
*******************************************************************************/

/******************************************************************************
* Function Prototypes
*******************************************************************************/
#ifdef __cplusplus
extern "C"{
#endif

void CAN_Init();
void CAN_FilterConfig(void);
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan);
uint32_t isCommandIdConfigured(uint32_t canId);
void parseCanMessageQueue(void);
void parsePGNCanMessageQueue(void);

void updateCANQuery(uint8_t canTxqueryCount);
void sendMessageCAN (void);
void executeCANQueries(void);
void CAN_Routine(void);

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* INC_USER_CAN_H_ */
