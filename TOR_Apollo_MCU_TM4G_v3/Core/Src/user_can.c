/*
 * user_can.c
 *
 *  Created on: Nov 15, 2023
 *      Author: abhisheks
 */

#include "user_can.h"
#include "user_can_config.h"
#include "main.h"
#include "stdlib.h"
#include "externs.h"
#include "error_handler.h"
#include "user_Timer.h"
#include "user_config.h"


CAN_TxHeaderTypeDef   TxHeader;
CAN_RxHeaderTypeDef   RxHeader;
CAN_FilterTypeDef  sFilterConfig;

uint8_t     TxData[8] = {0,1,1,0,2,0,2,0};
uint8_t     RxData[8];
uint32_t	TxMailbox;

uint8_t gu8MaxSpeed = 0, gu8MinSpeed = 255;
uint16_t gu16SpeedCounter = 0, gu16MaxSpeedCounter = 0, gu16MinSpeedCounter = 0;

uint32_t CANTxFlag = FALSE;
uint32_t gu32CanQueryArray[15] ={ 336,337,0,0,0,0,0,0,0	};

uint32_t u32CanIdsReceived[CAN_BUFFER_LENGTH] = {'0'};
uint32_t u32CanRxMsgLoopCounter = 0;
uint32_t u32CanIdsReceived_2[CAN_BUFFER_LENGTH] = {'0'};
uint64_t gu64CanMessageArray_2[MAX_CAN_IDS_SUPPORTED];

uint32_t canTestVariable = 0;
uint32_t u32IdSearchFlag = FALSE;

uint64_t gu64CanMessageArray[MAX_CAN_IDS_SUPPORTED] = {0};
uint64_t gu64CanMessageArrayTest[MAX_CAN_IDS_SUPPORTED] = {0};
uint64_t u64CanMessageReceived[CAN_BUFFER_LENGTH] = {0};
uint64_t gu64CanMessageIDPGN[MAX_CAN_IDS_SUPPORTED] = {0};

uint32_t gu32HMHour = 0, gu32HMMinute = 0;

volatile uint32_t gu32ProcessCanMessage = 0;
//strCanReceivedMsg unCanReceivedMsgs[MAX_CAN_IDS_SUPPORTED];
//change by AJ
strCanReceivedMsg unCanReceivedMsgs[CAN_BUFFER_LENGTH];
//unCan1939CommandId unCan1939ReceivedId[MAX_CAN_IDS_SUPPORTED];
uint32_t gu32CanBAMArray[3];
uint8_t MultiFrameCAN_ID[150];
uint16_t MultiFramecounter;
uint8_t MultiFrameRequest[8];
uint32_t multiNumBytes;
uint32_t NumberMultiFrame ;
uint32_t u32CanMsgID_1;
uint32_t u32MultiFrameCommandId ;
_Bool receivingCANDATA ;
_Bool u32MultiFrameFlag ;
uint32_t u32MultiFrameRequestId;

uint16_t MultiFrameReceivedCounter;
strCanReceivedMsg unCanReceivedMsgs_2[MAX_CAN_IDS_SUPPORTED];


//uint64_t gu64CanMessageArray[MAX_CAN_IDS_SUPPORTED] = {0};
//uint64_t gu64CanMessageArrayTest[MAX_CAN_IDS_SUPPORTED] = {0};
//uint64_t u64CanMessageReceived[CAN_BUFFER_LENGTH] = {0};
//uint64_t gu64CanMessageIDPGN[MAX_CAN_IDS_SUPPORTED] = {0};

//uint32_t gu32HMHour = 0, gu32HMMinute = 0;

//volatile uint32_t gu32ProcessCanMessage = 0;


//unCan1939CommandId unCan1939ReceivedId[MAX_CAN_IDS_SUPPORTED];
//change by AJ
unCan1939CommandId unCan1939ReceivedId[CAN_BUFFER_LENGTH];
unCan1939CommandId unCan1939ReceivedBAMId[3];

RequestedCANID_t requestedCANIDs_Tx[MAX_Tx_CAN_IDS_SUPPORTED + 1];

/* Configuration Array for Can Peripheral
 * Prerequisite : [0] - Will / Should always contain ONLY CAN BaudRate / Bit Rate !
 * CommandIds   : [1 to 50] Sequential list of CAN Command Ids to be captured .
 * Defaults / Example : { 500, 1, 2, 3, 0x3AD,0x1FF,6,7,0x7FF,9  ,0x3AB};
 *                        BR ,Id,Id,Id, Id  , Id  ,Id , Id  ,Id , Id
 * */
uint32_t gu32CanConfigurationArray[(MAX_CAN_IDS_SUPPORTED + 1)] = {'0'};
uint32_t gu32PGNBAMArray[3]={'0'};

/* Configuration Array for Can Peripheral - PGN Filter
 * Prerequisite : [0] - Will / Should always contain ONLY CAN BaudRate / Bit Rate !
 * CommandIds   : [1 to 50] Sequential list of CAN Command Ids to be captured .
 * Defaults / Example : { 500, 1, 2, 3, 0x3AD,0x1FF,6,7,0x7FF,9  ,0x3AB};
 *                        BR ,Id,Id,Id, Id  , Id  ,Id , Id  ,Id , Id
 * */

uint32_t gu32CanConfigurationArrayPGN[(MAX_CAN_IDS_SUPPORTED + 1)] = {'0'};


volatile enmCanQueryState canCurrentState = enmCANQUERY_IDLE;
uint32_t gu32CANQueryCommandResponseReceivedFlag = FALSE;

//static uint32_t u32IdStatus = 0;
//static uint32_t u32IdStatus_1 = 0;
//RequestedCANID_t requestedCANIDs_Tx[MAX_Tx_CAN_IDS_SUPPORTED];

RequestedCANIDTimer_t requestedCANID_Time[MAX_Tx_CAN_IDS_SUPPORTED];

