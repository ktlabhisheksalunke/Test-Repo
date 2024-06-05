/*******************************************************************************
* Title                 :   Serial Communication Handling
* Filename              :   serial_communication.h
* Author                :   VEDANT JOSHI
* Origin Date           :   08/03/2022
* Version               :   1.0.0
* Compiler              :
* Target                :   STM32F437 - Tor4Eth
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
*  08/03/22   1.0.0    VEDANT JOSHI   Initial Release.
*
*******************************************************************************/

/** @file  serial_communication.h
 *  @brief Application Serial Communication Handling Functions
 */


/******************************************************************************
* Includes
*******************************************************************************/

#ifndef INC_USER_SERIAL_COMM_H_
#define INC_USER_SERIAL_COMM_H_

/******************************************************************************
* Module Preprocessor Constants
*******************************************************************************/

/******************************************************************************
* Module Preprocessor Macros
*******************************************************************************/

/******************************************************************************
* Module Typedefs
*******************************************************************************/

/******************************************************************************
* Module Variables
*******************************************************************************/

/******************************************************************************
* Module Preprocessor Macros
*******************************************************************************/

/******************************************************************************
* Module Typedefs
*******************************************************************************/
/******************************************************************************
* Module Variable Definitions
*******************************************************************************/

/******************************************************************************
* Function Prototypes
*******************************************************************************/

#ifdef __cplusplus
extern "C"{
#endif

extern uint8_t gu8MBResponseFlag;
//void GSM_CharReception_Callback(void);
void UART2_Init(enum_UART_BAUD_RATE enmMbBaudrate, enum_UART_PARITY enumMbParity, enum_UART_STOP_BIT enumMbStopBit);

#ifdef __cplusplus
} // extern "C"
#endif

//***************************************** End of File ********************************************************



#endif /* INC_USER_SERIAL_COMM_H_ */
