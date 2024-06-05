/*
 * user_rtc.c
 *
 *  Created on: Jan 8, 2024
 *      Author: abhisheks
 */

#ifndef SRC_USER_RTC_C_
#define SRC_USER_RTC_C_

//RTC_time
#include "user_rtc.h"

#include "string.h"
#include "stdlib.h"
#include "stdio.h"

#include "main.h"
#include "externs.h"
#include "user_queue.h"

RTC_TimeTypeDef RTC_time;
RTC_DateTypeDef RTC_date;

RTC_TimeTypeDef RTC_temptime;
RTC_DateTypeDef RTC_tempdate;

//static uint8_t RTC_BkupRead = 1;

char RTC_Date 	[3];
char RTC_Month 	[3];
char RTC_Year 	[3];
char RTC_Century 	[3];

char RTC_Hours 		[3];
char RTC_Minutes 	[3];
char RTC_Seconds 	[3];

enmRTC_State RTC_SyncState = RTC_RoutineIdle;

static int16_t RTC_compareDateTime(RTC_DateTypeDef RTC_currdate, RTC_TimeTypeDef RTC_currtime,RTC_DateTypeDef RTC_newdate, RTC_TimeTypeDef RTC_newtime);

void RTC_Routine(void){
	switch (RTC_SyncState) {
		case RTC_RoutineIdle:
			if (RTC_SyncTimer == 0) {
				RTC_SyncState = RTC_SyncFromModule;
//				RTC_SyncTimer = TEN_SEC; // commented by AJ 300524
				break;
			}
			break;

//		case RTC_SyncFromModule:
//			RTC_getDateTime();
//			if ((RTC_compareDateTime(RTC_date, RTC_time, RTC_tempdate, RTC_temptime) == -1)) {
//				if ((DPS_IsModuleAlive) && (DPS_NTPSyncFLag == 1)) {
//					RTC_SyncModuleNTP = 1;
//					RTC_SyncState = RTC_RoutineIdle;
//				}
//				else{
//					RTC_SyncState = RTC_SyncFromBkup;
//				}
//			}
//			else{
////				RTC_UpdateBackupReg();
//				RTC_SyncState = RTC_RoutineIdle;
//			}
//			break;
		case RTC_SyncFromModule:
			if((DPS_NTPSyncFLag == 1))
			{
				DPS_NTPSyncFLag = 0;
				RTC_getDateTime();
				if ((RTC_compareDateTime(RTC_date, RTC_time, RTC_tempdate, RTC_temptime) >= 300))
				{
					HAL_RTC_SetTime(&hrtc,&RTC_temptime,RTC_FORMAT_BIN);
					HAL_RTC_SetDate(&hrtc,&RTC_tempdate,RTC_FORMAT_BIN);
				}
			}
			RTC_SyncState = RTC_RoutineIdle;
			break;

		case RTC_SyncFromBkup:
			RTC_SyncState = RTC_RoutineIdle;
//			if (RTC_BkupRead) {
//				RTC_getDateTime();
//				RTC_LoadBkpDateTime();
//			}
//			RTC_BkupRead = 0;
			break;

		default:
			break;
	}
}