uint32_t u32BAMArr0 = 0;
uint32_t u32BAMArr1 = 0;

void CAN_Init()
{
	hcan1.Instance = CAN1;
    canCurrentState = enmCANQUERY_IDLE;
	if(!gu32CANMode)
	{
		hcan1.Init.Mode = CAN_MODE_NORMAL;
		HAL_GPIO_WritePin(CAN1_MODE_GPIO_Port,CAN1_MODE_Pin,GPIO_PIN_RESET);
	}
	else if(gu32CANMode)
	{
		hcan1.Init.Mode = CAN_MODE_SILENT;
		HAL_GPIO_WritePin(CAN1_MODE_GPIO_Port,CAN1_MODE_Pin,GPIO_PIN_SET);
	}
	hcan1.Init.AutoBusOff = ENABLE;
	hcan1.Init.AutoWakeUp = ENABLE;
	hcan1.Init.AutoRetransmission = ENABLE;
	hcan1.Init.ReceiveFifoLocked = DISABLE;
										// By default 0 : Not Locked
										// FIFO Locked : Next incoming message will be discarded
										// FIFO UnLocked : Next incoming message will be overwrite the previous
	hcan1.Init.TimeTriggeredMode = DISABLE;
	hcan1.Init.TransmitFifoPriority = ENABLE;
										// Control transmission order
										// 0: Priority driven by Identifier of message
										// 1: Priority driven by request order


	// Setting related to Bit Timings
	uint32_t CAN_bitrate;

	if ((gu32CANType == 1))
	{
		CAN_bitrate = gu32CanConfigurationArray[0];
	}
	else if ((gu32CANType == 2))
  {
		CAN_bitrate = gu32CanConfigurationArrayPGN[0];
	}
	else if (gu32CANType == 3) {
		CAN_bitrate = requestedCANIDs_Tx[0].gu32TxCANId;
	}

	switch(CAN_bitrate)
	{
		case 1000:
			hcan1.Init.Prescaler = 5;
			hcan1.Init.TimeSeg1 = CAN_BS1_13TQ;
			hcan1.Init.TimeSeg2 = CAN_BS2_2TQ;
			hcan1.Init.SyncJumpWidth = CAN_SJW_1TQ;
		break;

		case 800:
			hcan1.Init.Prescaler = 10;
			hcan1.Init.TimeSeg1 = CAN_BS1_2TQ;
			hcan1.Init.TimeSeg2 = CAN_BS2_7TQ;
			hcan1.Init.SyncJumpWidth = CAN_SJW_1TQ;
		break;

		case 500:
			hcan1.Init.Prescaler = 10;
			hcan1.Init.TimeSeg1 = CAN_BS1_13TQ;
			hcan1.Init.TimeSeg2 = CAN_BS2_2TQ;
			hcan1.Init.SyncJumpWidth = CAN_SJW_1TQ;
		break;

		case 250:
			hcan1.Init.Prescaler = 20;
			hcan1.Init.TimeSeg1 = CAN_BS1_13TQ;
			hcan1.Init.TimeSeg2 = CAN_BS2_2TQ;
			hcan1.Init.SyncJumpWidth = CAN_SJW_1TQ;
		break;

		case 200:
			hcan1.Init.Prescaler = 25;
			hcan1.Init.TimeSeg1 = CAN_BS1_13TQ;
			hcan1.Init.TimeSeg2 = CAN_BS1_2TQ;
			hcan1.Init.SyncJumpWidth = CAN_SJW_1TQ;
		break;

		case 125:
			hcan1.Init.Prescaler = 40;
			hcan1.Init.TimeSeg1 = CAN_BS1_13TQ;
			hcan1.Init.TimeSeg2 = CAN_BS2_2TQ;
			hcan1.Init.SyncJumpWidth = CAN_SJW_1TQ;
		break;

		case 100:
			hcan1.Init.Prescaler = 50;
			hcan1.Init.TimeSeg1 = CAN_BS1_13TQ;
			hcan1.Init.TimeSeg2 = CAN_BS2_2TQ;
			hcan1.Init.SyncJumpWidth = CAN_SJW_1TQ;
		break;

		case 95:
			hcan1.Init.Prescaler = 54;
			hcan1.Init.TimeSeg1 = CAN_BS1_13TQ;
			hcan1.Init.TimeSeg2 = CAN_BS2_2TQ;
			hcan1.Init.SyncJumpWidth = CAN_SJW_1TQ;
		break;

		case 83:
			hcan1.Init.Prescaler = 60;
			hcan1.Init.TimeSeg1 = CAN_BS1_13TQ;
			hcan1.Init.TimeSeg2 = CAN_BS2_2TQ;
			hcan1.Init.SyncJumpWidth = CAN_SJW_1TQ;
		break;

		case 50:
			hcan1.Init.Prescaler = 100;
			hcan1.Init.TimeSeg1 = CAN_BS1_13TQ;
			hcan1.Init.TimeSeg2 = CAN_BS2_2TQ;
			hcan1.Init.SyncJumpWidth = CAN_SJW_1TQ;
		break;

		case 47:
			hcan1.Init.Prescaler = 105;
			hcan1.Init.TimeSeg1 = CAN_BS1_13TQ;
			hcan1.Init.TimeSeg2 = CAN_BS2_2TQ;
			hcan1.Init.SyncJumpWidth = CAN_SJW_1TQ;
		break;

		case 40:
			hcan1.Init.Prescaler = 125;
			hcan1.Init.TimeSeg1 = CAN_BS1_13TQ;
			hcan1.Init.TimeSeg2 = CAN_BS2_2TQ;
			hcan1.Init.SyncJumpWidth = CAN_SJW_1TQ;
		break;

		case 33:
			hcan1.Init.Prescaler = 150;
			hcan1.Init.TimeSeg1 = CAN_BS1_13TQ;
			hcan1.Init.TimeSeg2 = CAN_BS2_2TQ;
			hcan1.Init.SyncJumpWidth = CAN_SJW_1TQ;
		break;

		case 20:
			hcan1.Init.Prescaler = 250;
			hcan1.Init.TimeSeg1 = CAN_BS1_13TQ;
			hcan1.Init.TimeSeg2 = CAN_BS2_2TQ;
			hcan1.Init.SyncJumpWidth = CAN_SJW_1TQ;
		break;

		case 10:
			hcan1.Init.Prescaler = 500;
			hcan1.Init.TimeSeg1 = CAN_BS1_13TQ;
			hcan1.Init.TimeSeg2 = CAN_BS2_2TQ;
			hcan1.Init.SyncJumpWidth = CAN_SJW_1TQ;
		break;

		case 5:
			hcan1.Init.Prescaler = 1000;
			hcan1.Init.TimeSeg1 = CAN_BS1_13TQ;
			hcan1.Init.TimeSeg2 = CAN_BS2_2TQ;
			hcan1.Init.SyncJumpWidth = CAN_SJW_1TQ;
		break;

		default:

			/* Illegal BaudRate Configured . Use Default 500 Kbps */
			hcan1.Init.Prescaler = 10;
			hcan1.Init.TimeSeg1 = CAN_BS1_13TQ;
			hcan1.Init.TimeSeg2 = CAN_BS2_2TQ;
			hcan1.Init.SyncJumpWidth = CAN_SJW_1TQ;
		break;
	}

	for(int i=0 ;i < gau32TxConfigCANIDs_Count ;i++)
	{
		requestedCANID_Time[i+1].period = requestedCANIDs_Tx[i+1].gu32TxCANTime;
		requestedCANID_Time[i+1].value = requestedCANID_Time[i+1].period;
	}

  	if (HAL_CAN_Init(&hcan1) != HAL_OK)
  	{
  		assertError(enmTORERRORS_CAN1_INIT,enmERRORSTATE_ACTIVE);
  	}
  	else
  	{
  		 assertError(enmTORERRORS_CAN1_INIT,enmERRORSTATE_NOERROR);
  	}
}

