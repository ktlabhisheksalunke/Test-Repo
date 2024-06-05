/*
  *****************************************************************************
  * @file    modbusmaster.c
  * @author  KloudQ Team
  * @version
  * @date
  * @brief  Modbus Master Functions
*******************************************************************************
*/
/******************************************************************************

            Copyright (c) by KloudQ Technologies Limited.

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

  KloudQ Technologies Limited
------------------------------------------------------------------------------
  Note : MODBUS Timing Calculations
------------------------------------------------------------------------------
Modbus states that a baud rate higher than 19200 must use a fixed 750 us
for inter character time out and 1.75 ms for a frame delay.
For baud rates below 19200 the timeing is more critical and has to be calculated.
E.g. 9600 baud in a 10 bit packet is 960 characters per second
In milliseconds this will be 960characters per 1000ms. So for 1 character
1000ms/960characters is 1.04167ms per character and finaly modbus states an
intercharacter must be 1.5T or 1.5 times longer than a normal character and thus
 1.5T = 1.04167ms * 1.5 = 1.5625ms. A frame delay is 3.5T.

We have implemented 100us timer Interrupt

if (baud > 19200)
{
    T1_5 = 8;
    T3_5 = 16;
}
else
{
    T1_5 = 15000000/baud;
    T3_5 = 35000000/baud;
}


------------------------------------------------------------------------------
How is data stored in Standard Modbus ?
------------------------------------------------------------------------------

Information is stored in the Slave device in four different tables.
Two tables store on/off discrete values (coils) and two store numerical values (registers).
The coils and registers each have a read-only table and read-write table.

Each table has 9999 values.
Each coil or contact is 1 bit and assigned a data address between 0000 and 270E.
Each register is 1 word = 16 bits = 2 bytes and also has data address between 0000 and 270E.

Coil/Register Numbers

Data Addresses      Type			            Table Name
1-9999  		0000 to 270E   Read-Write	Discrete Output Coils
10001-19999     0000 to 270E   Read-Only	Discrete Input Contacts
30001-39999     0000 to 270E   Read-Only	Analog Input Registers
40001-49999     0000 to 270E   Read-Write	Analog Output Holding Registers

Coil/Register Numbers can be thought of as location names since they do not appear in
the actual messages.The Data Addresses are used in the messages.

For example, the first Holding Register, number 40001, has the Data Address 0000.
The difference between these two values is the offset.
Each table has a different offset. 1, 10001, 30001 and 40001.


What are extended register addresses?

Since the range of the analog output holding registers is 40001 to 49999,
it implies that there cannot be more than 9999 registers.
Although this is usually enough for most applications, there are cases where more registers
would be beneficial.

Registers 40001 to 49999 correspond to data addresses 0000 to 270E.
If we utilize the remaining data addresses 270F to FFFF, over six times as many registers
can be available,65536 in total.This would correspond to register numbers from 40001 to 105536.

Many modbus software drivers (for Master PCs) were written with the 40001 to 49999 limits
and cannot access extended registers in slave devices. And many slave devices do not support
maps using the extended registers. But on the other hand, some slave devices do support these
registers and some Master software can access it, especially if custom software is written.


-------------------------------------------------------------------------------
Limitations of Modbus
-------------------------------------------------------------------------------

Since Modbus was designed in the late 1970s to communicate to programmable logic controllers, the number of
data types is limited to those understood by PLCs at the time. Large binary objects are not
supported.
No standard way exists for a node to find the description of a data object, for example,
to determine whether a register value represents a temperature between 30 and 175 degrees.
Since Modbus is a master/slave protocol, there is no way for a field device to "report by
exception" (except over Ethernet TCP/IP, called open-mbus) � the master node must routinely
poll each field device and look for changes in the data. This consumes bandwidth and network
time in applications where bandwidth may be expensive, such as over a low-bit-rate radio link.

Modbus is restricted to addressing 254 devices on one data link, which limits the number of
field devices that may be connected to a master station (once again, Ethernet TCP/IP being
an exception).
Modbus transmissions must be contiguous, which limits the types of remote communications
devices to those that can buffer data to avoid gaps in the transmission.
Modbus protocol itself provides no security against unauthorized commands or interception
of data.
-------------------------------------------------------------------------------
*******************************************************************************
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "externs.h"
#include "main.h"
#include "stm32l4xx_hal.h"
//#include "tim.h"
#include "externs.h"
#include "user_modbus_rs485.h"
#include "payload.h"
#include "user_Timer.h"
#include "user_UART.h"
#include "user_serial_comm.h"
#include <user_config.h>
#include "user_ApplicationDefines.h"

char gu8MBRTUPayloadString[1100] = {'0'};
char tempunsignedRTUPaylaod[1024] = {'0'};
char binaryString[64];
int Exception_code;
uint8_t TempBuff[8];
char TempMbPayloadBuff[35] = {'0'};
char TempMbPayloadBuff1[1000] = {'0'};
uint8_t MBWritePayload = 0;

uint8_t u8MBQueryCharacterCounter;
uint8_t gu8MBResponseFlag = 0;
uint8_t u8MBQueryRegisterAddressCounter = 0;
uint8_t u8MBWrQueryRegisterAddressCounter = 0;
int lengthtocallcrc = 0;

uint32_t ModbusBytesReceived = 0;
volatile uint32_t gu16ModbusFrameEndTimer = 0;
volatile uint32_t gu32ModbusResponseTimeout = 0;
volatile uint32_t gu32ModbusPollDelay = 0;
volatile uint32_t gu32ModbusCycelRestartTmr = 0;

enmMODBUSFSMState modbusState = enmMODBUS_IDLE;
ST_Config st_DeviceConfig = {0};
ModbusData ModbusDataReceived;
strctModbusMaster master;

void UpdateModbusQueryFrame(St_MBMasterQueryData *MbMasterQueryData);
void UpdateModbusMultiWrQueryframe(St_MBMasterMultiWriteQueryData *MbMasterQueryData);
static void MB_FillSlaveRawData(uint16_t u16QueryCntr, uint16_t u16NumberofBytesReceived);

/****************************************************************************
 Function modbusCRC16
 Purpose: Calculate MODBUS CRC
 Input:	uint8_t * data ,uint8_t length
 Return value: None.


 Note(s)(if-any) : Low and High bytes are swapped

 Change History:
 Author           	Date                Remarks
 KloudQ Team      11-04-18
******************************************************************************/
uint16_t modbusCRC16(uint8_t * data , uint8_t length)
{
	uint16_t crc = 0xFFFF;

	  for (uint16_t pos = 0; pos < length; pos++)
	  {
	    crc ^= (uint16_t)data[pos];         // XOR byte into least sig. byte of crc

	    for (int i = 8; i != 0; i--)  		// Loop over each bit
	    {
	      if ((crc & 0x0001) != 0) 			// If the LSB is set
	      {
	        crc >>= 1;                    	// Shift right and XOR 0xA001
	        crc ^= 0xA001;
	      }
	      else                            	// Else LSB is not set
	        crc >>= 1;                    	// Just shift right
	    }
	  }
	  return crc;
}

