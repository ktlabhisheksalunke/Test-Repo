/*
 * user_DeviceInfo.h
 *
 *  Created on: Nov 6, 2023
 *      Author: abhisheks
 */

#ifndef INC_USER_DEVICEINFO_H_
#define INC_USER_DEVICEINFO_H_

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "main.h"
#include "externs.h"
#include "user_ApplicationDefines.h"

#define STM32_UUID ((uint32_t *) 0x1FFF7590)						/* 96 bit Unique Identification ID */
#define STM32_FLASHSIZE (*((uint32_t *)  0x1FFF75E0))				/* 16 bit Flash Size */

#define USEMEMSIZEINDINFO (0)
typedef struct
{
	uint32_t u32UUID0;
	uint32_t u32UUID1;
	uint32_t u32UUID2;
	uint32_t u32FlashSize;
}strSTMDeviceSignature;

extern uint32_t u32FlashSize;
void updateDeviceSignature(void);

#endif /* INC_USER_DEVICEINFO_H_ */