/****************************************************************************
 Function: canFilterConfig
 Purpose: Init CAN peripheral with filter configuration
 Input: None.

 Return value: None

 Refer Link for timing calculations :
 http://www.bittiming.can-wiki.info/

 Clock = 48 Mhz (Refer Clock Configuration in CubeMX for details)

 Bit Rate    Pre-scaler  time quanta  Seg 1  Seg 2   Sample Point
 kbps

 1000			3			16			13	   2	    87.5
 500			6			16			13     2		87.5
 250			12			16			13     2    	87.5
 125			24			16			13     2		87.5
 100			30			16			13     2		87.5
 83.33			36			16			13     2		87.5
 50				60			16			13     2		87.5
 20				150			16			13     2		87.5
 10				300			16			13     2		87.5


 Note(s)(if-any) :

 Change History:
 Author            	Date                Remarks
 KloudQ Team        22/03/2020			initial Definitions
 kloudq				27/03/2020			Bit Calculation Added
 kloudq				20/04/2021			Added support for STM32L433 MCU
******************************************************************************/


void CAN_FilterConfig(void)
{
	sFilterConfig.FilterActivation = ENABLE;
	sFilterConfig.FilterBank = 0;
	sFilterConfig.FilterFIFOAssignment = CAN_RX_FIFO0;
	sFilterConfig.FilterIdHigh = 0x0000;
	sFilterConfig.FilterIdLow = 0x0000;
	sFilterConfig.FilterMaskIdHigh = 0x0000;
	sFilterConfig.FilterMaskIdLow = 0x0000;
	sFilterConfig.FilterMode = CAN_FILTERMODE_IDMASK;
	sFilterConfig.FilterScale = CAN_FILTERSCALE_32BIT;
	sFilterConfig.SlaveStartFilterBank = 14;

  	if (HAL_CAN_ConfigFilter(&hcan1, &sFilterConfig) != HAL_OK)
  	{
  		assertError(enmTORERRORS_CAN1_CONFIGFILTER,enmERRORSTATE_ACTIVE);
  	}
  	else
  	{
  		assertError(enmTORERRORS_CAN1_CONFIGFILTER,enmERRORSTATE_NOERROR);
  	}

  	/*##-3- Start the CAN peripheral ###########################################*/
  	  if (HAL_CAN_Start(&hcan1) != HAL_OK)
  	  {
  	    /* Start Error */
  		CAN_Error_Handler();
  	  }

  	/*##-4- Activate CAN RX notification #######################################*/
  	  if (HAL_CAN_ActivateNotification(&hcan1, CAN_IT_RX_FIFO0_MSG_PENDING) != HAL_OK)
  	  {
  		/* Notification Error */
  		  assertError(enmTORERRORS_CAN1_CONFIGFILTER,enmERRORSTATE_ACTIVE);
  	  }

  	memset(u32CanIdsReceived,0x00,sizeof(u32CanIdsReceived));
  	memset(unCanReceivedMsgs,0x00,sizeof(unCanReceivedMsgs));

}

/****************************************************************************
 Function: HAL_CAN_RxCpltCallback
 Purpose:  Rx complete callback in non blocking mode
 Input:	   CanHandle: pointer to a CAN_HandleTypeDef structure that contains
           the configuration information for the specified CAN.

 Return value: None
 Note(s)(if-any) :


 Change History:
 Author            	Date                Remarks
 KloudQ Team        20/04/21			initial code
******************************************************************************/
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
	/* LED Only for testing/ Indication . Can be removed in production if not required  */

	/* Get RX message */
	if (HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &RxHeader, RxData) != HAL_OK)
	{
		/* Reception Error */
		CAN_Error_Handler();
	}

	/* Parse the incoming data only if array location is available
	 * Added on 3/3/21 - For payload overwrite issue */

    receivingCANDATA = TRUE;
    