/****************************************************************************
 Function UpdateModbusQueryFrame
 Purpose: Update Modbus Query Frame
 Input:	strctModbusMaster * master
 Return value: None.


 Note : Logic Written Assuming Correctness of the Modbus Register Address is
 	 	validated at configuration software.


 Change History:
 Author           	Date                Remarks
 KloudQ Team 	   	11-04-18
 kloudQ Team	   	22-10-18			Update . Address Deference added
 Milind V			02-02-19			Updated Modbus query for Terex ONLY. It fetches 4 consecutive locations from register address 6
******************************************************************************/
void UpdateModbusQueryFrame(St_MBMasterQueryData *MbMasterQueryData)
{
	uint16_t crc ;

	/* Form Query Frame String */
	memset(master.u8QueryFrame,0x00,sizeof(master.u8QueryFrame));

	master.u8QueryFrame[0] = MbMasterQueryData->u8MbSlaveID;
	master.u8QueryFrame[1] = MbMasterQueryData->u8MbFunctionCode;
	master.u8QueryFrame[2] = ((MbMasterQueryData->u16MbStartingAddress - MB_ADDRESS_DEREF) >> 8);
	master.u8QueryFrame[3] = ((MbMasterQueryData->u16MbStartingAddress - MB_ADDRESS_DEREF)& 0xFF);
	master.u8QueryFrame[4] = ((MbMasterQueryData->u16MbNoOfPoints)>> 8);
	master.u8QueryFrame[5] = ((MbMasterQueryData->u16MbNoOfPoints) & 0xFF);
	crc = modbusCRC16(master.u8QueryFrame , 6);
	master.u8QueryFrame[6] = crc;
	master.u8QueryFrame[7] = (crc >> 8);
	/* End of Query Frame */
}

void UpdateModbusMultiWrQueryframe(St_MBMasterMultiWriteQueryData *MbMasterMultiWrQueryData)
{
	uint16_t crc ;
	int datanum = 0;
	/* Form Query Frame String */
	memset(master.u8MultiWrQueryframe,0x00,sizeof(master.u8MultiWrQueryframe));

	master.u8MultiWrQueryframe[0] = MbMasterMultiWrQueryData->u8MbSlaveID;
	master.u8MultiWrQueryframe[1] = MbMasterMultiWrQueryData->u8MbFunctionCode;
	master.u8MultiWrQueryframe[2] = ((MbMasterMultiWrQueryData->u16MbStartingAddress - MB_ADDRESS_DEREF) >> 8);
	master.u8MultiWrQueryframe[3] = ((MbMasterMultiWrQueryData->u16MbStartingAddress - MB_ADDRESS_DEREF)& 0xFF);
	master.u8MultiWrQueryframe[4] = ((MbMasterMultiWrQueryData->u16MbLengthtowr)>> 8);
	master.u8MultiWrQueryframe[5] = ((MbMasterMultiWrQueryData->u16MbLengthtowr) & 0xFF);
	master.u8MultiWrQueryframe[6] = MbMasterMultiWrQueryData -> u8MbBytetofollow;
	int actualregisterstowrite = 0;
	for (datanum = 0; datanum < MbMasterMultiWrQueryData -> u8MbBytetofollow;)
	{
	  master.u8MultiWrQueryframe[7 + datanum] = ((MbMasterMultiWrQueryData -> u16MbDatatowr[actualregisterstowrite]) >> 8);//7//9//
		datanum += 1;
		master.u8MultiWrQueryframe[7 + datanum ] = ((MbMasterMultiWrQueryData -> u16MbDatatowr[actualregisterstowrite]) & 0xFF);//8//10//
		datanum += 1;
		actualregisterstowrite++;
	}
	actualregisterstowrite = 0;
  lengthtocallcrc = 6 + datanum + 1 + 2;
	crc = modbusCRC16(master.u8MultiWrQueryframe , (lengthtocallcrc - 2));
	master.u8MultiWrQueryframe[7 + datanum] = crc;
	master.u8MultiWrQueryframe[7 + datanum + 1] = (crc >> 8);
}
	/* End of Query Frame */

/******************************************************************************
 Function Name:ModbusMaster_FSM
 Purpose:  Fetch Data over Modbus
 Input:	None
 Return value: None.

 Note : gu32ModbusPollDelay - As per slave device specification
 Default : 500 ms


 Change History:
 Author           	Date                Remarks
 KloudQ Team      11-04-18
 KloudQ Team 	  22-10-18				Update . Logic Optimised
 	 	 	 	 	 	 	 	 	 	Exception Frame from Slave Parsed
 	 	 	 	 	 	 	 	 	 	Response Status Array Added
 Milind V		  02-02-19				Added code which sets and resets the RE and DE line for Tx and Rx of data in IDLE and SEND cases
 	 	 	 	 	 	 	 	 	 	Modified if else loop in SEND case
 ******************************************************************************/
