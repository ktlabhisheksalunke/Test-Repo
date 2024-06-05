/*
 * user_HourMeter.h
 *
 *  Created on: Feb 29, 2024
 *      Author: abhisheks
 */

#ifndef INC_USER_HOURMETER_H_
#define INC_USER_HOURMETER_H_

#include <stdint.h>
#include <stm32l4xx.h>

#define NUM_HR_METER		3
#define RPM_HRM_CONFIGIDX (NUM_HR_METER-1)
#define RPM_HRM_THRESHIDX	(RPM_HRM_CONFIGIDX+1)

#define HR_METER_1ST_WR_CYCLE			((uint32_t)0)
#define HR_METER_2ND_WR_CYCLE			((uint32_t)1)


/********************************************/
//#define DPS_HRMETER_PORTS 				(int[]){DI_IN_1_uC_GPIO_Port, DI_IN_2_uC_GPIO_Port,	RPM_IN1_GPIO_Port}
//#define DPS_HRMETER_PINS	 				(int[]){DI_IN_1_uC_Pin, DI_IN_2_uC_Pin, RPM_IN1_Pin}
//#define DPS_HRMETER_EEPROMADDR	 	(int[]){EEPROM_PAGE3_ADD, EEPROM_PAGE4_ADD, EEPROM_PAGE5_ADD}

#define TOTAL_SEC_IN_ONE_MIN			(uint32_t)60
#define TOTAL_MS_IN_ONE_SEC				(uint32_t)1000

typedef struct{
	uint8_t En;

	uint32_t TotalHours;
	uint32_t TotalMin;

	uint8_t PinState;
	uint8_t Pin;						//unclear why

	volatile uint32_t RunningStatus;
	volatile uint32_t MinInFlag;
	volatile uint32_t MsCntr;
	volatile uint32_t SecCntr;

	GPIO_TypeDef* GPIOPort;
	uint16_t GPIOPin;
	uint16_t EEPROMloc;

}HrMeter_MStruct;

void HrMeter_ScanDigitalInputs(void);

void HrMeter_Routine(void);

extern  uint32_t gu32DI2;
extern uint32_t gu32DI1;
extern uint32_t gu32DI3;

//extern volatile uint32_t g_u32HrMtr1RunningStatus;
//extern volatile uint32_t g_u32HrMtr1_MinInFlag;
//extern volatile uint32_t g_u32HrMtr1_MsCntr;
//extern volatile uint32_t g_u32HrMtr1_SecCntr;
//
//extern volatile uint32_t g_u32HrMtr2RunningStatus;
//extern volatile uint32_t g_u32HrMtr2_MinInFlag;
//extern volatile uint32_t g_u32HrMtr2_MsCntr;
//extern volatile uint32_t g_u32HrMtr2_SecCntr;
//
//extern volatile uint32_t g_u32HrMtr3RunningStatus;
//extern volatile uint32_t g_u32HrMtr3_MinInFlag;
//extern volatile uint32_t g_u32HrMtr3_MsCntr;
//extern volatile uint32_t g_u32HrMtr3_SecCntr;

#endif /* INC_USER_HOURMETER_H_ */
