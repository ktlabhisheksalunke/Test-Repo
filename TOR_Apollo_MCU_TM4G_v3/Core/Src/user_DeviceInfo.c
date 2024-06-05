/*
 * user_DeviceInfo.c
 *
 *  Created on: Nov 6, 2023
 *      Author: abhisheks
 */
#include "user_DeviceInfo.h"

//#include "applicationdefines.h"
//#include "externs.h"

char dinfo[50] = {'0'};
char DPS_PaylodLenMsgBuff[60] = {0};


char buffuuid2[32] = {'0'};
uint32_t u32FlashSize = 0;

uint32_t gu32DeviceID = 0;

/****************************************************************************
 Function getDeviceSignature
 Purpose: Fetch Device UUID and Flash Size
 Input:	None.
 Return value: None .


 Note(s)(if-any) : Signature ASCII Array is updated

	 UUID -> 96 bit value
	 FlashSize -> 16 bit Value


 Change History:
 Author           	Date                Remarks
 KloudQ Team       11-04-18
 KloudQ Team	   19-09-18				Update . Flash Size removed from payload
******************************************************************************/
void updateDeviceSignature(void)
{

#if(USEMEMSIZEINDINFO == TRUE)
	char buffflash[16] = {'0'};

	u32FlashSize = (uint16_t)STM32_FLASHSIZE;
#endif

	gu32DeviceID = STM32_UUID[0];
	uint32_t u32UUID0 = 0;
	uint32_t u32UUID1 = 0;
	uint32_t u32UUID2 = 0;
	u32UUID0 = STM32_UUID[0];
	u32UUID1 = STM32_UUID[1];
	u32UUID2 = STM32_UUID[2];


#if(USEMEMSIZEINDINFO == TRUE)
	/*Integer to ASCII Flash Size */
	itoa(u32FlashSize,buffflash,10);
#endif
	/* Append Device Info to Array */
	itoa(u32UUID2,buffuuid2,10);
	strcpy(dinfo,buffuuid2);

	itoa(u32UUID1,buffuuid2,10);
	strcat(dinfo,buffuuid2);

	itoa(u32UUID0,buffuuid2,10);
	strcat(dinfo,buffuuid2);
#if(USEMEMSIZEINDINFO == TRUE)
	/* 16 bit Flash Size use if required */
	//strcat(dinfo,buffflash);
#endif

}

//******************************* End of File *******************************************************************