void ModbusMaster_FSM(void)
	{
//	static uint8_t s_u8TimeoutResponseQueryCntr = 0;
//	static uint8_t u8MBQueryCharacterCounter = 0;
//	uint16_t u16LoopCntr=0;
	if(st_DeviceConfig.u8ModbusMasterEnFlag == 0)
	{
		return;
	}
	switch(modbusState)
	{
		case enmMODBUS_IDLE:

			if(gu32ModbusPollDelay)
			{
				break;
			}

			/* Limit overflow of modbus operation state variable */
			if((master.u8MBOperationStatus != RESET) && (master.u8MBOperationStatus != SET))
			{
				master.u8MBOperationStatus = RESET;
			}

			//TODO remove Added for temporary testing
			master.u8MBOperationStatus = RESET;

			if(master.u8MBOperationStatus == RESET)
			{

				HAL_GPIO_WritePin(GPIOA,MB_DE_Pin,GPIO_PIN_SET);
				HAL_GPIO_WritePin(GPIOA,MB_RE_Pin,GPIO_PIN_SET);
				memset(master.u8SlaveResponseArray, 0x00, sizeof(master.u8SlaveResponseArray));
				master.u8MBResponseCharacterCounter = 0;
				gu8MBResponseFlag = 0;

				if(u8MBQueryRegisterAddressCounter >= (st_DeviceConfig.u16MbTotalNoOfQuerys)){

					u8MBQueryRegisterAddressCounter = 0;
					gu32ModbusCycelRestartTmr = master.u32MbCycleRestartTmr;
					master.u8MBOperationStatus = SET;
					break;
				}
				else
			  {
					UpdateModbusQueryFrame(&(st_DeviceConfig.stMbMasterQuerysArr[u8MBQueryRegisterAddressCounter]));
				}
                u8MBQueryCharacterCounter = 0;

				LL_USART_EnableIT_TXE(USART2);
				LL_USART_EnableIT_TC(USART2);

				LL_USART_TransmitData8(USART2,master.u8QueryFrame[u8MBQueryCharacterCounter]);
				modbusState = enmMODBUS_SEND_QUERY;
			}
			else{

				if(gu32ModbusCycelRestartTmr == 0){

					master.u8MBOperationStatus = RESET;
				}
			}
			break;

		case enmMODBUS_SEND_QUERY:

			if(!LL_USART_IsActiveFlag_TXE(USART2)){

				/*No Nothing . Wait For Previous Character Transmission */
			}
			else{

				if(u8MBQueryCharacterCounter < 8){

					LL_USART_TransmitData8(USART2,master.u8QueryFrame[u8MBQueryCharacterCounter++]);

				}
				else{

					HAL_Delay(1);
					u8MBQueryCharacterCounter = 0;
					master.u8MBResponseCharacterCounter = 0;
					modbusState = enmMODBBUS_AWAIT_RESPONSE;
					HAL_GPIO_WritePin(GPIOA,MB_DE_Pin,GPIO_PIN_RESET); // ~RE -> LOW - Tx disabled
					HAL_GPIO_WritePin(GPIOA,MB_RE_Pin,GPIO_PIN_RESET); // ~RE -> LOW - RCV enabled Tx disabled

					gu32ModbusResponseTimeout = master.u32MbResponseTimeout;
				}
			}
      break;

		case enmMODBBUS_AWAIT_RESPONSE:
		{
				if((gu8MBResponseFlag == 1)&&(gu16ModbusFrameEndTimer == 0))
				{
					/* Response Frame Received . Parse Response */
					modbusState = enmMODBUS_PARSE_RESPONSE;
					gu8MBResponseFlag = 0;

				}
				else if(gu32ModbusResponseTimeout == 0)
				{
					modbusState = enmMODBUS_RESPONSE_TIMEOUT;
					HAL_GPIO_WritePin(GPIOA,MB_RE_Pin,GPIO_PIN_SET);
				}
				else
				{
					//Waiting
				}
		}
		break;

		case enmMODBUS_PARSE_RESPONSE:

			/* Process received response */

			if(master.u8SlaveResponseArray[0] == st_DeviceConfig.stMbMasterQuerysArr[u8MBQueryRegisterAddressCounter].u8MbSlaveID)
			{
			if(master.u8SlaveResponseArray[1] == st_DeviceConfig.stMbMasterQuerysArr[u8MBQueryRegisterAddressCounter].u8MbFunctionCode)
			{
				uint8_t u8TempCRCArray[300];
				/* Function code and slave id is Correct .
				 * Verify CRC and Extract Number of data bytes*/
				memset((char *)u8TempCRCArray, 0, 100);
				u8TempCRCArray[0] = master.u8SlaveResponseArray[0];
				uint8_t u8MBNoDataBytes = (uint8_t)master.u8SlaveResponseArray[2];
				uint32_t TempLoopVar = 0;
				if(u8MBQueryRegisterAddressCounter == 0)
				{
					ModbusBytesReceived = u8MBNoDataBytes;
				}
				else
				{
					ModbusBytesReceived += u8MBNoDataBytes;
				}

				/* Add the data into array for CRC calculations */
				for(TempLoopVar =0; TempLoopVar < (u8MBNoDataBytes + 2); TempLoopVar++){

					u8TempCRCArray[TempLoopVar + 1] = master.u8SlaveResponseArray[TempLoopVar+1];
				}

				uint16_t u16TempCRC = modbusCRC16(u8TempCRCArray,(u8MBNoDataBytes + 3));
				uint8_t u8CRCHi = master.u8SlaveResponseArray[3 + u8MBNoDataBytes];    // CRC High Location
				uint8_t u8CRCLow = master.u8SlaveResponseArray[3 + u8MBNoDataBytes + 1]; // CRC Low Location
				uint16_t u16ReceivedCRC =  (uint16_t)(u8CRCLow<<8)|(uint16_t)u8CRCHi;

				if(u16ReceivedCRC == u16TempCRC)
				{
//					s_u8TimeoutResponseQueryCntr = 0;
//					HAL_GPIO_TogglePin(COMM_LED_uC_GPIO_Port, COMM_LED_uC_Pin);

					/* CRC is Correct */
					/* Query Successful . Attempt Next Query with recommended delay */

					master.u16QueryNCCntrBufff[u8MBQueryRegisterAddressCounter] = 0;
					master.enum_MBResponseStatusBuff[u8MBQueryRegisterAddressCounter] = enm_ResponseSucccess;
					MB_FillSlaveRawData(u8MBQueryRegisterAddressCounter,u8MBNoDataBytes);
					u8MBQueryRegisterAddressCounter++;
					master.u8MBNoQueryAttempts = 0;
					resetModbusPort();
					gu32ModbusPollDelay = master.gu32ModbusPollDelay;
					modbusState = enmMODBUS_IDLE;
					memset(master.u8SlaveResponseArray, 0x00, sizeof(master.u8SlaveResponseArray));
				}
				else
				{
					/* CRC is not valid . Try Again with same query */
					modbusState = enmMODBUS_RETRY_QUERY;
				}
			}
			else if((master.u8SlaveResponseArray[1] & 0x80) == 0x80)
			{
//				s_u8TimeoutResponseQueryCntr = 0;

				/* Exception Function code . Check if MSB is 1 eg : 04 -> 84 .
				 * Extract Modbus exception code
				 * Add Response Status Array and update it with Exception */

				/* Function code is invalid .
				 *
				 * Log Error Code */

				if(master.u16QueryNCCntrBufff[u8MBQueryRegisterAddressCounter]>3)
				{
										master.u16QueryNCCntrBufff[u8MBQueryRegisterAddressCounter]=0;
										Exception_code = master.u8SlaveResponseArray[2];
										master.enum_MBResponseStatusBuff[u8MBQueryRegisterAddressCounter] = Exception_code;//enm_ResponseException;  /*1 - Exception 0 - Success in query*/
										MB_FillSlaveRawData(u8MBQueryRegisterAddressCounter,0);
				}
				u8MBQueryRegisterAddressCounter++;
				resetModbusPort();
				gu32ModbusPollDelay = master.gu32ModbusPollDelay;
				modbusState = enmMODBUS_IDLE;
			}
			else
			{
				modbusState = enmMODBUS_RETRY_QUERY;
			}
			}
			else
			{
				modbusState = enmMODBUS_RETRY_QUERY;
			}

			break;

		case enmMODBUS_RETRY_QUERY:
					memset(master.u8SlaveResponseArray, 0x00, sizeof(master.u8SlaveResponseArray));
					if(master.u8MBNoQueryAttempts > MB_QUERY_RETRY_ATTEMPTS)
				  {
						/* Max Retry Attempt Reached . Log Error and fetch Next Address */

		//				master.u16QueryNCCntrBufff[u8MBQueryRegisterAddressCounter]++;

		//				if(master.u16QueryNCCntrBufff[u8MBQueryRegisterAddressCounter]>4)
							{
								master.enum_MBResponseStatusBuff[u8MBQueryRegisterAddressCounter] = enm_ResponseCRCInvalid;
								MB_FillSlaveRawData(u8MBQueryRegisterAddressCounter,0);
						  }
							master.u8MBNoQueryAttempts = 0;
							u8MBQueryRegisterAddressCounter++;
					}
					else
					{
						/* Retry Same Register Address Query */
						master.u8MBNoQueryAttempts++;
					}

					resetModbusPort();
					gu32ModbusPollDelay = master.gu32ModbusPollDelay;
					modbusState = enmMODBUS_IDLE;
					break;

		case enmMODBUS_RESPONSE_TIMEOUT:
					if(master.u8MBNoQueryAttempts > MB_QUERY_RETRY_ATTEMPTS)
					{
						master.u8MBNoQueryAttempts++;
					}
					else
					{
						{
							master.enum_MBResponseStatusBuff[u8MBQueryRegisterAddressCounter] = enm_ResponseTimeOut;
							MB_FillSlaveRawData(u8MBQueryRegisterAddressCounter,0);
						}
						master.u8MBNoQueryAttempts = 0;
						u8MBQueryRegisterAddressCounter++;
						gu32ModbusPollDelay = FIVEHUNDRED_MS;/*500 ms For enmMODBUS_IDLE state */
          }
					resetModbusPort();
					modbusState = enmMODBUS_IDLE;
					break;
	}
}