int16_t RTC_compareDateTime(RTC_DateTypeDef RTC_currdate, RTC_TimeTypeDef RTC_currtime,RTC_DateTypeDef RTC_newdate, RTC_TimeTypeDef RTC_newtime) {
//    if (RTC_currdate.Year > RTC_newdate.Year)
//        return 1;
//    else if (RTC_currdate.Year < RTC_newdate.Year)
//        return -1;
//    else {
//        if (RTC_currdate.Month > RTC_newdate.Month)
//            return 1;
//        else if (RTC_currdate.Month < RTC_newdate.Month)
//            return -1;
//        else {
//            if (RTC_currdate.Date > RTC_newdate.Date)
//                return 1;
//            else if (RTC_currdate.Date < RTC_newdate.Date)
//                return -1;
//            else {
//                if (RTC_currtime.Hours > RTC_newtime.Hours)
//                    return 1;
//                else if (RTC_currtime.Hours < RTC_newtime.Hours)
//                    return -1;
//                else {
//                    if (RTC_currtime.Minutes > RTC_newtime.Minutes)
//                        return 1;
//                    else if (RTC_currtime.Minutes < RTC_newtime.Minutes)
//                        return -1;
//                    else {
//                    		return 0; // Both date-times are equal
//                    }
//                }
//            }
//        }
//    }

	if((RTC_currdate.Year != RTC_newdate.Year)
		|| (RTC_currdate.Month != RTC_newdate.Month)
		|| (RTC_currdate.Date != RTC_newdate.Date))
		{
			return 300;
		}
		else
		{
			int Ref_total_seconds =  RTC_newtime.Hours * 3600 + RTC_newtime.Minutes * 60 + RTC_newtime.Seconds;
			int RTC_total_seconds = RTC_currtime.Hours * 3600 + RTC_currtime.Minutes * 60 + RTC_currtime.Seconds;
			if((RTC_total_seconds - Ref_total_seconds) < 0)
			{
				return ((RTC_total_seconds - Ref_total_seconds)*-1);
			}
			else
			{
				return (RTC_total_seconds - Ref_total_seconds);
			}
		}
}


void RTC_LoadBkpDateTime(void)
{
	HAL_PWR_EnableBkUpAccess();
	RTC_temptime.Seconds = HAL_RTCEx_BKUPRead(&hrtc,RTC_BKP_DR0);
	RTC_temptime.Minutes = HAL_RTCEx_BKUPRead(&hrtc,RTC_BKP_DR1);
	RTC_temptime.Hours = HAL_RTCEx_BKUPRead(&hrtc,RTC_BKP_DR2);

	RTC_tempdate.Date = HAL_RTCEx_BKUPRead(&hrtc,RTC_BKP_DR3);
	RTC_tempdate.Month = HAL_RTCEx_BKUPRead(&hrtc,RTC_BKP_DR4);
	RTC_tempdate.Year = HAL_RTCEx_BKUPRead(&hrtc,RTC_BKP_DR5);

	if (RTC_compareDateTime(RTC_date, RTC_time, RTC_tempdate, RTC_temptime) == -1) {
		/*Update RTC with fetched data */
		HAL_RTC_SetTime(&hrtc,&RTC_time,RTC_FORMAT_BIN);
		HAL_RTC_SetDate(&hrtc,&RTC_date,RTC_FORMAT_BIN);
	}

	HAL_PWR_DisableBkUpAccess();
}

void RTC_getDateTime(void){
	HAL_RTC_GetTime(&hrtc,&RTC_time,RTC_FORMAT_BIN);
	HAL_RTC_GetDate(&hrtc,&RTC_date,RTC_FORMAT_BIN);
}

void RTC_UpdateBackupReg(void)
{
	/* Update Backup Registers */
	HAL_PWR_EnableBkUpAccess();
	HAL_RTCEx_BKUPWrite(&hrtc , RTC_BKP_DR0 ,RTC_time.Hours);
	HAL_RTCEx_BKUPWrite(&hrtc , RTC_BKP_DR1 ,RTC_time.Minutes);
	HAL_RTCEx_BKUPWrite(&hrtc , RTC_BKP_DR2 ,RTC_time.Seconds);
	HAL_RTCEx_BKUPWrite(&hrtc , RTC_BKP_DR3 ,RTC_date.Date);
	HAL_RTCEx_BKUPWrite(&hrtc , RTC_BKP_DR4 ,RTC_date.Month);
	HAL_RTCEx_BKUPWrite(&hrtc , RTC_BKP_DR5 ,RTC_date.Year);
	HAL_PWR_DisableBkUpAccess();
}