//    int i = 1;

	if(st_DeviceConfig.gau32BAMType == 1 )
	{
	    u32CanMsgID_1 = RxHeader.ExtId;

		if(gu32CanBAMArray[0] == u32CanMsgID_1)
		{
			u32MultiFrameRequestId = 0;
			u32MultiFrameCommandId = 0;

			memset(MultiFrameRequest,0x00,sizeof(MultiFrameRequest));
//			memset(MultiFrameCAN_ID,0x00,sizeof(MultiFrameCAN_ID));

			u32MultiFrameRequestId = u32CanMsgID_1;
		
			if((RxData[5] == 0xCA) && (RxData[6] == 0xFE) && (RxData[7] == 0))
			{
				NumberMultiFrame  = RxData[3];
				multiNumBytes = RxData[1];
				multiNumBytes = multiNumBytes - 0x02;
				u32MultiFrameFlag = TRUE;

				//memset(canDataRxTx.MultiFrameRequest,0x00,sizeof(canDataRxTx.MultiFrameRequest));

				MultiFrameRequest[0] = (RxData[7]);
				MultiFrameRequest[1] = (RxData[6]);
				MultiFrameRequest[2] = (RxData[5]);
				MultiFrameRequest[3] = (RxData[4]);
				MultiFrameRequest[4] = (RxData[3]);
				MultiFrameRequest[5] = (RxData[2]);
				MultiFrameRequest[6] = (RxData[1]);
				MultiFrameRequest[7] = (RxData[0]);

				HAL_GPIO_TogglePin(COMM_LED_uC_GPIO_Port,COMM_LED_uC_Pin);

			}
		}//end of if(gu32CanBAMArray[0] == u32CanMsgID_1)

		else if(u32MultiFrameFlag == TRUE && gu32CanBAMArray[1] == u32CanMsgID_1)
		{
			u32MultiFrameCommandId = RxHeader.ExtId;

			if(RxData[0] == 0)
			{
				MultiFramecounter = (RxData[0]); // 8 byte data, canDataRxTx.RxData[0] will be from 1 to
			}
			else
			{
//				MultiFramecounter = ((RxData[0]*8)-8); // 8 byte data, canDataRxTx.RxData[0] will be from 1 to 8
			MultiFramecounter = (RxData[0]*8); // 8 byte data, canDataRxTx.RxData[0] will be from 1 to 8
			}

//			memset(MultiFrameCAN_ID,0x00,sizeof(MultiFrameCAN_ID));

			MultiFrameCAN_ID[MultiFramecounter++] = RxData[0];
			MultiFrameCAN_ID[MultiFramecounter++] = RxData[1];
			MultiFrameCAN_ID[MultiFramecounter++] = RxData[2];
			MultiFrameCAN_ID[MultiFramecounter++] = RxData[3];
			MultiFrameCAN_ID[MultiFramecounter++] = RxData[4];
			MultiFrameCAN_ID[MultiFramecounter++] = RxData[5];
			MultiFrameCAN_ID[MultiFramecounter++] = RxData[6];
			MultiFrameCAN_ID[MultiFramecounter]   = RxData[7];

			HAL_GPIO_TogglePin(COMM_LED_uC_GPIO_Port,COMM_LED_uC_Pin);


		}//end of else if(u32MultiFrameFlag == TRUE && gu32CanBAMArray[1] == u32CanMsgID_1)
	}//end of if(st_DeviceConfig.gau32BAMType == 1 )

	else if(st_DeviceConfig.gau32BAMType == 2)
	{
		unCan1939ReceivedBAMId[0].u32J1939CommandId = RxHeader.ExtId;

		if(gu32PGNBAMArray[0] == unCan1939ReceivedBAMId[0].u16J1939PGN )
		{
			MultiFrameReceivedCounter = 0;
			u32MultiFrameRequestId = 0;
			u32MultiFrameCommandId = 0;

			memset(MultiFrameRequest,0x00,sizeof(MultiFrameRequest));
//			memset(MultiFrameCAN_ID,0x00,sizeof(MultiFrameCAN_ID));

			u32MultiFrameRequestId =  RxHeader.ExtId;

			if((RxData[5] == 0xCA) && (RxData[6] == 0xFE) && (RxData[7] == 0))
			{

				NumberMultiFrame  = RxData[3];
				multiNumBytes = RxData[1];
				multiNumBytes = multiNumBytes - 0x02;
				u32MultiFrameFlag = TRUE;

				MultiFrameRequest[0] = (RxData[7]);
				MultiFrameRequest[1] = (RxData[6]);
				MultiFrameRequest[2] = (RxData[5]);
				MultiFrameRequest[3] = (RxData[4]);
				MultiFrameRequest[4] = (RxData[3]);
				MultiFrameRequest[5] = (RxData[2]);
				MultiFrameRequest[6] = (RxData[1]);
				MultiFrameRequest[7] = (RxData[0]);

				HAL_GPIO_TogglePin(COMM_LED_uC_GPIO_Port,COMM_LED_uC_Pin);

			}
		}//endof if(gu32PGNBAMArray[0] == unCan1939ReceivedBAMId[0].u16J1939PGN )

		else if((gu32PGNBAMArray[1] == unCan1939ReceivedBAMId[0].u16J1939PGN)  && (u32MultiFrameFlag == TRUE))
		{
			u32MultiFrameCommandId = RxHeader.ExtId;

			if(RxData[0] == 0)
			{
				MultiFramecounter = (RxData[0]); // 8 byte data, canDataRxTx.RxData[0] will be from 1 to
			}
			else
			{
//				MultiFramecounter = ((RxData[0]*8)-8); // 8 byte data, canDataRxTx.RxData[0] will be from 1 to 8
//			updated by AJ on 160524 b'cos if RxData[0] = 1 i.e (1*8)-8 = 0 which will over write the 0th data
				MultiFramecounter = (RxData[0]*8); // 8 byte data, canDataRxTx.RxData[0] will be from 1 to 8
			}

			MultiFrameCAN_ID[MultiFramecounter++] = RxData[0];
			MultiFrameCAN_ID[MultiFramecounter++] = RxData[1];
			MultiFrameCAN_ID[MultiFramecounter++] = RxData[2];
			MultiFrameCAN_ID[MultiFramecounter++] = RxData[3];
			MultiFrameCAN_ID[MultiFramecounter++] = RxData[4];
			MultiFrameCAN_ID[MultiFramecounter++] = RxData[5];
			MultiFrameCAN_ID[MultiFramecounter++] = RxData[6];
			MultiFrameCAN_ID[MultiFramecounter]   = RxData[7];
			HAL_GPIO_TogglePin(COMM_LED_uC_GPIO_Port,COMM_LED_uC_Pin);


		}//end of else if((gu32PGNBAMArray[1] == unCan1939ReceivedBAMId[0].u16J1939PGN)  && (u32MultiFrameFlag == TRUE))



	}//end of else if(st_DeviceConfig.gau32BAMType == 2)

	else{}

	if(RxHeader.IDE == CAN_ID_EXT)
	{
		u32CanMsgID_1 = RxHeader.ExtId;
	}
	else if(RxHeader.IDE == CAN_ID_STD)
	{
		u32CanMsgID_1= RxHeader.StdId;
	}
	else{}

	if(u32CanMsgID_1 != gu32CanBAMArray[0] || u32CanMsgID_1 != gu32CanBAMArray[1] )
	{
		if(unCan1939ReceivedId[u32CanRxMsgLoopCounter].u32J1939CommandId==0)
		{

			if(RxHeader.IDE == CAN_ID_EXT)
			{
				unCan1939ReceivedId[u32CanRxMsgLoopCounter].u32J1939CommandId = RxHeader.ExtId;
			}
			else if(RxHeader.IDE == CAN_ID_STD)
			{
				unCan1939ReceivedId[u32CanRxMsgLoopCounter].u32J1939CommandId = RxHeader.StdId;
			}

			unCanReceivedMsgs[u32CanRxMsgLoopCounter].u8CanMsgByte0 = (RxData[7]);
			unCanReceivedMsgs[u32CanRxMsgLoopCounter].u8CanMsgByte1 = (RxData[6]);
			unCanReceivedMsgs[u32CanRxMsgLoopCounter].u8CanMsgByte2 = (RxData[5]);
			unCanReceivedMsgs[u32CanRxMsgLoopCounter].u8CanMsgByte3 = (RxData[4]);
			unCanReceivedMsgs[u32CanRxMsgLoopCounter].u8CanMsgByte4 = (RxData[3]);
			unCanReceivedMsgs[u32CanRxMsgLoopCounter].u8CanMsgByte5 = (RxData[2]);
			unCanReceivedMsgs[u32CanRxMsgLoopCounter].u8CanMsgByte6 = (RxData[1]);
			unCanReceivedMsgs[u32CanRxMsgLoopCounter].u8CanMsgByte7 = (RxData[0]);
			memset(RxData, 0x00, sizeof(RxData));
			u32CanRxMsgLoopCounter++;
			HAL_GPIO_TogglePin(COMM_LED_uC_GPIO_Port,COMM_LED_uC_Pin);
		}//end of if(u32CanIdsReceived[u32CanRxMsgLoopCounter]==0)
	}//end of if(u32CanMsgID_1 != gu32CanBAMArray[0] || u32CanMsgID_1 != gu32CanBAMArray[1] )

	memset(RxData,0x00,sizeof(RxData));

	if(u32CanRxMsgLoopCounter >= CAN_BUFFER_LENGTH)
		u32CanRxMsgLoopCounter = 0;



}