void ModbusMasterMultiWrite_FSM(void)
	{
//	static uint8_t s_u8TimeoutResponseQueryCntr = 0;
	static uint8_t u8MBMultiWrQueryCharacterCounter = 0;
//	uint16_t u16LoopCntr=0;
	if(st_DeviceConfig.u8ModbusMasterEnFlag == 0)
	{
		return;
	}
	switch(modbusState)
	{
	case enmMODBUS_IDLE:

		if(gu32ModbusPollDelay)
		{
			break;
		}

		/* Limit overflow of modbus operation state variable */
		if((master.u8MBOperationStatus != RESET) && (master.u8MBOperationStatus != SET))
		{
			master.u8MBOperationStatus = RESET;
		}

		//TODO remove Added for temporary testing
//		master.u8MBOperationStatus = RESET;

		if(master.u8MBOperationStatus == RESET)
		{
			HAL_GPIO_WritePin(GPIOA,MB_DE_Pin,GPIO_PIN_SET);
			HAL_GPIO_WritePin(GPIOA,MB_RE_Pin,GPIO_PIN_SET);
			memset(master.u8SlaveResponseArray, 0x00, sizeof(master.u8SlaveResponseArray));
			master.u8MBResponseCharacterCounter = 0;
			gu8MBResponseFlag = 0;
		  if(SinglewriteFlag == 1)
		  {
					if(u8MBQueryRegisterAddressCounter >= (st_DeviceConfig.u16MbTotalNoOfQuerys))
					{
							u8MBQueryRegisterAddressCounter = 0;
							gu32ModbusCycelRestartTmr = master.u32MbCycleRestartTmr;
							master.u8MBOperationStatus = SET;
							MBWritePayload = 1;
							Queue_Enequeue(&DPS_LiveQueue,(char *)DPS_MBWritePayload());
				      memset(TempMbPayloadBuff1, 0, sizeof(TempMbPayloadBuff1));
					    MBWritePayload =0;
					    Config_Reload = 1;
//							Config_MBWriteCmd = 0;
							Config_Get(DPS_CONFIG_MB, DPS_ConfigStr.DPS_MBConfig, DPS_MAX_CONFIG_SIZE);
							break;
					}
					else
					{
							UpdateModbusQueryFrame(&(st_DeviceConfig.stMbMasterQuerysArr[u8MBQueryRegisterAddressCounter]));
					}
					LL_USART_TransmitData8(USART2,master.u8QueryFrame[u8MBQueryCharacterCounter++]);
					modbusState = enmMODBUS_SEND_QUERY;
		 }
		else if(MultiwriteFlag == 1)
		{
					if(u8MBWrQueryRegisterAddressCounter >= (st_DeviceConfig.u16MbTotalNoOfWrQuerys))
				  {
							u8MBWrQueryRegisterAddressCounter = 0;
							gu32ModbusCycelRestartTmr = master.u32MbCycleRestartTmr;
							master.u8MBOperationStatus = SET;
							MBWritePayload = 1;
							Queue_Enequeue(&DPS_LiveQueue,(char *)DPS_MBWritePayload());
				      memset(TempMbPayloadBuff1, 0, sizeof(TempMbPayloadBuff1));
					    MBWritePayload =0;
					  	Config_Reload = 1;
//					  	Config_MBWriteCmd = 0;
							Config_Get(DPS_CONFIG_MB, DPS_ConfigStr.DPS_MBConfig, DPS_MAX_CONFIG_SIZE);

				  }
				  else
					{
							UpdateModbusMultiWrQueryframe(&(st_DeviceConfig.stMbMasterMultiWriteQuerysArr[u8MBWrQueryRegisterAddressCounter]));
				  }

				LL_USART_TransmitData8(USART2,master.u8MultiWrQueryframe[u8MBMultiWrQueryCharacterCounter++]);
				modbusState = enmMODBUS_SEND_QUERY;
		}
		else
	  {

			if(gu32ModbusCycelRestartTmr == 0)
			{
				master.u8MBOperationStatus = RESET;
			}
		}
	}

	break;

	case enmMODBUS_SEND_QUERY:

		if(!LL_USART_IsActiveFlag_TXE(USART2)){

			/*No Nothing . Wait For Previous Character Transmission */
		}
		else
		{
		  if(SinglewriteFlag == 1)
		  {
					if(u8MBQueryCharacterCounter < 8)
					{
							LL_USART_TransmitData8(USART2,master.u8QueryFrame[u8MBQueryCharacterCounter++]);
					}
					else
					{
							HAL_Delay(1);
							u8MBQueryCharacterCounter = 0;
							master.u8MBResponseCharacterCounter = 0;
							modbusState = enmMODBBUS_AWAIT_RESPONSE;
							HAL_GPIO_WritePin(GPIOA,MB_DE_Pin,GPIO_PIN_RESET); // ~RE -> LOW - Tx disabled
							HAL_GPIO_WritePin(GPIOA,MB_RE_Pin,GPIO_PIN_RESET); // ~RE -> LOW - RCV enabled Tx disabled
							gu32ModbusResponseTimeout = master.u32MbResponseTimeout;
					}
		  }
		  if(MultiwriteFlag == 1)
		  {
					if(u8MBMultiWrQueryCharacterCounter < lengthtocallcrc)
					{
							LL_USART_TransmitData8(USART2,master.u8MultiWrQueryframe[u8MBMultiWrQueryCharacterCounter++]);
					}
					else
					{
							HAL_Delay(1);
							u8MBMultiWrQueryCharacterCounter = 0;
							master.u8MBResponseCharacterCounter = 0;
							modbusState = enmMODBBUS_AWAIT_RESPONSE;
							HAL_GPIO_WritePin(GPIOA,MB_DE_Pin,GPIO_PIN_RESET); // ~RE -> LOW - Tx disabled
							HAL_GPIO_WritePin(GPIOA,MB_RE_Pin,GPIO_PIN_RESET); // ~RE -> LOW - RCV enabled Tx disabled
							gu32ModbusResponseTimeout = master.u32MbResponseTimeout;
					}
		  }
		}
		break;

		case enmMODBBUS_AWAIT_RESPONSE:

			if((gu8MBResponseFlag == 1)&&(gu16ModbusFrameEndTimer == 0)){

				/* Check for EOF */

				if(gu16ModbusFrameEndTimer == 0){

					/* Response Frame Received . Parse Response */
					modbusState = enmMODBUS_PARSE_RESPONSE;
					gu8MBResponseFlag = 0;

				}
				else if(gu32ModbusResponseTimeout == 0){ /* Response Timeout */ //Added by Anil More.

					modbusState = enmMODBUS_RESPONSE_TIMEOUT;
				}
			}
			else{

				/* Response Timeout */
				if(gu32ModbusResponseTimeout == 0){

					modbusState = enmMODBUS_RESPONSE_TIMEOUT;
				}

			}
			break;
		case enmMODBUS_PARSE_RESPONSE:

		     if(SinglewriteFlag == 1)
			{
			/* Process received response */
			if(master.u8SlaveResponseArray[0] == st_DeviceConfig.stMbMasterQuerysArr[u8MBQueryRegisterAddressCounter].u8MbSlaveID)
			{
			if(master.u8SlaveResponseArray[1] == st_DeviceConfig.stMbMasterQuerysArr[u8MBQueryRegisterAddressCounter].u8MbFunctionCode)
			{
			  if(master.u8SlaveResponseArray[1] == 5 || master.u8SlaveResponseArray[1] == 6)
			  {
			  uint8_t u8TempCRCArray[300];
				/* Function code and slave id is Correct */
				memset((char *)u8TempCRCArray, 0, 100);
				if (memcmp(master.u8SlaveResponseArray, master.u8QueryFrame, sizeof(master.u8QueryFrame)) == 0)
				{
				  memset(TempMbPayloadBuff, 0, sizeof(TempMbPayloadBuff));
					for(int u16LoopCntr=0;u16LoopCntr < 8;u16LoopCntr++)
					{
						memset(TempBuff, 0, sizeof(TempBuff));
						sprintf((char * )TempBuff,"%02d,",(int)master.u8SlaveResponseArray[u16LoopCntr]);
						strcat((char * )TempMbPayloadBuff,(char * )TempBuff);

					}

			    strcat((char * )TempMbPayloadBuff1,(char * )TempMbPayloadBuff);
//			    Queue_Enequeue(&DPS_LiveQueue,(char *)DPS_GenDiagnostic(enmDiagnostic_MB_WRITE_QUERY_SUCCESSFUL));
//			  	MBWritePayload = 1;
			  	u8MBQueryRegisterAddressCounter++;
					master.u8MBNoQueryAttempts = 0;
					resetModbusPort();
					gu32ModbusPollDelay = master.gu32ModbusPollDelay;
					modbusState = enmMODBUS_IDLE;
					break;
				}
				else
				{
					/*Write query unsuccessfull*/
					modbusState = enmMODBUS_RETRY_QUERY;
				}
			}

			else
			{
			    /*FC not Matched*/
				modbusState = enmMODBUS_RETRY_QUERY;
			}
			}

			else
			{
			    /*Slave Id not Matched*/
				modbusState = enmMODBUS_RETRY_QUERY;
			}
	    }
			}
		     else if(MultiwriteFlag == 1)
					{

					if(master.u8SlaveResponseArray[0] == st_DeviceConfig.stMbMasterMultiWriteQuerysArr[u8MBWrQueryRegisterAddressCounter].u8MbSlaveID)// slave id
					{
					  uint8_t u8TempCRCArray[300];
					  memset((char *)u8TempCRCArray, 0, 100);
						if(master.u8SlaveResponseArray[1] == st_DeviceConfig.stMbMasterMultiWriteQuerysArr[u8MBWrQueryRegisterAddressCounter].u8MbFunctionCode)//function code
						{
							if(master.u8SlaveResponseArray[1] == 15 || master.u8SlaveResponseArray[1] == 16)
							{
								if(st_DeviceConfig.stMbMasterMultiWriteQuerysArr[u8MBWrQueryRegisterAddressCounter].u16MbStartingAddress == ((master.u8SlaveResponseArray[2] << 8)| (master.u8SlaveResponseArray[3])))
								{
									if(st_DeviceConfig.stMbMasterMultiWriteQuerysArr[u8MBWrQueryRegisterAddressCounter].u16MbLengthtowr == ((master.u8SlaveResponseArray[4] << 8)| (master.u8SlaveResponseArray[5])))
									{
									uint8_t CRCCheck[8] = {0};
									for(int LoopVar =0; LoopVar < 6; LoopVar++)
									{
										CRCCheck[LoopVar] = master.u8SlaveResponseArray[LoopVar];

									}
									uint16_t CalculatedCRC ;
									CalculatedCRC = modbusCRC16(CRCCheck ,6);
									int receivedCRC = 0;
									receivedCRC =  (uint16_t)(master.u8SlaveResponseArray[7]<<8)|(uint16_t)master.u8SlaveResponseArray[6];
									if(CalculatedCRC == receivedCRC)
									{
									  memset(TempMbPayloadBuff, 0, sizeof(TempMbPayloadBuff));
										for(int u16LoopCntr=0;u16LoopCntr < 8;u16LoopCntr++)
										{
										  memset(TempBuff, 0, sizeof(TempBuff));
											sprintf((char * )TempBuff,"%d,",(int)master.u8SlaveResponseArray[u16LoopCntr]);
											strcat((char * )TempMbPayloadBuff,(char * )TempBuff);
										}
//      						Queue_LiveEnequeue(&DPS_LiveQueue,(char *)DPS_GenDiagnostic(enmDiagnostic_MB_WRITE_QUERY_SUCCESSFUL));
								    strcat((char * )TempMbPayloadBuff1,(char * )TempMbPayloadBuff);
								  //	MBWritePayload = 1;
										u8MBWrQueryRegisterAddressCounter++;
										master.u8MBNoQueryAttempts = 0;
										resetModbusPort();
										gu32ModbusPollDelay = master.gu32ModbusPollDelay;
										modbusState = enmMODBUS_IDLE;
										break;
									}
									else
									{

									}
								}
								else
								{
									/*Write query unsuccessfull*/
									modbusState = enmMODBUS_RETRY_QUERY;
									break;
								}
							}
							else
							{
								/*Write query unsuccessfull */
								modbusState = enmMODBUS_RETRY_QUERY;
								break;
							}
						}
						else
						{
							///nothing to do or handle later
						}
					}
					else
					{
						/* FC not Matched */
						modbusState = enmMODBUS_RETRY_QUERY;
					}
			 }
			 else
			 {
					/* Slave ID not Matched */
					modbusState = enmMODBUS_RETRY_QUERY;
				}
			}

			break;

  case enmMODBUS_RETRY_QUERY:
					memset(master.u8SlaveResponseArray, 0x00, sizeof(master.u8SlaveResponseArray));
					if(master.u8MBNoQueryAttempts < MB_QUERY_RETRY_ATTEMPTS)
					{
							master.u8MBNoQueryAttempts++;
					}
					else
					{
							if(SinglewriteFlag == 1)
							{
									u8MBQueryRegisterAddressCounter++;
							}
							else if(MultiwriteFlag == 1)
							{
									u8MBWrQueryRegisterAddressCounter++;
							}
							master.u8MBNoQueryAttempts = 0;
							memset(master.u8SlaveResponseArray, 0x00, sizeof(master.u8SlaveResponseArray));
					}
					resetModbusPort();
					gu32ModbusPollDelay = master.gu32ModbusPollDelay;
					modbusState = enmMODBUS_IDLE;
		break;
      case enmMODBUS_RESPONSE_TIMEOUT:
						memset(master.u8SlaveResponseArray, 0x00, sizeof(master.u8SlaveResponseArray));
						if(master.u8MBNoQueryAttempts < MB_QUERY_RETRY_ATTEMPTS)
						{
								master.u8MBNoQueryAttempts++;
						}
						else
						{
								master.u8MBNoQueryAttempts = 0;
								if(SinglewriteFlag == 1)
								{
										u8MBQueryRegisterAddressCounter++;
								}
								else if(MultiwriteFlag == 1)
								{
										u8MBWrQueryRegisterAddressCounter++;
								}
								memset(master.u8SlaveResponseArray, 0x00, sizeof(master.u8SlaveResponseArray));
								gu32ModbusPollDelay = FIVEHUNDRED_MS;/*500 ms For enmMODBUS_IDLE state */
						}
						resetModbusPort();
						modbusState = enmMODBUS_IDLE;
	    break;
	}
}

