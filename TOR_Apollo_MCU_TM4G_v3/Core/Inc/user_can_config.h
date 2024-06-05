/*
 * user_can_config.h
 *
 *  Created on: Jun 17, 2023
 *      Author: vedantj
 */

#ifndef INC_USER_CAN_CONFIG_H_
#define INC_USER_CAN_CONFIG_H_

#include "user_can.h"

/**
* This enumeration is a list of CAN config errors
*/
typedef enum
{
	enmCONFIG_COMMTYPE = 1,
	enmCONFIG_CANTYPE,
	enmCONFIG_CANMODE,
	enmCONFIG_CANBITRATE,
	enmCONFIG_CANCSV,
	enmCONFIG_CANNOOFCANID,
	enmCONFIG_CANNOOFTXCANID,
}enmCANConfigErrors;

typedef enum{
	enmCANEXTRACT_SOF = 0,
	enmCANEXTRACT_DEVICEID,
	enmCANEXTRACT_COMMTYPE,
	enmCANEXTRACT_CAN,
	enmCANEXTRACT_CANCONFIGTYPE,
	enmCANEXTRACT_CANMODE,
	enmCANEXTRACT_CANBITRATE,

}enmCANExtractConfig;

typedef struct {
    int count;
    int ids[MAX_CAN_IDS_SUPPORTED];
} CANIDList;

typedef struct {
	int time;
	int id;
	int dataLength;
	char data[MAX_DATA_LENGTH];
} RequestedCANID;

//typedef struct {
//    int numBroadcastCANIDs;
//    CANIDList broadcastCANIDs;
//
//    int numRequestedCANIDs;
////    RequestedCANID requestedCANIDs[MAX_CAN_IDS_SUPPORTED];
//} ParsedData;

void verifyCANConfigData(void);
void extractCANConfiguration(void);
void copyCANData(void);
void parseCANID();

#endif /* INC_USER_CAN_CONFIG_H_ */