/****************************************************************************
 Function: HAL_CAN_ErrorCallback
 Purpose:  CAN Error
 Input:	   hcan: pointer to a CAN_HandleTypeDef structure that contains
           the configuration information for the specified CAN.

 Return value: None
 Note(s)(if-any) :


 Change History:
 Author            	Date                Remarks
 KloudQ Team        20/04/21			initial code
******************************************************************************/
void HAL_CAN_ErrorCallback(CAN_HandleTypeDef *hcan)
{
	canTestVariable = hcan->ErrorCode;
	HAL_CAN_DeInit(&hcan1);
	CAN_FilterConfig();
}

/******************************************************************************
 Function Name: isCommandIdConfigured
 Purpose: Checks if received Id is configured for CAN
 Input:	uint32_t canId

 Return value: uint32_t u32PositioninConfigArray - Id Position in configuration Array
												 - 0 If Id not configured
 Notes:
 How to use this function for Testing

uint32_t idIndex = 0;
	do
	{
		idIndex = isCommandIdConfigured(0x0803FF00);
	}while(u32IdSearchFlag != 2);

	u32IdSearchFlag = 0;

 Change History:

 Author           	Date                Remarks
 KTL   				18-5-2020			Initial Draft . Tested All conditions
******************************************************************************/

uint32_t LoopCounter_2 = 0;

uint32_t isCommandIdConfigured(uint32_t canId)
{

	static uint32_t u32PositioninConfigArray = 0;

	if(LoopCounter_2 == 0)
	{
		u32IdSearchFlag = 1;
		u32PositioninConfigArray = 0;
	}

	if(u32IdSearchFlag == 1)
	{
		if(gu32CANType == 1)
		{
			if(gu32CanConfigurationArray[LoopCounter_2] == canId)
			{
				/*
				 * If Received CanID is found in configuration Array
				 * then parse the frame else ignore .
				 */
				u32PositioninConfigArray = LoopCounter_2;
				u32IdSearchFlag = 2;
				LoopCounter_2 = 0;
			}
			else
			{
				LoopCounter_2++;
				if (LoopCounter_2 == MAX_CAN_IDS_SUPPORTED)
				{
					LoopCounter_2 = 0;
					u32IdSearchFlag = 2;
				}
			}
		}

		else if(gu32CANType == 2)
		{
			if(gu32CanConfigurationArrayPGN[LoopCounter_2] == canId)
			{
				/*
				 * If Received CanID is found in configuration Array
				 * then parse the frame else ignore .
				 */
				u32PositioninConfigArray = LoopCounter_2;
				u32IdSearchFlag = 2;
				LoopCounter_2 = 0;
			}
			else
			{
				LoopCounter_2++;
				if (LoopCounter_2 == MAX_CAN_IDS_SUPPORTED)
				{
					LoopCounter_2 = 0;
					u32IdSearchFlag = 2;
				}
			}
		}
	}
	return u32PositioninConfigArray;
}

