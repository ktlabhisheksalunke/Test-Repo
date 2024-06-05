/*
 * UserEeprom.h
 *
 *  Created on: Dec 9, 2022
 *      Author: anilm
 */

#ifndef INC_USEREEPROM_H_
#define INC_USEREEPROM_H_

/* Includes ------------------------------------------------------------------*/
#include "main.h"
//#include "digitalInputs.h"

#define CONFIG_AT24XX_SIZE 		(32)			/* Configure Used EEPROM memory */


/*AT24C08, 8K SERIAL EEPROM:
 * Internally organized with 64 pages of 16 bytes each,
 * the 8K requires a 10-bit data word address for random word addressing*/



#define PAGE_SIZE    16
#define PAGE_NUM     64

#define EEPROM_MEMORY_ADDRESS_SIZE					(I2C_MEMADD_SIZE_16BIT)
#define EEPROM_ADDRESS 								(0xA0)

#define EEPROM_PAGE1_ADD						(uint16_t)0x0000
#define EEPROM_PAGE2_ADD						(uint16_t)0x0010
#define EEPROM_PAGE3_ADD						(uint16_t)0x0020
#define EEPROM_PAGE4_ADD						(uint16_t)0x0030
#define EEPROM_PAGE5_ADD						(uint16_t)0x0040
#define EEPROM_PAGE6_ADD						(uint16_t)0x0050
#define EEPROM_PAGE7_ADD						(uint16_t)0x0060
#define EEPROM_PAGE8_ADD						(uint16_t)0x0070
#define EEPROM_PAGE9_ADD						(uint16_t)0x0080
#define EEPROM_PAGE10_ADD						(uint16_t)0x0090
#define EEPROM_PAGE11_ADD						(uint16_t)0x00A0
#define EEPROM_PAGE12_ADD						(uint16_t)0x00B0
#define EEPROM_PAGE13_ADD						(uint16_t)0x00C0
#define EEPROM_PAGE14_ADD						(uint16_t)0x00D0
#define EEPROM_PAGE15_ADD						(uint16_t)0x00E0
#define EEPROM_PAGE16_ADD						(uint16_t)0x00F0
#define EEPROM_PAGE17_ADD						(uint16_t)0x0100
#define EEPROM_PAGE18_ADD						(uint16_t)0x0110
#define EEPROM_PAGE19_ADD						(uint16_t)0x0120
#define EEPROM_PAGE20_ADD						(uint16_t)0x0130
#define EEPROM_PAGE21_ADD						(uint16_t)0x0140
#define EEPROM_PAGE22_ADD						(uint16_t)0x0150
#define EEPROM_PAGE23_ADD						(uint16_t)0x0160
#define EEPROM_PAGE24_ADD						(uint16_t)0x0170
#define EEPROM_PAGE25_ADD						(uint16_t)0x0180
#define EEPROM_PAGE26_ADD						(uint16_t)0x0190
#define EEPROM_PAGE27_ADD						(uint16_t)0x01A0
#define EEPROM_PAGE28_ADD						(uint16_t)0x01B0
#define EEPROM_PAGE29_ADD						(uint16_t)0x01C0
#define EEPROM_PAGE30_ADD						(uint16_t)0x01D0
#define EEPROM_PAGE31_ADD						(uint16_t)0x01E0
#define EEPROM_PAGE32_ADD						(uint16_t)0x01F0
#define EEPROM_PAGE33_ADD						(uint16_t)0x0200
#define EEPROM_PAGE34_ADD						(uint16_t)0x0210
#define EEPROM_PAGE35_ADD						(uint16_t)0x0220
#define EEPROM_PAGE36_ADD						(uint16_t)0x0230
#define EEPROM_PAGE37_ADD						(uint16_t)0x0240
#define EEPROM_PAGE38_ADD						(uint16_t)0x0250
#define EEPROM_PAGE39_ADD						(uint16_t)0x0260
#define EEPROM_PAGE40_ADD						(uint16_t)0x0270
#define EEPROM_PAGE41_ADD						(uint16_t)0x0280
#define EEPROM_PAGE42_ADD						(uint16_t)0x0290
#define EEPROM_PAGE43_ADD						(uint16_t)0x02A0
#define EEPROM_PAGE44_ADD						(uint16_t)0x02B0
#define EEPROM_PAGE45_ADD						(uint16_t)0x02C0
#define EEPROM_PAGE46_ADD						(uint16_t)0x02D0
#define EEPROM_PAGE47_ADD						(uint16_t)0x02E0
#define EEPROM_PAGE48_ADD						(uint16_t)0x02F0
#define EEPROM_PAGE49_ADD						(uint16_t)0x0300
#define EEPROM_PAGE50_ADD						(uint16_t)0x0310
#define EEPROM_PAGE51_ADD						(uint16_t)0x0320
#define EEPROM_PAGE52_ADD						(uint16_t)0x0330
#define EEPROM_PAGE53_ADD						(uint16_t)0x0340
#define EEPROM_PAGE54_ADD						(uint16_t)0x0350
#define EEPROM_PAGE55_ADD						(uint16_t)0x0360
#define EEPROM_PAGE56_ADD						(uint16_t)0x0370
#define EEPROM_PAGE57_ADD						(uint16_t)0x0380
#define EEPROM_PAGE58_ADD						(uint16_t)0x0390
#define EEPROM_PAGE59_ADD						(uint16_t)0x03A0
#define EEPROM_PAGE60_ADD						(uint16_t)0x03B0
#define EEPROM_PAGE61_ADD						(uint16_t)0x03C0
#define EEPROM_PAGE62_ADD						(uint16_t)0x03D0
#define EEPROM_PAGE63_ADD						(uint16_t)0x03E0
#define EEPROM_PAGE64_ADD						(uint16_t)0x03F0


