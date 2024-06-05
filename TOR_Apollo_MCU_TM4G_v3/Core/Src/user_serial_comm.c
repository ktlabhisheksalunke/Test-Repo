/*******************************************************************************
* Title                 :   Serial Communication (UART / USART) Interface
* Filename              :   serial_communication.c
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
*    Date    Version     Author           Description
*  14/03/23   1.0.0    VEDANT JOSHI      Initial Release.
*
*******************************************************************************/

/** @file  serial_communication.c
 *  @brief Utilities for serial communication interface
 */

/******************************************************************************
* Includes
*******************************************************************************/
//#include "applicationdefines.h"
#include "main.h"
#include "stm32l4xx_hal.h"
#include <string.h>
#include <stdlib.h>
#include "user_modbus_rs485.h"
#include "user_Timer.h"
#include "user_serial_comm.h"
#include "externs.h"

/******************************************************************************
 Function Name: MODBUS_CharReception_Callback
 Purpose: Byte Received Interrupt (Modbus slave)
 Input:	None
 Return value: None.

	Note(s)(if any)
	-> Called from ISR
	-> USART2

 Change History:
 Author           	Date                Remarks
 KloudQ Team       11-04-18
******************************************************************************/

volatile uint8_t  u8receivedChar;


void MODBUS_CharReception_Callback(void)
{
	u8receivedChar =LL_USART_ReceiveData8(USART2);
	/*Only for debug*/

	if(gu8MBResponseFlag == 0){

		if((st_DeviceConfig.stMbMasterQuerysArr[u8MBQueryRegisterAddressCounter].u8MbSlaveID == u8receivedChar)||(st_DeviceConfig.stMbMasterMultiWriteQuerysArr[u8MBWrQueryRegisterAddressCounter].u8MbSlaveID == u8receivedChar))
	 {
		  master.u8SlaveResponseArray[master.u8MBResponseCharacterCounter++] = u8receivedChar;
			/* Response from slave is received */
			gu16ModbusFrameEndTimer = TWO_SEC;
			gu8MBResponseFlag = 1;

		}
	}
	else{

		gu16ModbusFrameEndTimer = ONEHUNDRED_MS;//ONE_SEC;
		master.u8SlaveResponseArray[master.u8MBResponseCharacterCounter++] = u8receivedChar;

	}
}

/******************************************************************************
 Function Name: UART2_Init(enum_UART_BAUD_RATE enmMbBaudrate, enum_UART_PARITY enumMbParity, enum_UART_STOP_BIT enumMbStopBit)
 Purpose: Byte Received Interrupt (Modbus slave)
 Input:	None
 Return value: None.

	Note(s)(if any)
	-> Called from ISR
	-> USART2

 Change History:
 Author           	Date                Remarks
 KloudQ Team       11-04-18
******************************************************************************/

/* USART2 init function */