/******************************************************************************
 Function Name: parseCanMessageQueue
 Purpose: Parse CAN Message . If command ID is configured store the message
 Input:	None
 Return value: None

 Notes:

 Change History:

 Author           	Date                Remarks
 KTL   				19-5-2020			Initial Draft . Tested All conditions
 KTL				27-5-2020			Convert to Array Logic . Tested
******************************************************************************/
//uint32_t temp = 0;
uint32_t gu32CanIdParserCounter = 0;
//uint32_t u32CanMsgID = 0;
//uint32_t u32ParserState = 0;
//uint32_t u32IdStatus = 0;

void parseCanMessageQueue(void)
{
		static uint32_t u32CanMsgID = 0;
			static uint32_t u32ParserState = 0;
			static uint32_t u32IdStatus = 0;
//	if(u32CanIdsReceived[gu32CanIdParserCounter] != 0)
	if(unCan1939ReceivedId[gu32CanIdParserCounter].u32J1939CommandId != 0)
	{
		if(u32ParserState == 0)
		{
			/* Message Available. Parse The Message */
//			u32CanMsgID = u32CanIdsReceived[gu32CanIdParserCounter];
		if ((gu32CANCommType == 2) || (gu32CANCommType == 5))
		{
			u32CanMsgID = unCan1939ReceivedId[gu32CanIdParserCounter].u16J1939PGN;
		}
		else
		{
			u32CanMsgID = unCan1939ReceivedId[gu32CanIdParserCounter].u32J1939CommandId;
		}
			u32ParserState = 1;
		}
		else if(u32ParserState == 1)
		{
			/* In Process */
			if(u32IdSearchFlag == 2)
			{
				/* Search Process Completed */
				if(u32IdStatus != 0)
				{
					gu64CanMessageArray[u32IdStatus] =  ((uint64_t)unCanReceivedMsgs[gu32CanIdParserCounter].u8CanMsgByte7 << 56)|
														((uint64_t)unCanReceivedMsgs[gu32CanIdParserCounter].u8CanMsgByte6 << 48)|
														((uint64_t)unCanReceivedMsgs[gu32CanIdParserCounter].u8CanMsgByte5 << 40)|
														((uint64_t)unCanReceivedMsgs[gu32CanIdParserCounter].u8CanMsgByte4 << 32)|
														((uint64_t)unCanReceivedMsgs[gu32CanIdParserCounter].u8CanMsgByte3 << 24)|
														((uint64_t)unCanReceivedMsgs[gu32CanIdParserCounter].u8CanMsgByte2 << 16)|
														((uint64_t)unCanReceivedMsgs[gu32CanIdParserCounter].u8CanMsgByte1 << 8) |
														((uint64_t)unCanReceivedMsgs[gu32CanIdParserCounter].u8CanMsgByte0);
					if ((gu32CANCommType == 2) || (gu32CANCommType == 5))
					{
						gu64CanMessageIDPGN[u32IdStatus] = unCan1939ReceivedId[gu32CanIdParserCounter].u32J1939CommandId;
					}

						gu64CanMessageArrayTest[u32IdStatus] = gu64CanMessageArray[u32IdStatus];

//					u32CanIdsReceived_2[u32IdStatus] = u32CanIdsReceived[gu32CanIdParserCounter];
					// Reset Array Value for new Message
					unCanReceivedMsgs[gu32CanIdParserCounter].u8CanMsgByte7 = 0;
					unCanReceivedMsgs[gu32CanIdParserCounter].u8CanMsgByte6 = 0;
					unCanReceivedMsgs[gu32CanIdParserCounter].u8CanMsgByte5 = 0;
					unCanReceivedMsgs[gu32CanIdParserCounter].u8CanMsgByte4 = 0;
					unCanReceivedMsgs[gu32CanIdParserCounter].u8CanMsgByte3 = 0;
					unCanReceivedMsgs[gu32CanIdParserCounter].u8CanMsgByte2 = 0;
					unCanReceivedMsgs[gu32CanIdParserCounter].u8CanMsgByte1 = 0;
					unCanReceivedMsgs[gu32CanIdParserCounter].u8CanMsgByte0 = 0;
//					u32CanIdsReceived[gu32CanIdParserCounter] = 0;
					 unCan1939ReceivedId[gu32CanIdParserCounter].u32J1939CommandId = 0;
					u32IdStatus = 0;
				}
				else if(u32IdStatus == 0)
				{
					/* Command Id is not Configured . Discard the Message*/
					unCanReceivedMsgs[gu32CanIdParserCounter].u8CanMsgByte7 = 0;
					unCanReceivedMsgs[gu32CanIdParserCounter].u8CanMsgByte6 = 0;
					unCanReceivedMsgs[gu32CanIdParserCounter].u8CanMsgByte5 = 0;
					unCanReceivedMsgs[gu32CanIdParserCounter].u8CanMsgByte4 = 0;
					unCanReceivedMsgs[gu32CanIdParserCounter].u8CanMsgByte3 = 0;
					unCanReceivedMsgs[gu32CanIdParserCounter].u8CanMsgByte2 = 0;
					unCanReceivedMsgs[gu32CanIdParserCounter].u8CanMsgByte1 = 0;
					unCanReceivedMsgs[gu32CanIdParserCounter].u8CanMsgByte0 = 0;
//					u32CanIdsReceived[gu32CanIdParserCounter] = 0;
					 unCan1939ReceivedId[gu32CanIdParserCounter].u32J1939CommandId = 0;
				}
				u32IdSearchFlag = 0;
				u32ParserState = 0;
				u32CanMsgID = 0;
				gu32CanIdParserCounter++;
			}
			else
				u32IdStatus = isCommandIdConfigured(u32CanMsgID);
		}
	}
	else
	{
		gu32CanIdParserCounter++;
	}
	if(gu32CanIdParserCounter >= CAN_BUFFER_LENGTH)
		gu32CanIdParserCounter = 0;
}