void resetModbusPort()
{
	HAL_GPIO_WritePin(GPIOA,MB_DE_Pin,GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOA,MB_RE_Pin,GPIO_PIN_SET);
}

/******************************************************************************
 Function: MB_FillSlaveRawData
 Purpose:  Update All salves raw data in single buffer master.u8SlaveData Buffer
 Input:
 	 	 1) Modbus Query Counter ,
 	 	 2) Total bytes received in Modbus Response
 Return value: None.

 Note(s)(if-any):

 Change History:
 Author           	Date                Remarks
 Anil More		    23-05-2022

******************************************************************************/

static void MB_FillSlaveRawData(uint16_t u16QueryCntr, uint16_t u16NumberofBytesReceived)
{
	if(WriteQueryReceivedflag)
	{
		return;
	}

	uint16_t u16DataFillStartIndex = 0;
	uint16_t u16LoopCntr= 0;

	if(st_DeviceConfig.u16MbTotalNoOfQuerys > u16QueryCntr)
	{
		/*Step 1: Calculate the start index for the store data of the respective query*/
		for(u16LoopCntr =0; u16LoopCntr < u16QueryCntr; u16LoopCntr++)
		{
			u16DataFillStartIndex += master.u16NoOfBytesForValidResponse[u16LoopCntr];
		}

		/* Step 2: verify below 2 conditions
		 *   1) Response for selected query is valid
		 *   2) Number of bytes received are same as expected Bytes
		 *   If the above two conditions are true Then Fill The Received Raw data in the master.u8SlaveData buffer
		 *   If any condition is false in the above 2 conditions, fill the zero data.
		 */

		if((master.enum_MBResponseStatusBuff[u16QueryCntr] == enm_ResponseSucccess)&&
				(u16NumberofBytesReceived ==  master.u16NoOfBytesForValidResponse[u16QueryCntr] ))
			{
					/*communication established successfully Fill Response data*/
					for(u16LoopCntr=0;u16LoopCntr<master.u16NoOfBytesForValidResponse[u16QueryCntr];u16LoopCntr++)
					{
							master.u8SlaveData[u16DataFillStartIndex+u16LoopCntr]= master.u8SlaveResponseArray[3+u16LoopCntr];
					}
					MB_UpdateModbusPayload();
		  }
		  else
		  {
					/*Something went wrong Either Timeout Error or CRC error or garbage data received*/
					for(u16LoopCntr=0;u16LoopCntr<master.u16NoOfBytesForValidResponse[u16QueryCntr];u16LoopCntr++)
					{
//				  master.u8SlaveData[u16DataFillStartIndex+u16LoopCntr]= 0;
					}
					 MB_UpdateModbusPayload();
		  }
	}
	else
	{
		//DO Nothing
		//Invalid State
	}
}