void EEPROM_RetriveBuffMemData(void);
void EEPROM_WriteBuffMemData(void);

/*Internal Hour meter*/
typedef struct
{
	uint32_t u32TotalHours;
	uint32_t u32TotalMin;
}stHourMeterTime;

/*Internal Hour meter*/
typedef struct
{
	uint32_t u32BuffFlashReadAddress;
	uint32_t u32BuffFlashWriteAddress;
	uint32_t u32CheckSum;
}stEepromBuffMem;


extern stEepromBuffMem g_stEepromBuffMemAdd;


/* EEPROM Endurance
 * Endurance: 1 Million Write Cycles
 * Data Retention: 100 Years
 * We write after cross the minute. i.e we can write max 1 Million Minutes.
 *  1 Million Minutes. = 16666.66667 hours
 *  But We are considering only 16000 Hours. (for safety purpose)
 */
#define HOUR_METER_FIRST_LOC_MAX_CNT                (uint16_t)16000


//total 12 Bytes required for Buffer memory Add storing.
#define EEPROM_BUFF_MEM_ADD_DATA_COPY_1_ADD			EEPROM_PAGE1_ADD //(uint16_t)0x00  //0x00 to 0x0B	Buff Mem add First Copy of Buff mem Read And Wr Add with
																	//total 12 bytes 0 to 0x0B ( with 0 and 0x0B)
#define EEPROM_BUFF_MEM_ADD_DATA_COPY_2_ADD			EEPROM_PAGE2_ADD //(uint16_t)0x10 	//0x10 to 0x1B	Buff Mem add	Second Copy of Buff mem Read And Wr Add
																 	//total 12 bytes 0 to 0x0B ( with 0 and 0x0B)

void EEPROM_RetriveHrMeterData(uint8_t HrMeter_Sel, uint16_t HrMeter_eepromPage);
void EEPROM_WriteHrMeterData(uint8_t HrMeter_Sel, uint16_t HrMeter_eepromPage);




#endif /* INC_USEREEPROM_H_ */