/******************************************************************************
* Function : parsePGNCanMessageQueue(uint32_t canId)
*
* This function is used Parse CAN Message . If command ID is configured the store the message
*
* PRE-CONDITION: Enable CAN interface in CubeMx . Enable CAN Interrupt
*
* POST-CONDITION: Stored message of configured ID
*
*
	parseCanMessageQueue();

*******************************************************************************/
//uint32_t temp = 0;
//
//void parsePGNCanMessageQueue(void)
// {
//		static uint32_t u32CanMsgID = 0;
//		static uint32_t u32ParserState = 0;
//		static uint32_t u32IdStatus = 0;
//
//	if(unCan1939ReceivedId[gu32CanIdParserCounter].u32J1939CommandId != 0)
//	{
//		if(u32ParserState == 0)
//		{
//			/* Message Available. Parse The Message */
//			u32CanMsgID = unCan1939ReceivedId[gu32CanIdParserCounter].u16J1939PGN;
//			u32ParserState = 1;
//		}
//		else if(u32ParserState == 1)
//		{
//			/* In Process */
//			if(u32IdSearchFlag == 2)
//			{
//				/* Search Process Completed */
//				if(u32IdStatus != 0)
//				{
//
//					gu64CanMessageArray[u32IdStatus] =  ((uint64_t)unCanReceivedMsgs[gu32CanIdParserCounter].u8CanMsgByte7 << 56)|
//														((uint64_t)unCanReceivedMsgs[gu32CanIdParserCounter].u8CanMsgByte6 << 48)|
//														((uint64_t)unCanReceivedMsgs[gu32CanIdParserCounter].u8CanMsgByte5 << 40)|
//														((uint64_t)unCanReceivedMsgs[gu32CanIdParserCounter].u8CanMsgByte4 << 32)|
//														((uint64_t)unCanReceivedMsgs[gu32CanIdParserCounter].u8CanMsgByte3 << 24)|
//														((uint64_t)unCanReceivedMsgs[gu32CanIdParserCounter].u8CanMsgByte2 << 16)|
//														((uint64_t)unCanReceivedMsgs[gu32CanIdParserCounter].u8CanMsgByte1 << 8) |
//														((uint64_t)unCanReceivedMsgs[gu32CanIdParserCounter].u8CanMsgByte0);
//
//					gu64CanMessageArrayTest[u32IdStatus] = gu64CanMessageArray[u32IdStatus];
//
//					gu64CanMessageIDPGN[u32IdStatus] = unCan1939ReceivedId[gu32CanIdParserCounter].u32J1939CommandId;
//
//					// Reset Array Value for new Message
//					unCanReceivedMsgs[gu32CanIdParserCounter].u8CanMsgByte7 = 0;
//					unCanReceivedMsgs[gu32CanIdParserCounter].u8CanMsgByte6 = 0;
//					unCanReceivedMsgs[gu32CanIdParserCounter].u8CanMsgByte5 = 0;
//					unCanReceivedMsgs[gu32CanIdParserCounter].u8CanMsgByte4 = 0;
//					unCanReceivedMsgs[gu32CanIdParserCounter].u8CanMsgByte3 = 0;
//					unCanReceivedMsgs[gu32CanIdParserCounter].u8CanMsgByte2 = 0;
//					unCanReceivedMsgs[gu32CanIdParserCounter].u8CanMsgByte1 = 0;
//					unCanReceivedMsgs[gu32CanIdParserCounter].u8CanMsgByte0 = 0;
//					u32CanIdsReceived[gu32CanIdParserCounter] = 0;
//					unCan1939ReceivedId[gu32CanIdParserCounter].u32J1939CommandId = 0;
//					u32IdStatus = 0;
//				}
//				else if(u32IdStatus == 0)
//				{
//					/* Command Id is not Configured . Discard the Message*/
//					unCanReceivedMsgs[gu32CanIdParserCounter].u8CanMsgByte7 = 0;
//					unCanReceivedMsgs[gu32CanIdParserCounter].u8CanMsgByte6 = 0;
//					unCanReceivedMsgs[gu32CanIdParserCounter].u8CanMsgByte5 = 0;
//					unCanReceivedMsgs[gu32CanIdParserCounter].u8CanMsgByte4 = 0;
//					unCanReceivedMsgs[gu32CanIdParserCounter].u8CanMsgByte3 = 0;
//					unCanReceivedMsgs[gu32CanIdParserCounter].u8CanMsgByte2 = 0;
//					unCanReceivedMsgs[gu32CanIdParserCounter].u8CanMsgByte1 = 0;
//					unCanReceivedMsgs[gu32CanIdParserCounter].u8CanMsgByte0 = 0;
//					u32CanIdsReceived[gu32CanIdParserCounter] = 0;
//					unCan1939ReceivedId[gu32CanIdParserCounter].u32J1939CommandId = 0;
//				}
//				u32IdSearchFlag = 0;
//				u32ParserState = 0;
//				u32CanMsgID = 0;
//				gu32CanIdParserCounter++;
//			}
//			else
//				u32IdStatus = isCommandIdConfigured(u32CanMsgID);
//		}
//	}
//	else
//	{
//		gu32CanIdParserCounter++;
//	}
//
//	if(gu32CanIdParserCounter >= CAN_BUFFER_LENGTH){
//		gu32CanIdParserCounter = 0;
//	}
//}

