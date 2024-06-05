/*
 * user_can_config.c
 *
 *  Created on: Jun 17, 2023
 *      Author: vedantj
 */

/* Config variables - CAN */
#include "main.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "externs.h"
#include "user_can.h"
#include "user_can_config.h"

uint32_t Config_CANConfigErr = 0;

/* CAN config */
uint32_t gu32CANType = 1;
uint32_t gu32CANCommType = 1;
uint32_t gu32OperateCAN = 1;
uint32_t gu32CANBitRate = 500;
uint32_t gu32CANMode = CAN_MODE_NORMAL;
uint32_t gu32CANNoOfID ;
uint32_t gu32TxCANNoOfID;

/* CAN Configuration Update Variables */
char gau8ConfigCANIsEnabled[2] = "";
char gau8ConfigCANMode[2] = "";
char gau8ConfigCANbitRate[5] = "";
char gau8ConfigCANNoOfIDs[3] = "";
char gau8ConfigTxCANNoOfIDs[3] = "";
char gau8ConfigCANType[2] = "";

uint32_t gau32ConfigCANIDs_Count = 0;
uint32_t gau32TxConfigCANIDs_Count = 0;
uint32_t gau32ConfigCANIDs[CONFIG_CAN_ID] = {'0'};
uint32_t gau32TxConfigCANIDs[CONFIG_CAN_ID_Tx];

/*
 * from user_systemconfig
 */
char * cansof = NULL;
char * caneof = NULL;
uint8_t gu8CommType;
/*
 * from user_modbus
 */
uint32_t gu32ExtractParamCounter = 0;

//static void verifyCANConfigData(void);


RequestedCANID requestedCANIDs[MAX_Tx_CAN_IDS_SUPPORTED + 1];

char canCSV_Enabled;

void parseCANID()
{
	char *token = NULL;
	char *rest = cansof;
//	_Bool canBroadID = 0;
	int j = 0;
	int broadcastCount = 0 ,reqCount = 0 ,reqDataCount = 0;
	// Parsing the string

	// Get Number of BroadCast CAN ID Count

	token = strtok_r(rest, ",", &rest);
	canCSV_Enabled = atoi(token);

	token = strtok_r(rest, ",", &rest);
	gau32ConfigCANIDs_Count = atoi(token);

	token = strtok_r(NULL, ",", &rest);

	// copy broadcast can id  till gau32ConfigCANIDs_Count

	for (int i = 0; i < gau32ConfigCANIDs_Count;) {

		if(strstr(":",token))
		{
			break;
		}

		// Check for "^"  CAN ID Seprator

	 	if(strstr("^",token))
		{
			token = strtok_r(NULL, ",", &rest);
			if(strstr(":",token))
			{
				break;
			}
			gau32ConfigCANIDs[i] = (int)strtol(token, NULL, 16);
			broadcastCount++;
		}
	 	else{
	 		break;
	 	}
		token = strtok_r(NULL, ",", &rest);
		i++;

		// Check for ":" Request - Respone CAN ID Seprator

//		else if(strstr(":",token))
//		{
//			gau32ConfigCANIDs[i]=0;
//			canBroadID = 1;
//		}
//		i++;
//
//		/* Check if their is any mismatch between number of
//		    Broadcast CAN ID & Actual Broadcast CAN ID */
//
//		if(!canBroadID)
//		{
//			token = strtok_r(NULL, ",", &rest);
//		}
	}

	// actual broadcast can id count

	gau32ConfigCANIDs_Count = broadcastCount;

	// execute loop till token != NULL

	while(token != NULL)
	{
		// Check for ":" Request - Respone CAN ID Seprator

		if(strstr(":",token))
		{
			// Get Number of Request - Respone CAN ID Count

			token = strtok_r(NULL, ",", &rest);
			gau32TxConfigCANIDs_Count = atoi(token);

			token = strtok_r(NULL, ",", &rest);

			// copy Request - Respone can id  till gau32TxConfigCANIDs_Count

			for (int i = 0; i < gau32TxConfigCANIDs_Count;)
			{
				// Check for "^"  CAN ID Seprator

				if(strstr("^",token))
				{
					// copy Request can id time

					token = strtok_r(NULL, ",", &rest);
					requestedCANIDs[i].time = (int)strtol(token, NULL, 0);

					// copy Request can id

					token = strtok_r(NULL, ",", &rest);
					requestedCANIDs[i].id = (int)strtol(token, NULL, 16);

					// copy Request can id DLC

					token = strtok_r(NULL, ",", &rest);
					requestedCANIDs[i].dataLength = atoi(token);

					token = strtok_r(NULL, ",", &rest);

					// copy Request can id Data

					// check Data length should be in in range 1 to 8

					if( requestedCANIDs[i].dataLength >= 1 && requestedCANIDs[i].dataLength <= 8 )
					{
						// reset previous data

						memset(requestedCANIDs[i].data, 0x00, sizeof(requestedCANIDs[i].data));

						for (j = 0; j < requestedCANIDs[i].dataLength; )
						{
							if(strstr("^",token))
							{
								requestedCANIDs[i].data[j] = 0;
							}
							else{
								requestedCANIDs[i].data[j] = (unsigned char)strtol(token, NULL,16);
								reqDataCount++;
								token = strtok_r(NULL, ",", &rest);
							}
							j++;
						}
						reqDataCount = 0;
						i++;
					}
					reqCount++;
				}
				// check for next token
				else
				{
					token = strtok_r(NULL, ",", &rest);
					if(token == NULL)
					{
						break;
					}
				}
			}
		}
		else
		{
			token = strtok_r(NULL, ",", &rest);
			// printf(" token = %s\n",token);
		}
	}
	gau32TxConfigCANIDs_Count = reqCount;
	if ((gau32TxConfigCANIDs_Count == 0) && (gu32CANCommType > 3)) {
		gu32CANCommType = gu32CANCommType - 3;
	}
	free(rest);
	rest = NULL;
	free(token);
	token= NULL;
}