/******************************************************************************
 Function: MB_UpdateModbusPayload
 Purpose:  Update GSM Payload Array
 Input:	None
 Return value: None.

 Note(s)(if-any):

 Change History:
 Author           	Date                Remarks
 KloudQ Team      11-04-18
 KloudQ Team	  10-09-18				Update. Modbus data added to gsm payload
 KloudQ Team	  22-10-18				Update . For Loop removed
******************************************************************************/

int  MB_UpdateModbusPayload(void)
{
	if(WriteQueryReceivedflag)
	{
		return 1;
	}
	uint8_t u8TempBuff[20] = {0};
	uint16_t u16QueryLoopCtr = 0,u16BytesLoopCntr = 0;
	uint16_t u16DataBuffIndex = 0;
	int16_t u16TempRegValue = 0;//chnaging uint16_t to int16_t
  int16_t temp =0;
	uint8_t unsignedTempBuff[20] = {0};
	uint16_t unsignedTempRegValue = 0;
	memset(gu8MBRTUPayloadString,0x00,sizeof(gu8MBRTUPayloadString));
	memset(tempunsignedRTUPaylaod,0x00,sizeof(tempunsignedRTUPaylaod));

	for(u16QueryLoopCtr=0;u16QueryLoopCtr<st_DeviceConfig.u16MbTotalNoOfQuerys;u16QueryLoopCtr++)
  {
	  if(enm_ResponseSucccess == master.enum_MBResponseStatusBuff[u16QueryLoopCtr])
    {
      if((st_DeviceConfig.stMbMasterQuerysArr[u16QueryLoopCtr].u8MbFunctionCode == 0x01) || (st_DeviceConfig.stMbMasterQuerysArr[u16QueryLoopCtr].u8MbFunctionCode == 0x02))
	    {
			   if(master.u16NoOfBytesForValidResponse[u16QueryLoopCtr] == 1)
			   {
						unsignedTempRegValue = (uint16_t)master.u8SlaveData[u16DataBuffIndex];
						decimalToBinaryWithCommas(unsignedTempRegValue, binaryString);
						strcat((char *)tempunsignedRTUPaylaod,(char * )binaryString);
						strcat((char *)gu8MBRTUPayloadString,(char * )binaryString);
						u16DataBuffIndex += 1;
			   }
			   else if(master.u16NoOfBytesForValidResponse[u16QueryLoopCtr] == 2)
			   {
						unsignedTempRegValue = ((uint16_t)master.u8SlaveData[u16DataBuffIndex+1] << (uint16_t)8)|((uint16_t)master.u8SlaveData[u16DataBuffIndex]);
						decimalToBinaryWithCommas(unsignedTempRegValue, binaryString);
						strcat((char *)tempunsignedRTUPaylaod,(char * )binaryString);
						strcat((char *)gu8MBRTUPayloadString,(char * )binaryString);
						u16DataBuffIndex += 2;
			  }
			else
			{
					//nothing to do
			}
	 }
	 else if((st_DeviceConfig.stMbMasterQuerysArr[u16QueryLoopCtr].u8MbFunctionCode == 0x03) || (st_DeviceConfig.stMbMasterQuerysArr[u16QueryLoopCtr].u8MbFunctionCode  == 0x04))
	 {
		for(u16BytesLoopCntr=0; u16BytesLoopCntr < (master.u16NoOfBytesForValidResponse[u16QueryLoopCtr]/2) ;u16BytesLoopCntr++) //Slave raw data
		{
			memset(u8TempBuff, 0, sizeof(u8TempBuff));
			memset(unsignedTempBuff, 0, sizeof(unsignedTempBuff));
			//chnaging uint16_t to int16_t
			u16TempRegValue = ((uint16_t)master.u8SlaveData[u16DataBuffIndex] << 8)|((uint8_t)master.u8SlaveData[u16DataBuffIndex+1]);
			/* Below patch Just for reference*/
			unsignedTempRegValue = ((uint16_t)master.u8SlaveData[u16DataBuffIndex] << (uint16_t)8)|((uint16_t)master.u8SlaveData[u16DataBuffIndex+1]);
			sprintf((char * )unsignedTempBuff,"%05d,",(int)unsignedTempRegValue);
			strcat((char *)tempunsignedRTUPaylaod,(char * )unsignedTempBuff);
			/*Above patch Just for reference*/
			if(u16TempRegValue > 32676)
			{
				temp = u16TempRegValue - 65536;
				sprintf((char * )u8TempBuff,"%05d,",(int)temp);
				u16DataBuffIndex += 2;
				strcat((char *)gu8MBRTUPayloadString,(char * )u8TempBuff);
			}
			else
			{
				sprintf((char * )u8TempBuff,"%05d,",(int)u16TempRegValue);
				u16DataBuffIndex += 2;
				strcat((char *)gu8MBRTUPayloadString,(char * )u8TempBuff);
			}
		}
	}
	else
	{
		//nothing to do
	}
}
  else
  {
  	if((st_DeviceConfig.stMbMasterQuerysArr[u16QueryLoopCtr].u8MbFunctionCode == 0x01) || (st_DeviceConfig.stMbMasterQuerysArr[u16QueryLoopCtr].u8MbFunctionCode == 0x02))
	  {
  		memset(u8TempBuff, 0, sizeof(u8TempBuff));
			sprintf((char * )u8TempBuff,"Er%d,",(int)master.enum_MBResponseStatusBuff[u16QueryLoopCtr]);
			strcat((char *)gu8MBRTUPayloadString,(char * )u8TempBuff);
		  strcat((char *)tempunsignedRTUPaylaod,(char * )u8TempBuff);
			if (master.u16NoOfBytesForValidResponse[u16QueryLoopCtr] == 1)
			{
				u16DataBuffIndex += 1;
			}
			else if (master.u16NoOfBytesForValidResponse[u16QueryLoopCtr] == 2)
			{
				u16DataBuffIndex += 2;
			}
			else
			{
				// may need to handle.
			}
	  }
	  else if ((st_DeviceConfig.stMbMasterQuerysArr[u16QueryLoopCtr].u8MbFunctionCode == 0x03) || (st_DeviceConfig.stMbMasterQuerysArr[u16QueryLoopCtr].u8MbFunctionCode == 0x04))
	  {
			for(u16BytesLoopCntr=0;u16BytesLoopCntr < (master.u16NoOfBytesForValidResponse[u16QueryLoopCtr]/2) ;u16BytesLoopCntr++) //Slave raw data
			{
				memset(u8TempBuff, 0, sizeof(u8TempBuff));
				sprintf((char * )u8TempBuff,"Er%d,",(int)master.enum_MBResponseStatusBuff[u16QueryLoopCtr]);
				strcat((char *)gu8MBRTUPayloadString,(char * )u8TempBuff);
				strcat((char *)tempunsignedRTUPaylaod,(char * )u8TempBuff);
				u16DataBuffIndex += 2;
			}
	 }
	 else
	 {

	 }
  }
}
return 0;
}