/******************************************************************************
* Function : updateCANQuery()
*
*******************************************************************************/



void updateCANQuery(uint8_t canTxqueryCount)
{
//	TxHeader.StdId =  requestedCANIDs_Tx[canTxqueryCount].gu32TxCANId;
//	TxHeader.ExtId =  requestedCANIDs_Tx[canTxqueryCount].gu32TxCANId;

	if(requestedCANIDs_Tx[canTxqueryCount].gu32TxCANId > 0x7FF)
	{
		TxHeader.ExtId = requestedCANIDs_Tx[canTxqueryCount].gu32TxCANId;// gu32TxCANId[gu32TxCANCounter];
		TxHeader.IDE = CAN_ID_EXT;//CAN_ID_STD;//

	}
	else
	{
	TxHeader.StdId = requestedCANIDs_Tx[canTxqueryCount].gu32TxCANId;//gu32TxCANId[gu32TxCANCounter];
	TxHeader.IDE = CAN_ID_STD;//CAN_ID_EXT;
	}

//	TxHeader.IDE = CAN_ID_EXT;				// CAN_ID_STD;
											// IDE = 0 : 11 bit Identifier Frame (Standard)
	 	 	 	 	 	 	 	 	 	 	// IDE = 1 : 29 bit Identifier Frame (Extended)

	TxHeader.RTR = CAN_RTR_DATA; 			// Remote Data Frame

	TxHeader.DLC = requestedCANIDs_Tx[canTxqueryCount].dataLength;
	TxHeader.TransmitGlobalTime = ENABLE;	// Use in Time Trigger Communication
	TxData[0] = requestedCANIDs_Tx[canTxqueryCount].data[0];
	TxData[1] = requestedCANIDs_Tx[canTxqueryCount].data[1];
	TxData[2] = requestedCANIDs_Tx[canTxqueryCount].data[2];
	TxData[3] = requestedCANIDs_Tx[canTxqueryCount].data[3];
	TxData[4] = requestedCANIDs_Tx[canTxqueryCount].data[4];
	TxData[5] = requestedCANIDs_Tx[canTxqueryCount].data[5];
	TxData[6] = requestedCANIDs_Tx[canTxqueryCount].data[6];
	TxData[7] = requestedCANIDs_Tx[canTxqueryCount].data[7];

	CANTxFlag = TRUE;
}

/******************************************************************************
* Function : sendMessageCAN()
*
*******************************************************************************/

void sendMessageCAN (void)
{

	if (HAL_CAN_AddTxMessage(&hcan1, &TxHeader, TxData, &TxMailbox) != HAL_OK)
	{
	  /* Transmission request Error */
	  CAN_Error_Handler();         // Commented by Vedant on 30.08.23
	}
}

/******************************************************************************
* Function : executeCANQueries()
*
*******************************************************************************/
uint8_t canTxqueryCount = 0;

void executeCANQueries(void)
{
	switch(canCurrentState)
	{
		case enmCANQUERY_IDLE:
			/* Do not Query  */
			canCurrentState = enmCANQUERY_UPDATEQUERY;
			break;

		case enmCANQUERY_UPDATEQUERY:
			/* Update counter for Next query */

			if(canTxqueryCount >= gu32TxCANNoOfID  )
			{
				canTxqueryCount = 1;

			}
			else
			{
				canTxqueryCount++;
			}

			if(requestedCANID_Time[canTxqueryCount].value == 0)
			{
				updateCANQuery(canTxqueryCount);
			}

			if(CANTxFlag == TRUE)
			{
				canCurrentState = enmCANQUERY_SENDQUERY;
			}
			else
			{
				canCurrentState = enmCANQUERY_IDLE;
			}
			break;

		case enmCANQUERY_SENDQUERY:

			/* Transmit Query */

			/* Transmit Query */
			if(requestedCANID_Time[canTxqueryCount].value == 0)
			{
				sendMessageCAN();
				requestedCANID_Time[canTxqueryCount].value = requestedCANID_Time[canTxqueryCount].period;
			}

			gu32CANCommandResponseTimeout = FIVE_MS;
			canCurrentState = enmCANQUERY_AWAITRESPONSE;

			break;

		case enmCANQUERY_AWAITRESPONSE:
			if((gu32CANQueryCommandResponseReceivedFlag == TRUE) && (gu32CANCommandResponseTimeout != 0))
			{
				canCurrentState = enmCANQUERY_PASRERESPONSE;
				gu32CANCommandResponseTimeout = 0;
			}
			else if(gu32CANCommandResponseTimeout == 0)
			{
				canCurrentState = enmCANQUERY_RESPONSETIMEOUT;
			}
			else{}

			break;
		case enmCANQUERY_PASRERESPONSE:
			/* Store / Parse  received response */
			canCurrentState = enmCANQUERY_IDLE;

			if(gu32CANQueryCommandResponseReceivedFlag == TRUE)
			{
				gu32CANQueryCommandResponseReceivedFlag = FALSE;
			}
			break;

		case enmCANQUERY_RESPONSETIMEOUT:
			/*Response not received */
			canCurrentState = enmCANQUERY_IDLE;
			gu32CANQueryCommandResponseReceivedFlag = FALSE;
			break;

		default:
			/* Undefined State */
			canCurrentState = enmCANQUERY_IDLE;
			break;
	}
}

void CAN_Routine(void)
{
		if((gu32CANCommType == 1) || (gu32CANCommType == 2) || (gu32CANCommType == 4) || (gu32CANCommType == 5))
		{
			parseCanMessageQueue();
		}

		if ((gu32CANCommType == 3) | (gu32CANCommType == 4) | (gu32CANCommType == 5))
		{
			if((!gu32CANMode) && boolOnIntBattCANErrorFlag == FALSE)
			{
				executeCANQueries();
			}
		}
}

/*************************************************** End of Functions *********************************************************/