/******************************************************************************
* Function : extractCANConfiguration()
*//**
* \b Description:
*
* This function is used to extract CAN config from received remote config strings
*
* PRE-CONDITION:
*
* POST-CONDITION: None
*
* @return 		None.
*
* \b Example Example:
* @code
*
* 	extractCANConfiguration();
*
* @endcode
*
* @see
*
* <br><b> - HISTORY OF CHANGES - </b>
*
* <table align="left" style="width:800px">
* <tr><td> Date       </td><td> Software Version </td><td> Initials </td><td> Description </td></tr>
* <tr><td> 08/03/2022 </td><td> 0.0.1            </td><td> HL100133 </td><td> Interface Created </td></tr>
* </table><br><br>
* <hr>
*
*******************************************************************************/
void extractCANConfiguration(void)
{
	gu32ExtractParamCounter = 0;

	while(cansof != NULL)
	{
		switch(gu32ExtractParamCounter)
		{
			case 0:
				/* SOF */
			break;

			case 1:
				/* HW ID */
			break;

			case 2:
				/* TWO WAY Communication */
			break;

			case 3:
				/* CAN Configuration : 4 */
			break;

			case 4:
				/* CAN TYPE */
				for(int i = 0;*cansof != ',';i++)
				{
					 gau8ConfigCANType[i] = *cansof++;
				}
			break;

			case 5:
				/* CAN MODE Normal Silent */
				for(int i = 0;*cansof != ',';i++)
				{
					gau8ConfigCANMode[i] = *cansof++;
				}
			break;

			case 6:
				/* CANBit Rate */
				for(int i = 0;*cansof != ',';i++)
				{
					gau8ConfigCANbitRate[i] = *cansof++;
				}
			break;

			case 7:
			{
			 //   printf("cansof = %s\n", cansof);
				parseCANID();
				gu32ExtractParamCounter++;

			}
			break;

			default:
			break;

		}// switch case end

		if(*cansof == ',')
		{
			gu32ExtractParamCounter++;
		}

		cansof++;

		if(gu32ExtractParamCounter >= 8)
		{
			break;
		}
	}

	gu32ExtractParamCounter = 0;
	verifyCANConfigData();
}
/******************************************************************************
* Function : verifyCANConfigData()
*//**
* \b Description:
*
* This function is used to varify CAN config from received remote config strings
*
* PRE-CONDITION:
*
* POST-CONDITION: None
*
* @return 		None.
*
* \b Example Example:
* @code
*
* 	verifyCANConfigData();
*
* @endcode
*
* @see
*
* <br><b> - HISTORY OF CHANGES - </b>
*
* <table align="left" style="width:800px">
* <tr><td> Date       </td><td> Software Version </td><td> Initials </td><td> Description </td></tr>
* <tr><td> 08/03/2022 </td><td> 0.0.1            </td><td> HL100133 </td><td> Interface Created </td></tr>
* </table><br><br>
* <hr>
*
*******************************************************************************/
void verifyCANConfigData(void)
{

	if(gu8CommType >= 3)
		Config_CANConfigErr |= (1 << enmCONFIG_COMMTYPE);
	else
		Config_CANConfigErr &= ~(1 << enmCONFIG_COMMTYPE);

	if(gu32CANCommType > 5)
		Config_CANConfigErr |= (1 << enmCONFIG_CANTYPE);
	else
		Config_CANConfigErr &= ~(1 << enmCONFIG_CANTYPE);

	if(gu32CANMode >= 2)
		Config_CANConfigErr |= (1 << enmCONFIG_CANMODE);
	else
		Config_CANConfigErr &= ~(1 << enmCONFIG_CANMODE);

	if(gu32CANBitRate > 1000 || gu32CANBitRate == 0)
		Config_CANConfigErr |= (1 << enmCONFIG_CANBITRATE);
	else
		Config_CANConfigErr &= ~(1 << enmCONFIG_CANBITRATE);

	if((gau32ConfigCANIDs_Count > MAX_CAN_IDS_SUPPORTED) && (gau32ConfigCANIDs_Count <= 0))
		Config_CANConfigErr |= (1 << enmCONFIG_CANNOOFCANID);
	else
		Config_CANConfigErr &= ~(1 << enmCONFIG_CANNOOFCANID);

	if(gau32TxConfigCANIDs_Count > MAX_Tx_CAN_IDS_SUPPORTED )
		Config_CANConfigErr |= (1 << enmCONFIG_CANNOOFTXCANID);
	else
		Config_CANConfigErr &= ~(1 << enmCONFIG_CANNOOFTXCANID);

}
