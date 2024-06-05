/*
 * user_rtc.h
 *
 *  Created on: Jan 8, 2024
 *      Author: abhisheks
 */

#ifndef INC_USER_RTC_H_
#define INC_USER_RTC_H_

#include "user_HourMeter.h"
#include "stdint.h"

#define RTC_SIGNATURE_VAL   		(uint32_t)0xA5A5A5A5

//void RTC_extractDateTime(char * systempayload);

typedef enum{
	RTC_RoutineIdle = 0,
	RTC_SyncFromModule,
	RTC_SyncFromBkup,
}enmRTC_State;

void RTC_Routine(void);

void RTC_LoadBkpDateTime(void);
void RTC_getDateTime(void);

void RTC_UpdateBackupReg(void);
void RTC_PaylodTime(char * systemPayload);

void RTC_BackupHRMSec(uint8_t HrMeter_Sel, uint8_t u8Sec);
uint8_t RTC_GetHRMSecBkup(uint8_t HrMeter_Sel);

#endif /* INC_USER_RTC_H_ */