void UART2_Init(enum_UART_BAUD_RATE enmMbBaudrate, enum_UART_PARITY enumMbParity, enum_UART_STOP_BIT enumMbStopBit)
{

	  LL_USART_InitTypeDef USART_InitStruct = {0};

	  LL_GPIO_InitTypeDef GPIO_InitStruct = {0};
	  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

	  /** Initializes the peripherals clock
	   */
	   PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART2;
	   PeriphClkInit.Usart2ClockSelection = RCC_USART2CLKSOURCE_PCLK1;
	   if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
	   {
	     Error_Handler();
	   }

	   /* Peripheral clock enable */
	   LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_USART2);

	   LL_AHB2_GRP1_EnableClock(LL_AHB2_GRP1_PERIPH_GPIOA);
	   /**USART2 GPIO Configuration
	   PA2   ------> USART2_TX
	   PA3   ------> USART2_RX
	   */
	   GPIO_InitStruct.Pin = LL_GPIO_PIN_2|LL_GPIO_PIN_3;
	   GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
	   GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;
	   GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
	   GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
	   GPIO_InitStruct.Alternate = LL_GPIO_AF_7;
	   LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	   /* USART2 interrupt Init */
	   NVIC_SetPriority(USART2_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),0, 0));
	   NVIC_EnableIRQ(USART2_IRQn);

	  /* USER CODE BEGIN USART2_Init 1 */

	  /* USER CODE END USART2_Init 1 */


	  /* USER CODE BEGIN USART2_Init 1 */

	  /* USER CODE END USART2_Init 1 */

	  switch(enmMbBaudrate)
	  {

			case enum_BAUD_RATE_2400:
				USART_InitStruct.BaudRate = 2400;
			break;

		  case enum_BAUD_RATE_4800:
			  USART_InitStruct.BaudRate = 4800;
		  break;

			 case enum_BAUD_RATE_9600:
				  USART_InitStruct.BaudRate = 9600;
			  break;

			 case enum_BAUD_RATE_14400:
				  USART_InitStruct.BaudRate = 14400;
			  break;

		 case enum_BAUD_RATE_19200:
			  USART_InitStruct.BaudRate = 19200;
		  break;

		 case enum_BAUD_RATE_28800:
			  USART_InitStruct.BaudRate = 28800;
		  break;

		 case enum_BAUD_RATE_33600:
			  USART_InitStruct.BaudRate = 33600;
		  break;

		 case enum_BAUD_RATE_38400:
			  USART_InitStruct.BaudRate = 38400;
		  break;

		 case enum_BAUD_RATE_57600:
			  USART_InitStruct.BaudRate = 57600;
		  break;

		 case enum_BAUD_RATE_115200:
			  USART_InitStruct.BaudRate = 115200;
		  break;

		 case enum_BAUD_RATE_230400:
			  USART_InitStruct.BaudRate = 230400;
		  break;

		 case enum_BAUD_RATE_460800:
			  USART_InitStruct.BaudRate = 460800;
		  break;

		 case enum_BAUD_RATE_921600:
			  USART_InitStruct.BaudRate = 921600;
		  break;

		 case enum_BAUD_RATE_1000000:
			  USART_InitStruct.BaudRate = 1000000;
		  break;

		 default://If selection is invalid then default baud rate is 9600
			  USART_InitStruct.BaudRate = 115200;
		  break;
	  }


	  switch(enumMbParity){

		 case enum_USART_PARITY_ODD:
				USART_InitStruct.Parity = LL_USART_PARITY_ODD;
//			USART_InitStruct.DataWidth = LL_USART_DATAWIDTH_9B;
		  break;

		 case enum_USART_PARITY_EVEN:
				USART_InitStruct.Parity = LL_USART_PARITY_EVEN;
//			USART_InitStruct.DataWidth = LL_USART_DATAWIDTH_9B;
		  break;

		 case enum_USART_PARITY_NONE:
				USART_InitStruct.Parity = LL_USART_PARITY_NONE;
//			USART_InitStruct.DataWidth = LL_USART_DATAWIDTH_8B;
			  break;

		 default:
				USART_InitStruct.Parity = LL_USART_PARITY_NONE;
//			USART_InitStruct.DataWidth = LL_USART_DATAWIDTH_8B;
			  break;
	   }

	  switch(enumMbStopBit){

			case enum_USART__STOPBITS_2:
				USART_InitStruct.StopBits = LL_USART_STOPBITS_2;
			break;

			case enum_USART_STOPBITS_1:
				USART_InitStruct.StopBits = LL_USART_STOPBITS_1;
			break;

			default:
				USART_InitStruct.StopBits = LL_USART_STOPBITS_1;
			break;
	   }

	  USART_InitStruct.DataWidth = LL_USART_DATAWIDTH_8B;
	  USART_InitStruct.TransferDirection = LL_USART_DIRECTION_TX_RX;
	  USART_InitStruct.HardwareFlowControl = LL_USART_HWCONTROL_NONE;
	  USART_InitStruct.OverSampling = LL_USART_OVERSAMPLING_16;
	  LL_USART_Init(USART2, &USART_InitStruct);
	  LL_USART_DisableOverrunDetect(USART2);
	  LL_USART_ConfigAsyncMode(USART2);
	  LL_USART_Enable(USART2);
	  /* USER CODE BEGIN USART2_Init 2 */

	  /* USER CODE END USART2_Init 2 */

}