void RTC_PaylodTime(char * systemPayload)
{
	uint8_t convBuff[20] = {0};

	HAL_RTC_GetTime(&hrtc,&RTC_time,RTC_FORMAT_BIN);
	HAL_RTC_GetDate(&hrtc,&RTC_date,RTC_FORMAT_BIN);

//	RTC_UpdateBackupReg();

//	Update_Backup_Reg(&SDate1,&RTC_time);


//	strcat((char *)systemPayload,(char * )"20");
	memset(convBuff, 0, sizeof(convBuff));
	sprintf((char * )convBuff,"%02d",RTC_date.Date);
	strcat((char *)systemPayload,(char * )convBuff);
	strcat((char *)systemPayload,(char * )"/");

	memset(convBuff, 0, sizeof(convBuff));
	sprintf((char * )convBuff,"%02d",RTC_date.Month);
	strcat((char *)systemPayload,(char * )convBuff);
	strcat((char *)systemPayload,(char * )"/");

	strcat((char *)systemPayload,(char * )RTC_Century);
	memset(convBuff, 0, sizeof(convBuff));
	sprintf((char * )convBuff,"%02d",RTC_date.Year);
	strcat((char *)systemPayload,(char * )convBuff);
	strcat((char *)systemPayload,(char * )" ");

	memset(convBuff, 0, sizeof(convBuff));
	sprintf((char * )convBuff,"%02d",RTC_time.Hours);
	strcat((char *)systemPayload,(char * )convBuff);
	strcat((char *)systemPayload,(char * )":");

	memset(convBuff, 0, sizeof(convBuff));
	sprintf((char * )convBuff,"%02d",RTC_time.Minutes);
	strcat((char *)systemPayload,(char * )convBuff);
	strcat((char *)systemPayload,(char * )":");

	memset(convBuff, 0, sizeof(convBuff));
	sprintf((char * )convBuff,"%02d",RTC_time.Seconds);
	strcat((char *)systemPayload,(char * )convBuff);

//  RTC IS NOT UPDATED
//	if(RTC_date.Year < 21)
//	{
//		/*Default Date Time*/
//		strcat((char *)systemPayload,(char * )TIME_STRING1);
//	}
//	else
//	{
//
//	}
	strcat((char *)systemPayload,(char * )",");

}//end of void PaylodTime(char * systemPayload)

uint32_t RTC_read_backup_reg(uint32_t BackupRegister)
{
    RTC_HandleTypeDef RtcHandle;
    RtcHandle.Instance = RTC;
    return HAL_RTCEx_BKUPRead(&RtcHandle, BackupRegister);
}

void RTC_BackupHRMSec(uint8_t HrMeter_Sel, uint8_t u8Sec)
{
	/*This will never happen but we added cross check for safety purpose*/
	if(u8Sec > 60)
	{	u8Sec = 60;	}
	 /*Write Invalid Signature before Write*/
	 HAL_RTCEx_BKUPWrite(&hrtc,3 + (HrMeter_Sel * 2),0);
	/* Hour meter Second value */
	uint32_t Data = 0;
	Data = (uint32_t)u8Sec;
	HAL_RTCEx_BKUPWrite(&hrtc,4 + (HrMeter_Sel * 2),Data);
	/* Update backup register with sign */
	HAL_RTCEx_BKUPWrite(&hrtc,3 + (HrMeter_Sel * 2),RTC_SIGNATURE_VAL);/*Write Valid Signature before Write*/
}

uint8_t RTC_GetHRMSecBkup(uint8_t HrMeter_Sel)
{
	uint8_t u8SecondValue = 0;
	uint32_t u32RtcSignatureVal = 0;
	uint32_t Data = 0;
	/* Get date */
	Data = RTC_read_backup_reg(4 + (HrMeter_Sel*2));
	HAL_Delay(10);
	/*If Backup Reg signature is match then write all reg*/
	/* Get Sign */
	u32RtcSignatureVal = RTC_read_backup_reg(3 + (HrMeter_Sel*2));
	if(u32RtcSignatureVal == RTC_SIGNATURE_VAL)
  	{
			if(Data <= 60)
			{
				u8SecondValue = Data;
			}
  	}

	return u8SecondValue;
}

#endif /* SRC_USER_RTC_C_ */