void decimalToBinaryWithCommas(uint16_t number, char* binaryString)
{
   char temp[16] = {0};  // 16-bit number for this example
   int count = 0;
   while (number > 0)
	 {
			temp[count] = (number % 2) + '0';
			number /= 2;
			count++;
   }
   if (count == 0)
	 {
      strcpy(binaryString, "0,");
   }
	 else
	 {
      int index = 0;
		  memset(binaryString,0x00,64);
      for (int i = count-1; i >= 0; i--)
		  {
          binaryString[index++] = temp[i];
          if (i > 0)
		      {
             binaryString[index++] = '^';
          }
      }
		  binaryString[index] = ',';
   }
}


/******************************************************************************
 Function Name: setupModbus
 Purpose: Initialise MODBUS Structure
 Input:	None.
 Return value: None.

 Note(s)(if any) :


 Change History:
 Author           	Date                Remarks
 KloudQ Team       11-04-18

******************************************************************************/
void setupModbus(void)
{
	uint16_t u16LoopCntr = 0,u16CalLength = 0,u16TempVar = 0;

	/* Initialize Modbus UART
	 * Initialize Modbus RE and DE Pin as Output
	 */

	GPIO_InitTypeDef GPIO_InitStruct = {0};

	/*Configure GPIO pin Output Level */

	HAL_GPIO_WritePin(GPIOA, MB_RE_Pin|MB_DE_Pin, GPIO_PIN_RESET);

	/*Configure GPIO pins : PCPin PCPin PCPin */
	GPIO_InitStruct.Pin = MB_RE_Pin|MB_DE_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	/* MODBUS GPIO and Peripherals Initialization */

	HAL_GPIO_WritePin(GPIOA,MB_DE_Pin,GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOA,MB_RE_Pin,GPIO_PIN_SET);

	UART2_Init(st_DeviceConfig.enumMbBaudRate,st_DeviceConfig.enumMBParity,st_DeviceConfig.enumMbStopBit);

	LL_USART_EnableIT_RXNE(USART2);

	/* Time difference between two data scanning cycles(1 cycle means Total query are fired and wait for again Query 0 Fires) */

//	master.u32MbCycleRestartTmr = HUNDRED_MS;
	master.u32MbCycleRestartTmr = THREE_SEC;
	/* Time Difference between Query */

	master.gu32ModbusPollDelay = gu32Modbus485PollingTime;//HUNDRED_MS;

	/* Modbus Timeout Error Time */

	master.u32MbResponseTimeout = gu32ModbusResponseTimeout;//TWO_SEC;
	master.u8MBOperationStatus = RESET;
	gu32ModbusPollDelay = 0;

	/* Update data bytes length for each query */

	for(u16LoopCntr=0;u16LoopCntr<st_DeviceConfig.u16MbTotalNoOfQuerys;u16LoopCntr++){

		u16TempVar = 0;
		u16CalLength = 0;
		if((st_DeviceConfig.stMbMasterQuerysArr[u16LoopCntr].u8MbFunctionCode == 0x01)
				|| (st_DeviceConfig.stMbMasterQuerysArr[u16LoopCntr].u8MbFunctionCode == 0x02)){

			u16CalLength = st_DeviceConfig.stMbMasterQuerysArr[u16LoopCntr].u16MbNoOfPoints;
			u16TempVar = u16CalLength /(uint16_t)8; //8 Reg in one byte

			if(u16CalLength %(uint16_t)8){

				u16CalLength = u16TempVar+(uint16_t)1;
			}
			else{

				u16CalLength = u16TempVar;
			}

		}
		else if((st_DeviceConfig.stMbMasterQuerysArr[u16LoopCntr].u8MbFunctionCode == 0x03)
				|| (st_DeviceConfig.stMbMasterQuerysArr[u16LoopCntr].u8MbFunctionCode == 0x04))
		{
			u16CalLength = st_DeviceConfig.stMbMasterQuerysArr[u16LoopCntr].u16MbNoOfPoints * 2;
		}
		else{

			u16CalLength = 0;
		}

		// Maximum data bytes at valid Response

		master.u16NoOfBytesForValidResponse[u16LoopCntr] = u16CalLength;
		master.enum_MBResponseStatusBuff[u16LoopCntr] = enm_ResponseTimeOut;
		master.u16QueryNCCntrBufff[u16LoopCntr] = 5;
	}
}


/****************************************************************************
 Function: UserAppl_Init
 Purpose: Initialize the user editable settings.

 Input:	None
 Return value: None


 Note(s)(if-any) :

 Change History:
 Author            	Date                Remarks
 KloudQ Team        03/06/2022			100367
******************************************************************************/
void UserAppl_FillDefaultparameters(ST_Config *ptrStruct)
{
//	uint16_t querryCounter = 0;

	/*Data uploading frequency*/
	ptrStruct->u32DataUploadingFreq = DATA_UPLADING_FREQUENCY;

	ptrStruct->u8BufferMemEnFlag = 0;

	ptrStruct->u8ModbusMasterEnFlag = 1;

	switch(Baudrate)
	{
		case 2400:
			ptrStruct->enumMbBaudRate = enum_BAUD_RATE_2400;
		break;

		case 4800:
			ptrStruct->enumMbBaudRate = enum_BAUD_RATE_4800;
		break;

		case 9600:
			ptrStruct->enumMbBaudRate = enum_BAUD_RATE_9600;
		break;

		case 14400:
			ptrStruct->enumMbBaudRate = enum_BAUD_RATE_14400;
		break;

		case 19200:
			ptrStruct->enumMbBaudRate = enum_BAUD_RATE_19200;
		break;

		case 28800:
			ptrStruct->enumMbBaudRate = enum_BAUD_RATE_28800;
		break;

		case 33600:
			ptrStruct->enumMbBaudRate = enum_BAUD_RATE_33600;
		break;

		case 38400:
			ptrStruct->enumMbBaudRate = enum_BAUD_RATE_38400;
		break;

		case 57600:
			ptrStruct->enumMbBaudRate = enum_BAUD_RATE_57600;
		break;

		case 115200:
			ptrStruct->enumMbBaudRate = enum_BAUD_RATE_115200;
		break;

		case 230400:
			ptrStruct->enumMbBaudRate = enum_BAUD_RATE_230400;
		break;

		case 460800:
			ptrStruct->enumMbBaudRate = enum_BAUD_RATE_460800;
		break;

		case 921600:
			ptrStruct->enumMbBaudRate = enum_BAUD_RATE_921600;
		break;

		case 1000000:
			ptrStruct->enumMbBaudRate = enum_BAUD_RATE_1000000;
		break;

		default://If selection is invalid then default baud rate is 115200
			ptrStruct->enumMbBaudRate = enum_BAUD_RATE_115200;
		break;
	}

switch(Parity)
{

		case 0:
			ptrStruct->enumMBParity = enum_USART_PARITY_NONE;
		break;

		case 1:
			ptrStruct->enumMBParity = enum_USART_PARITY_ODD;
		break;

		case 2:
			ptrStruct->enumMBParity = enum_USART_PARITY_EVEN;
		break;

		default:
			ptrStruct->enumMBParity = enum_USART_PARITY_NONE;
		break;
}

switch(Stopbit)
{
		case 2:
			ptrStruct->enumMbStopBit = enum_USART__STOPBITS_2;
		break;

		case 1:
			ptrStruct->enumMbStopBit = enum_USART_STOPBITS_1;
		break;

		default:
			ptrStruct->enumMbStopBit = enum_USART_STOPBITS_1;
		break;
}

//	ptrStruct->u16MbTotalNoOfQuerys = st_DeviceConfig.u16MbTotalNoOfQuerys;
//	for(querryCounter = 0 ; querryCounter < ptrStruct->u16MbTotalNoOfQuerys ; querryCounter++)
//	{
//		ptrStruct->stMbMasterQuerysArr[querryCounter].u8MbSlaveID = st_DeviceConfig.stMbMasterQuerysArr[querryCounter].u8MbSlaveID;
//		ptrStruct->stMbMasterQuerysArr[querryCounter].u8MbFunctionCode = st_DeviceConfig.stMbMasterQuerysArr[querryCounter].u8MbFunctionCode;
//		ptrStruct->stMbMasterQuerysArr[querryCounter].u16MbStartingAddress = st_DeviceConfig.stMbMasterQuerysArr[querryCounter].u16MbStartingAddress ;
//		ptrStruct->stMbMasterQuerysArr[querryCounter].u16MbNoOfPoints = st_DeviceConfig.stMbMasterQuerysArr[querryCounter].u16MbNoOfPoints ;
//	}


}
