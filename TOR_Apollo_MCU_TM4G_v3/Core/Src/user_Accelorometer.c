/*
  *****************************************************************************
  * @file    	Accelorometer.c
  * @author  	Anil More
  * @version
  * @date		06/01/2023
  * @brief   	Functions for Accessing MXC3420AL G-Sensor, conversing the data,
 	 	 	 	 and generate real time flags, and calibrate G-Sensor
  *
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

*/

#include <user_Accelorometer.h>
#include "main.h"
#include "user_rtc.h"
#include "string.h"
#include "stdlib.h"
#include "stdio.h"
#include "externs.h"
//#include  "i2c.h"
//#include  "timer.h"
#include  "payload.h"
#include "user_config.h"

static void Acc_CalculateTiltAngles(stAccData_t *pst_AccData);

static int32_t sensor_convert_data(uint8_t high_byte, uint8_t low_byte);
void GenrateAccPayload(stAccData_t *pstAccelratData);



volatile uint32_t gu32SensorVheInMotionTmr;
volatile uint32_t gu32SensorFsmMotionTmr;
stAccData_t g_stDeviceAccLiveData;
stAccData_t g_stDeviceAccOffsetData;

/*Accelerometer Signed Data in Positive or Negative*/

int32_t g_i32LatAxisValue = 0;  //X
int32_t g_i32LongAxisValue = 0;  //Y
int32_t g_i32VerticleAxisValue = 0; //Z

int32_t g_i16AccTemperatureRawVal = 0;
float_t g_f32AccTeperature = 0;
uint8_t g_u8ShakeDetIntPinFlag = 0;

uint8_t u8I2CWriteBuff[15] = {0};
uint8_t u8I2CReadBuff[15] = {0};

st_DeviceConfigCalibData g_DeviceConfigCalibData;

uint8_t g_u8AccCalibratemode = 0;
uint8_t g_u8GSensCalibState = 0;

/******************* Start This variable Will use during Calibration purpose Only.******************************************************************/
/*X axis Values in m/s^2. 1000 Multiplier */
int32_t i32LongAxisCalibRawVAlue[ACC_CALIB_SAMPLE_LEN];
int32_t i32LongAxisCalibAvgValue;
int32_t i32LongAxisCalibMinValue;
int32_t i32LongAxisCalibMaxValue;
/*Y axis Values in m/s^2. 1000 Multiplier */
int32_t i32LatAxisCalibRawVAlue[ACC_CALIB_SAMPLE_LEN];
int32_t i32LatAxisCalibAvgValue;
int32_t i32LatAxisCalibMinValue;
int32_t i32LatAxisCalibMaxValue;
/*Z axis Values in m/s^2. 1000 Multiplier */
int32_t i32VerticleAxisCalibRawVAlue[ACC_CALIB_SAMPLE_LEN];
int32_t i32VerticleAxisCalibAvgValue;
int32_t i32VerticleAxisCalibMinValue;
int32_t i32VerticleAxiCalibsMaxValue;

uint32_t u32CalibSampleIndex = 0;

char Config_AccCaliberation [128];

static e_GSensorState s_egState = E_G_SENSOR_STATE_IDLE;
static uint8_t s_u8SensorConfig = 0;

// *,IMEI,SET,GSENSOR__,

int32_t ACC_VehInMotionLimit_MPS = 0;								//1
float_t ACC_XYVehGradientLimit_Normal = 0.00;				//2
float_t ACC_XYVehGradientLimit_Extreme = 0.00;			//3
float_t ACC_XYVehGradientMaxVal = 0.00;							//4

int32_t ACC_SuddenAccLimit_Max = 0;									//5
int32_t ACC_SuddenAccLimit_Extreme = 0;							//6
int32_t ACC_SuddenAccLimit_High = 0;								//7

int32_t ACC_SuddenBrakeLimit_Max = 0;								//8
int32_t ACC_SuddenBrakeLimit_Extreme = 0;						//9
int32_t ACC_SuddenBrakeLimit_High = 0;							//10

int32_t ACC_SuddenImpactLimit = 0;									//11

float_t ACC_DeviceTamperAngle = 0;									//12


/******************* End This Values Will use for during Calibration purpose Only.******************************************************************/

/**********************************************************************************
 Function Name: 	G_SensorFSM
 Purpose:			Runs from main to process for update G-Sensor Parameters
 Input:				None.
 Return value: 		None.

 Change History:
 Author: Anil More         Date:   06/01/2023           Remarks

Update on 	11/04/2023 - Added logic for Flag generation
			15/05/2023 - G-Sensor Calibration logic
************************************************************************************/
void G_SensorFSM(void)
{

	switch(s_egState)
	{
		case E_G_SENSOR_STATE_IDLE:
		{
			if(gu32SensorFsmMotionTmr == 0)
			{
				if(s_u8SensorConfig == 0)
				{
					s_egState = E_G_SENSOR_STATE_CONFIGURE;
				}
				else
				{
					s_egState = E_G_SENSOR_STATE_READDATA;
				}
			}
		}break;
		case E_G_SENSOR_STATE_CONFIGURE:
		{
			memset(u8I2CWriteBuff,0x00,sizeof(u8I2CWriteBuff));
			/*1.Switch to WAKE state and stay there. (by Default Chip in Steady state )*/
			u8I2CWriteBuff[0] =  0x00;//WAKE. Clocks are running and X, Y, and Z-axis data are acquired at the sample rate.
			HAL_I2C_Mem_Write(&hi2c1, ACC_SLAVE_ID_MXC3420AL, MXC3420AL_MODE_REG_WR_ADD, I2C_MEMADD_SIZE_8BIT, (uint8_t* )&u8I2CWriteBuff, 1,100);

			/*2.Configure the accelerometer Range or FSR*/
			u8I2CWriteBuff[0] = ACC_MXC3420AL_CNTRL_REG_VAL;
			HAL_I2C_Mem_Write(&hi2c1, ACC_SLAVE_ID_MXC3420AL, MXC3420AL_FSR_REG_WR_ADD, I2C_MEMADD_SIZE_8BIT, (uint8_t* )&u8I2CWriteBuff, 1,100);

			/*3.Switch to WAKE state and stay there. (by Default Chip in Steady state )*/
			u8I2CWriteBuff[0] =  0x01;//WAKE. Clocks are running and X, Y, and Z-axis data are acquired at the sample rate.
			HAL_I2C_Mem_Write(&hi2c1, ACC_SLAVE_ID_MXC3420AL, MXC3420AL_MODE_REG_WR_ADD, I2C_MEMADD_SIZE_8BIT, (uint8_t* )&u8I2CWriteBuff, 1,100);

			s_egState = E_G_SENSOR_STATE_IDLE;
			gu32SensorFsmMotionTmr = ONEHUNDRED_MS;
			s_u8SensorConfig =1;

		}break;
		case E_G_SENSOR_STATE_READDATA:
		{
			uint16_t u16TempVar= 0;

			memset(u8I2CReadBuff,0x00,sizeof(u8I2CReadBuff));

			memset(u8I2CReadBuff,0x00,sizeof(u8I2CReadBuff));
			HAL_I2C_Mem_Read(&hi2c1, ACC_SLAVE_ID_MXC3420AL, MXC3420AL_XOUT_EX_L_RD_ADD, I2C_MEMADD_SIZE_8BIT, (uint8_t* )&u8I2CReadBuff, 6,1000);

//			g_i32XAxisValue = sensor_convert_data(u8I2CReadBuff[1], u8I2CReadBuff[0]);  //X
//			g_i32YAxisValue = sensor_convert_data(u8I2CReadBuff[3], u8I2CReadBuff[2]);//y
//			g_i32ZAxisValue = sensor_convert_data(u8I2CReadBuff[5], u8I2CReadBuff[4]);//z
//

			g_i32LatAxisValue = sensor_convert_data(u8I2CReadBuff[1], u8I2CReadBuff[0]);  //X

			if(g_DeviceConfigCalibData.e_DeviceMountingPos == E_G_SENSOR_VERTICAL)
			{
				g_i32VerticleAxisValue = sensor_convert_data(u8I2CReadBuff[3], u8I2CReadBuff[2]);//y
				g_i32LongAxisValue = sensor_convert_data(u8I2CReadBuff[5], u8I2CReadBuff[4]);//z
				g_i32LongAxisValue *= (-1);
			}
			else
			{
				g_i32LongAxisValue = sensor_convert_data(u8I2CReadBuff[3], u8I2CReadBuff[2]);//y
				g_i32VerticleAxisValue = sensor_convert_data(u8I2CReadBuff[5], u8I2CReadBuff[4]);//z
			}

			/*Accelerometer sensor Temperature.*/
			{
				u16TempVar = (uint16_t)u8I2CReadBuff[6];
				if(u16TempVar > 0x07)
				{
					u16TempVar |= 0xFFF0;
					g_i16AccTemperatureRawVal = u16TempVar;
				}
				else
				{
					g_i16AccTemperatureRawVal = u16TempVar;
				}
				g_f32AccTeperature =  ((float_t)g_i16AccTemperatureRawVal*(float_t)0.586);
				g_f32AccTeperature += (float_t)25.0;
			}

			/*Add offset and if the result is between min val and max val then consider it as 0*/
			g_i32LatAxisValue -= g_DeviceConfigCalibData.i32LatAxisOffset;
			g_i32LongAxisValue -= g_DeviceConfigCalibData.i32LongAxisOffset;
			g_i32VerticleAxisValue -= g_DeviceConfigCalibData.i32VerticleAxisOffset;

			/*Fill Raw Value in data*/
			g_stDeviceAccLiveData.i32LatAxisRawVAlue[g_stDeviceAccLiveData.u16SampleIndex] = g_i32LatAxisValue;
			g_stDeviceAccLiveData.i32LongAxisRawVAlue[g_stDeviceAccLiveData.u16SampleIndex] = g_i32LongAxisValue;
			g_stDeviceAccLiveData.i32VerticleAxisRawVAlue[g_stDeviceAccLiveData.u16SampleIndex] = g_i32VerticleAxisValue;

			/*Check min value*/
			if(g_stDeviceAccLiveData.i32LongAxisMinValue > g_i32LongAxisValue){g_stDeviceAccLiveData.i32LongAxisMinValue = g_i32LongAxisValue;}
			if(g_stDeviceAccLiveData.i32LatAxisMinValue > g_i32LatAxisValue){g_stDeviceAccLiveData.i32LatAxisMinValue = g_i32LatAxisValue;}
			if(g_stDeviceAccLiveData.i32VerticleAxisMinValue > g_i32VerticleAxisValue){g_stDeviceAccLiveData.i32VerticleAxisMinValue = g_i32VerticleAxisValue;}

			/*Check Max value*/
			if(g_stDeviceAccLiveData.i32LongAxisMaxValue < g_i32LongAxisValue){g_stDeviceAccLiveData.i32LongAxisMaxValue = g_i32LongAxisValue;}
			if(g_stDeviceAccLiveData.i32LatAxisMaxValue < g_i32LatAxisValue){g_stDeviceAccLiveData.i32LatAxisMaxValue = g_i32LatAxisValue;}
			if(g_stDeviceAccLiveData.i32VerticleAxisMaxValue < g_i32VerticleAxisValue){g_stDeviceAccLiveData.i32VerticleAxisMaxValue = g_i32VerticleAxisValue;}


			g_stDeviceAccLiveData.u16SampleIndex++;

			if(g_stDeviceAccLiveData.u16SampleIndex >= ACC_SAMPLE_LEN)
			{
				int32_t i32TempVar = 0;
				//10*100 = 1000ms = 1Second
				g_stDeviceAccLiveData.u16SampleIndex = 0;
				uint32_t u32loopCntr = 0;
				int32_t i32XTotal=0,i32YTotal=0,i32ZTotal=0;
				for(u32loopCntr = 0; u32loopCntr< ACC_SAMPLE_LEN; u32loopCntr++)
				{
					i32XTotal += g_stDeviceAccLiveData.i32LongAxisRawVAlue[u32loopCntr];
					i32YTotal += g_stDeviceAccLiveData.i32LatAxisRawVAlue[u32loopCntr];
					i32ZTotal += g_stDeviceAccLiveData.i32VerticleAxisRawVAlue[u32loopCntr];
				}
				g_stDeviceAccLiveData.i32LongAxisAvgValue = i32XTotal/(int32_t)ACC_SAMPLE_LEN;
				g_stDeviceAccLiveData.i32LatAxisAvgValue = i32YTotal/(int32_t)ACC_SAMPLE_LEN;
				g_stDeviceAccLiveData.i32VerticleAxisAvgValue = i32ZTotal/(int32_t)ACC_SAMPLE_LEN;


				/*Calculate tilt angle*/
				Acc_CalculateTiltAngles(&g_stDeviceAccLiveData);

				/*Calibrate the Accelerometer */
				/* 	Anil More. 11/04/2023
				 *  assign value as per forward direction of vehicle or device mounting position.
				 *
				 * calibration mounting position will receive in calibration message (which will receive from Configuration Topic).
				 *
				 * **************************************************************************************************************************
				 * Calibration Message format as per below.
				 * "$,054061957514975180812293845,GC,1,@"
				 *  $- Start of Configuration string.
				 *  054061957514975180812293845 - Device ID (message is for Specific device).
				 *  GC - This Configuration message for G-sensor Calibration.
				 *  1 - Device Mounting position.
				 *  	* if 1- Device is Mounted on Horizontal surface. (Y axis is the axis of Moment).
				 *  			* If Vehicle Move in forward Direction then Y axis Increase in +ve direction and again stable at Normal value.
				 *  			for Ex. Vehicle is stand, at that time Y axis Value is 0.002 m/s^2, And then Vehicle is moved in
				 *  			forward direction that time Y-axis Value goes to 4.129 m/s^2 and again it is stabled at Old Value (0.002 m/s^2).
				 *
				 *  			* If Vehicle Move in Reversed Direction then Y axis value goes in -ve direction and again stable at Normal value.
				 *  			* Z axis will be Perpendicular to earth axis (9.8 Value).
				 *
				 *  	* if 2- Device is Mounted on Vertical Surface. (Z axis is the axis of Moment).
				 *  			* If Vehicle Move in forward Direction then Z axis Increase in +ve direction and again stable at Normal value.
				 *  			for Ex. Vehicle is stand at that time Z axis Value is 0.002 m/s^2, And then Vehicle is moved in
				 *  			forward direction that time Z-axis Value goes to 4.129 m/s^2 and again it is stabled at Old Value (0.002 m/s^2).
				 *
				 *  			* If Vehicle Move in Reversed Direction thenZY axis value goes in -ve direction and again stable at Normal value
				 *  			* Y axis will be Perpendicular to earth axis (9.8 Value).
				 *
				 *  @- End of Configuration string
				 *******************************************************************************************************************************
				 */

				if(g_u8AccCalibratemode)
				{
					if(G_SESN_CALIB_START == g_u8GSensCalibState)
					{
						memset(i32LongAxisCalibRawVAlue,0x00,(sizeof(i32LongAxisCalibRawVAlue)));
						memset(i32LatAxisCalibRawVAlue,0x00,(sizeof(i32LatAxisCalibRawVAlue)));
						memset(i32VerticleAxisCalibRawVAlue,0x00,(sizeof(i32VerticleAxisCalibRawVAlue)));

						i32LongAxisCalibMinValue= g_stDeviceAccLiveData.i32LongAxisMinValue;
						i32LongAxisCalibMaxValue=g_stDeviceAccLiveData.i32LongAxisMaxValue;

						i32LatAxisCalibMinValue=g_stDeviceAccLiveData.i32LatAxisMinValue;
						i32LatAxisCalibMaxValue=g_stDeviceAccLiveData.i32LatAxisMaxValue;;

						i32VerticleAxisCalibMinValue = g_stDeviceAccLiveData.i32VerticleAxisMinValue;
						i32VerticleAxiCalibsMaxValue = g_stDeviceAccLiveData.i32VerticleAxisMaxValue;

						u32CalibSampleIndex = 0;
						//Ignore this Reading  and start calibration algorithm from next reading
						g_u8GSensCalibState = G_SESN_CALIB_INPROGRESS;
						break;
					}
					else if(G_SESN_CALIB_INPROGRESS == g_u8GSensCalibState)
					{
						//TODO Start Calibration cycle for 15 seconds
						/*15 samples cycle 1Sec * 15 = 15 seconds*
						 * Get Min And Max Values of Each axis in 15 Seconds
						 * Calculate average of each axis vales in 15 Seconds.
						 */

						/*First read 15 cycle  as it is all axis value
						 * then calculate OFFSET for Each axis
						 * Minus Offset from minimum value and max value
						 * Save minimum and Max Value for Each axis.
						 * Use minimum and max values for calculations
						 *
						 * */
						i32LatAxisCalibRawVAlue[u32CalibSampleIndex] = g_stDeviceAccLiveData.i32LatAxisAvgValue;
						i32LongAxisCalibRawVAlue[u32CalibSampleIndex] = g_stDeviceAccLiveData.i32LongAxisAvgValue;
						i32VerticleAxisCalibRawVAlue[u32CalibSampleIndex] = g_stDeviceAccLiveData.i32VerticleAxisAvgValue;

						if(i32LongAxisCalibMinValue > g_stDeviceAccLiveData.i32LongAxisMinValue )
						{	i32LongAxisCalibMinValue = g_stDeviceAccLiveData.i32LongAxisMinValue;}
						if(i32LatAxisCalibMinValue > g_stDeviceAccLiveData.i32LatAxisMinValue)
						{	i32LatAxisCalibMinValue = g_stDeviceAccLiveData.i32LatAxisMinValue;}
						if(i32VerticleAxisCalibMinValue > g_stDeviceAccLiveData.i32VerticleAxisMinValue )
						{	i32VerticleAxisCalibMinValue = g_stDeviceAccLiveData.i32VerticleAxisMinValue;}

						if(i32LongAxisCalibMaxValue < g_stDeviceAccLiveData.i32LongAxisMaxValue )
						{	i32LongAxisCalibMaxValue = g_stDeviceAccLiveData.i32LongAxisMaxValue;}
						if(i32LatAxisCalibMaxValue < g_stDeviceAccLiveData.i32LatAxisMaxValue)
						{	i32LatAxisCalibMaxValue = g_stDeviceAccLiveData.i32LatAxisMaxValue;}
						if(i32VerticleAxiCalibsMaxValue < g_stDeviceAccLiveData.i32VerticleAxisMaxValue )
						{	i32VerticleAxiCalibsMaxValue = g_stDeviceAccLiveData.i32VerticleAxisMaxValue;}

						u32CalibSampleIndex++;
						if(u32CalibSampleIndex >= ACC_CALIB_SAMPLE_LEN)
						{
							i32LongAxisCalibAvgValue=0;
							i32LatAxisCalibAvgValue=0;
							i32VerticleAxisCalibAvgValue = 0;
							int32_t i32XTotal=0,i32YTotal=0,i32ZTotal=0;

							for(u32loopCntr = 0; u32loopCntr< ACC_CALIB_SAMPLE_LEN; u32loopCntr++)
							{
								i32XTotal += i32LatAxisCalibRawVAlue[u32loopCntr];
								i32YTotal	 += i32LongAxisCalibRawVAlue[u32loopCntr];
								i32ZTotal += i32VerticleAxisCalibRawVAlue[u32loopCntr];
							}


							i32LatAxisCalibAvgValue = i32XTotal/(int32_t)ACC_CALIB_SAMPLE_LEN;
							i32LongAxisCalibAvgValue = i32YTotal/(int32_t)ACC_CALIB_SAMPLE_LEN;
							i32VerticleAxisCalibAvgValue = i32ZTotal/(int32_t)ACC_CALIB_SAMPLE_LEN;

							g_DeviceConfigCalibData.i32LatAxisMinValue = i32LatAxisCalibMinValue - i32LatAxisCalibAvgValue;
							g_DeviceConfigCalibData.i32LatAxisMaxValue = i32LatAxisCalibMaxValue-i32LatAxisCalibAvgValue;

//							g_DeviceConfigCalibData.i32LongAxisMinValue = i32LongAxisCalibMinValue -i32LongAxisCalibAvgValue;
//							g_DeviceConfigCalibData.i32VerticleAxisMinValue = i32VerticleAxisCalibMinValue - i32VerticleAxisCalibAvgValue;
//
//							g_DeviceConfigCalibData.i32LongAxisMaxValue = i32LongAxisCalibMaxValue - i32LongAxisCalibAvgValue;
//							g_DeviceConfigCalibData.i32VerticleAxisMaxValue = i32VerticleAxiCalibsMaxValue - i32VerticleAxisCalibAvgValue;


							g_DeviceConfigCalibData.i32LatAxisOffset = (i32LatAxisCalibAvgValue+g_DeviceConfigCalibData.i32LatAxisOffset); //- 9806;
							if(g_u8AccCalibratemode == 2)
							{
								if(g_DeviceConfigCalibData.e_DeviceMountingPos != E_G_SENSOR_VERTICAL)
								{
									i32TempVar = g_DeviceConfigCalibData.i32VerticleAxisOffset;
									g_DeviceConfigCalibData.i32VerticleAxisOffset = (i32LongAxisCalibAvgValue+g_DeviceConfigCalibData.i32LongAxisOffset); //- 9806;
									g_DeviceConfigCalibData.i32LongAxisOffset 	 = (i32VerticleAxisCalibAvgValue+i32TempVar);
									g_DeviceConfigCalibData.i32LongAxisOffset *= (-1);

									g_DeviceConfigCalibData.i32VerticleAxisMinValue = i32LongAxisCalibMinValue -i32LongAxisCalibAvgValue;
									g_DeviceConfigCalibData.i32LongAxisMinValue = i32VerticleAxisCalibMinValue - i32VerticleAxisCalibAvgValue;

									g_DeviceConfigCalibData.i32VerticleAxisMaxValue = i32LongAxisCalibMaxValue - i32LongAxisCalibAvgValue;
									g_DeviceConfigCalibData.i32LongAxisMaxValue = i32VerticleAxiCalibsMaxValue - i32VerticleAxisCalibAvgValue;


									g_DeviceConfigCalibData.e_DeviceMountingPos = E_G_SENSOR_VERTICAL;
								}
								else
								{
									g_DeviceConfigCalibData.i32LongAxisOffset = (i32LongAxisCalibAvgValue+g_DeviceConfigCalibData.i32LongAxisOffset);
									g_DeviceConfigCalibData.i32VerticleAxisOffset = (i32VerticleAxisCalibAvgValue+g_DeviceConfigCalibData.i32VerticleAxisOffset);

									g_DeviceConfigCalibData.i32LongAxisMinValue = i32LongAxisCalibMinValue -i32LongAxisCalibAvgValue;
									g_DeviceConfigCalibData.i32VerticleAxisMinValue = i32VerticleAxisCalibMinValue - i32VerticleAxisCalibAvgValue;

									g_DeviceConfigCalibData.i32LongAxisMaxValue = i32LongAxisCalibMaxValue - i32LongAxisCalibAvgValue;
									g_DeviceConfigCalibData.i32VerticleAxisMaxValue = i32VerticleAxiCalibsMaxValue - i32VerticleAxisCalibAvgValue;


									g_DeviceConfigCalibData.e_DeviceMountingPos = E_G_SENSOR_VERTICAL;
								}
							}
							else
							{
								if(g_DeviceConfigCalibData.e_DeviceMountingPos == E_G_SENSOR_VERTICAL)
								{
									i32TempVar = g_DeviceConfigCalibData.i32VerticleAxisOffset;
									g_DeviceConfigCalibData.i32VerticleAxisOffset = (i32LongAxisCalibAvgValue+g_DeviceConfigCalibData.i32LongAxisOffset); //- 9806;
									g_DeviceConfigCalibData.i32LongAxisOffset 	 = (i32VerticleAxisCalibAvgValue+i32TempVar);
									g_DeviceConfigCalibData.i32VerticleAxisOffset *= (-1);

									g_DeviceConfigCalibData.i32VerticleAxisMinValue = i32LongAxisCalibMinValue -i32LongAxisCalibAvgValue;
									g_DeviceConfigCalibData.i32LongAxisMinValue = i32VerticleAxisCalibMinValue - i32VerticleAxisCalibAvgValue;

									g_DeviceConfigCalibData.i32VerticleAxisMaxValue = i32LongAxisCalibMaxValue - i32LongAxisCalibAvgValue;
									g_DeviceConfigCalibData.i32LongAxisMaxValue = i32VerticleAxiCalibsMaxValue - i32VerticleAxisCalibAvgValue;


									g_DeviceConfigCalibData.e_DeviceMountingPos = E_G_SENSOR_HORIZONTICAL;
								}
								else
								{
									g_DeviceConfigCalibData.i32LongAxisOffset = (i32LongAxisCalibAvgValue+g_DeviceConfigCalibData.i32LongAxisOffset);
									g_DeviceConfigCalibData.i32VerticleAxisOffset = (i32VerticleAxisCalibAvgValue+g_DeviceConfigCalibData.i32VerticleAxisOffset); //- 9806;

									g_DeviceConfigCalibData.i32LongAxisMinValue = i32LongAxisCalibMinValue -i32LongAxisCalibAvgValue;
									g_DeviceConfigCalibData.i32VerticleAxisMinValue = i32VerticleAxisCalibMinValue - i32VerticleAxisCalibAvgValue;

									g_DeviceConfigCalibData.i32LongAxisMaxValue = i32LongAxisCalibMaxValue - i32LongAxisCalibAvgValue;
									g_DeviceConfigCalibData.i32VerticleAxisMaxValue = i32VerticleAxiCalibsMaxValue - i32VerticleAxisCalibAvgValue;

									g_DeviceConfigCalibData.e_DeviceMountingPos = E_G_SENSOR_HORIZONTICAL;
								}
							}

							g_u8AccCalibratemode = 0;
							g_u8GSensCalibState = G_SESN_CALIB_DONE;
//							EEPROM_WriteDeviceConfigData();
							char temp[16];
							memset(Config_AccCaliberation,0,sizeof(Config_AccCaliberation));
							itoa(g_DeviceConfigCalibData.e_DeviceMountingPos,temp,10);
							strcpy(Config_AccCaliberation,temp);
							strcat(Config_AccCaliberation,(char *)",");
							itoa(g_DeviceConfigCalibData.i32LatAxisOffset,temp,10);
							strcat(Config_AccCaliberation,temp);
							strcat(Config_AccCaliberation,(char *)",");
							itoa(g_DeviceConfigCalibData.i32LatAxisMinValue,temp,10);
							strcat(Config_AccCaliberation,temp);
							strcat(Config_AccCaliberation,(char *)",");
							itoa(g_DeviceConfigCalibData.i32LatAxisMaxValue,temp,10);
							strcat(Config_AccCaliberation,temp);
							strcat(Config_AccCaliberation,(char *)",");
							itoa(g_DeviceConfigCalibData.i32LongAxisOffset,temp,10);
							strcat(Config_AccCaliberation,temp);
							strcat(Config_AccCaliberation,(char *)",");
							itoa(g_DeviceConfigCalibData.i32LongAxisMinValue,temp,10);
							strcat(Config_AccCaliberation,temp);
							strcat(Config_AccCaliberation,(char *)",");
							itoa(g_DeviceConfigCalibData.i32LongAxisMaxValue,temp,10);
							strcat(Config_AccCaliberation,temp);
							strcat(Config_AccCaliberation,(char *)",");
							itoa(g_DeviceConfigCalibData.i32VerticleAxisOffset,temp,10);
							strcat(Config_AccCaliberation,temp);
							strcat(Config_AccCaliberation,(char *)",");
							itoa(g_DeviceConfigCalibData.i32VerticleAxisMinValue,temp,10);
							strcat(Config_AccCaliberation,temp);
							strcat(Config_AccCaliberation,(char *)",");
							itoa(g_DeviceConfigCalibData.i32VerticleAxisMaxValue,temp,10);
							strcat(Config_AccCaliberation,temp);
							strcat(Config_AccCaliberation,(char *)",");
							Config_Write(Config_AccCaliberation, DPS_CONFIG_ACCALIB_PAGE, DPS_MAX_CONFIG_SIZE);
							/*Generate Diagnostic packet*/
//							Queue_LiveEnequeue(&gsmLivePayload,(char *)getDiagDataString(ENUM_DIG_G_SENS_CALIB));

						}
						break;
					}
				}

				/*Engine ON flag Generation Logic
				* 0- Engine OFF
				* 1-  Engine ON
				*/
//				{
//					if(g_stDeviceAccLiveData.u8EngineStausFlag == 0)
//					{
//						if(gu32RPM1_IpFreq>= (uint32_t)100)
//						{
//							g_stDeviceAccLiveData.u8EngineStausFlag = 1;
//						}
//						else if(gu32RPM1_IpFreq == 0)
//						{
////							if(HAL_GPIO_ReadPin(RPM1_IP_GPIO_Port, RPM1_IP_Pin) == GPIO_PIN_RESET)
////							{
////								if(HAL_GPIO_ReadPin(RPM1_IP_GPIO_Port, RPM1_IP_Pin) == GPIO_PIN_RESET)
////								{
////									g_stDeviceAccLiveData.u8EngineStausFlag = 1;
////								}
////							}
//						}
//					}
//					else
//					{
////						if((HAL_GPIO_ReadPin(RPM1_IP_GPIO_Port, RPM1_IP_Pin) != GPIO_PIN_RESET)&&(gu32RPM1_IpFreq == 0))
////						{
////							g_stDeviceAccLiveData.u8EngineStausFlag = 0;
////						}
//					}
//				}
				{
					/*Vehicle on slope flag Generation Logic
					* 0- not on a slope (less than  +5°  And Greater than -5°)
					* 1- Normal gradient ( in between +5° and +10°  Or In between -5° and -10°).
					* 2- extreme gradient  (Greater than  +10°  Or less than -10°)
					*/
					if((g_stDeviceAccLiveData.f32GradientAlongAxis > ACC_XY_VEH_EXTREME_GRADIENT_LIMIT)||
							(g_stDeviceAccLiveData.f32GradientAlongAxis < (-ACC_XY_VEH_NORMAL_GRADIENT_LIMIT)))
					{
						g_stDeviceAccLiveData.enumDeviceOnSlopeFlag = ENUM_V_EXTREME_GRADIENT;
					}
					else if(((g_stDeviceAccLiveData.f32GradientAlongAxis >= ACC_XY_VEH_NORMAL_GRADIENT_LIMIT)
					&&(g_stDeviceAccLiveData.f32GradientAlongAxis <= (ACC_XY_VEH_EXTREME_GRADIENT_LIMIT-ACC_XY_VEH_GRADIEN_HYST_VALUE)))
					||((g_stDeviceAccLiveData.f32GradientAlongAxis <= (-ACC_XY_VEH_NORMAL_GRADIENT_LIMIT))
							&&(g_stDeviceAccLiveData.f32GradientAlongAxis >= -(ACC_XY_VEH_EXTREME_GRADIENT_LIMIT-ACC_XY_VEH_GRADIEN_HYST_VALUE))))
					{
						g_stDeviceAccLiveData.enumDeviceOnSlopeFlag = ENUM_V_NORMAL_GRADIENT;
					}

					if(g_stDeviceAccLiveData.enumDeviceOnSlopeFlag != ENUM_V_EXTREME_GRADIENT)
					{
						if((g_stDeviceAccLiveData.f32GradientAroundAxis > ACC_XY_VEH_EXTREME_GRADIENT_LIMIT)||
								(g_stDeviceAccLiveData.f32GradientAroundAxis < (-ACC_XY_VEH_NORMAL_GRADIENT_LIMIT)))
						{
							g_stDeviceAccLiveData.enumDeviceOnSlopeFlag = ENUM_V_EXTREME_GRADIENT;
						}
						else if(((g_stDeviceAccLiveData.f32GradientAroundAxis >= ACC_XY_VEH_NORMAL_GRADIENT_LIMIT)
						&&(g_stDeviceAccLiveData.f32GradientAroundAxis <= (ACC_XY_VEH_EXTREME_GRADIENT_LIMIT-ACC_XY_VEH_GRADIEN_HYST_VALUE)))
						||((g_stDeviceAccLiveData.f32GradientAroundAxis <= (-ACC_XY_VEH_NORMAL_GRADIENT_LIMIT))
								&&(g_stDeviceAccLiveData.f32GradientAroundAxis >= -(ACC_XY_VEH_EXTREME_GRADIENT_LIMIT+ACC_XY_VEH_GRADIEN_HYST_VALUE))))
						{
							g_stDeviceAccLiveData.enumDeviceOnSlopeFlag = ENUM_V_NORMAL_GRADIENT;
						}
					}

					/*Not On Slope*/
					if(g_stDeviceAccLiveData.enumDeviceOnSlopeFlag != ENUM_V_NOT_ON_SLOPE)
					{
						/*Check is Vehicle is on Normal slope*/
						if((g_stDeviceAccLiveData.f32GradientAlongAxis < (ACC_XY_VEH_NORMAL_GRADIENT_LIMIT-ACC_XY_VEH_GRADIEN_HYST_VALUE))
							&&(g_stDeviceAccLiveData.f32GradientAlongAxis > -(ACC_XY_VEH_NORMAL_GRADIENT_LIMIT+ACC_XY_VEH_GRADIEN_HYST_VALUE))
							&&(g_stDeviceAccLiveData.f32GradientAroundAxis < (ACC_XY_VEH_NORMAL_GRADIENT_LIMIT-ACC_XY_VEH_GRADIEN_HYST_VALUE))
							&&(g_stDeviceAccLiveData.f32GradientAroundAxis > -(ACC_XY_VEH_NORMAL_GRADIENT_LIMIT+ACC_XY_VEH_GRADIEN_HYST_VALUE)))
						{
							g_stDeviceAccLiveData.enumDeviceOnSlopeFlag = ENUM_V_NOT_ON_SLOPE;
//							strHM10blemodule.u32HM10DataIgnoreTmr = TEN_MIN;
						}
					}
					static enumVeicleOnSlope s_enumDeviceOnSlopeOldFlag=ENUM_V_NOT_ON_SLOPE;
					if(s_enumDeviceOnSlopeOldFlag != g_stDeviceAccLiveData.enumDeviceOnSlopeFlag)
					{
						s_enumDeviceOnSlopeOldFlag = g_stDeviceAccLiveData.enumDeviceOnSlopeFlag;
//						Queue_LiveEnequeue(&gsmLivePayload,(char *)getMachineDataString());
						if(s_enumDeviceOnSlopeOldFlag != ENUM_V_NOT_ON_SLOPE)
						{
//							Queue_LiveEnequeue(&gsmLivePayload,(char *)getDiagDataString(ENUM_DIG_VHEICLE_ON_SLOPE));
						}
					}
				}



				{	/*Sudden Acceleration Flag Generation
					 *  Axis of Moment values are used for flag generation
					 * Sudden acceleration flag Generation Logic
					 * 0-Normal acceleration. less than 0.5 g
					 * 1- high acceleration   greater than 0.5g
					 * 2-extreme acceleration greater than 1.5g
					 */

					if(g_stDeviceAccLiveData.i32LongAxisMaxValue >= ACC_SUDDEN_ACC_EXTREME_LIMIT_M_PER_SEC_SQ)
					{
						g_stDeviceAccLiveData.enumSuddenAccFlag = ENUM_ACC_EXTREME_ACCLRATN;
					}
					else if((g_stDeviceAccLiveData.i32LongAxisMaxValue >= ACC_SUDDEN_ACC_HIGH_LIMIT_M_PER_SEC_SQ)&&
							(g_stDeviceAccLiveData.i32LongAxisMaxValue < (ACC_SUDDEN_ACC_EXTREME_LIMIT_M_PER_SEC_SQ-ACC_SUDDEN_ACC_HYST_VALUE)))
					{
						g_stDeviceAccLiveData.enumSuddenAccFlag = ENUM_ACC_HIGH_ACCLRATN;
					}
					else if(g_stDeviceAccLiveData.i32LongAxisMaxValue < (ACC_SUDDEN_ACC_HIGH_LIMIT_M_PER_SEC_SQ-ACC_SUDDEN_ACC_HYST_VALUE))
					{
						g_stDeviceAccLiveData.enumSuddenAccFlag = ENUM_ACC_NORMAL_ACCLRATN;
					}

					static enumSuddenAcclratn s_enumOldSuddenAccFlag = ENUM_ACC_NORMAL_ACCLRATN;
					if(s_enumOldSuddenAccFlag != g_stDeviceAccLiveData.enumSuddenAccFlag)
					{
						s_enumOldSuddenAccFlag =  g_stDeviceAccLiveData.enumSuddenAccFlag;
//						Queue_LiveEnequeue(&gsmLivePayload,(char *)getMachineDataString());
						if(s_enumOldSuddenAccFlag != ENUM_ACC_NORMAL_ACCLRATN)
						{
//							Queue_LiveEnequeue(&gsmLivePayload,(char *)getDiagDataString(ENUM_DIG_SUDDEN_ACCLRATN));
						}

					}
				}

				{
					/* Sudden Braking (de-acceleration)
					 * Sudden Braking flag Generation Logic
					 * Axis of Moment values are used for flag generation
					 *
					 * 0-Normal Braking. greater  than -0.5 g
					 * 1- high Braking  less than -0.5g
					 * 2-extreme Braking less than -1.5g
					 *
					 */
					if(g_stDeviceAccLiveData.i32LongAxisMinValue <= ACC_SUDDEN_BRAKING_EXTREME_LIMIT_M_PER_SEC_SQ)
					{
						g_stDeviceAccLiveData.enumSuddenBrakingFlag = ENUM_BRAKING_EXTREME_BRAKING;
					}
					else if((g_stDeviceAccLiveData.i32LongAxisMinValue <= ACC_SUDDEN_BRAKING_HIGH_LIMIT_M_PER_SEC_SQ)&&
							(g_stDeviceAccLiveData.i32LongAxisMinValue > (ACC_SUDDEN_BRAKING_EXTREME_LIMIT_M_PER_SEC_SQ-ACC_SUDDEN_BRAKING_HYST_VALUE)))
					{
						g_stDeviceAccLiveData.enumSuddenBrakingFlag = ENUM_BRAKING_HIGH_BRAKING;
					}
					else if(g_stDeviceAccLiveData.i32LongAxisMinValue > (ACC_SUDDEN_BRAKING_HIGH_LIMIT_M_PER_SEC_SQ-ACC_SUDDEN_BRAKING_HYST_VALUE))
					{
						g_stDeviceAccLiveData.enumSuddenBrakingFlag = ENUM_BRAKING_NORMAL_BRAKING;
					}

					static enumSuddenBraking s_enumOldSuddenBraking=ENUM_BRAKING_NORMAL_BRAKING;
					if(s_enumOldSuddenBraking != g_stDeviceAccLiveData.enumSuddenBrakingFlag)
					{
						s_enumOldSuddenBraking =  g_stDeviceAccLiveData.enumSuddenBrakingFlag;
//						Queue_LiveEnequeue(&gsmLivePayload,(char *)getMachineDataString());
						if(s_enumOldSuddenBraking != ENUM_BRAKING_NORMAL_BRAKING)
						{
//							Queue_LiveEnequeue(&gsmLivePayload,(char *)getDiagDataString(ENUM_DIG_SUDDEN_BRAKING));
						}
					}
				}

				{
					/*
					 * Impact
					 * Impact flag Generation Logic
					 * All Axis values are used for flag generation
					 * 0- No Impact -- base value is  between -2g to +2g
					 * 1- Impact 	-- Base value is less than -2g or greater than +2g
					 */

					if((g_stDeviceAccLiveData.i32LongAxisMaxValue  >= ACC_IMPACT_LIMIT_M_PER_SEC_SQ)||
						(g_stDeviceAccLiveData.i32LongAxisMinValue <= (-ACC_IMPACT_LIMIT_M_PER_SEC_SQ))||
						(g_stDeviceAccLiveData.i32LatAxisMaxValue  >= ACC_IMPACT_LIMIT_M_PER_SEC_SQ)||
						(g_stDeviceAccLiveData.i32LatAxisMinValue <= (-ACC_IMPACT_LIMIT_M_PER_SEC_SQ))||
						(g_stDeviceAccLiveData.i32VerticleAxisMaxValue  >= ACC_IMPACT_LIMIT_M_PER_SEC_SQ)||
						(g_stDeviceAccLiveData.i32VerticleAxisMinValue <= (-ACC_IMPACT_LIMIT_M_PER_SEC_SQ)))
					{
						if(g_stDeviceAccLiveData.u8DeviceImpactFlag == 0)
						{
							g_stDeviceAccLiveData.u8DeviceImpactFlag  = 1;
//							Queue_LiveEnequeue(&gsmLivePayload,(char *)getMachineDataString());
//							Queue_LiveEnequeue(&gsmLivePayload,(char *)getDiagDataString(ENUM_DIG_G_SENS_IMPACT));
						}
					}
					else
					{
						g_stDeviceAccLiveData.u8DeviceImpactFlag  = 0;
//						Queue_LiveEnequeue(&gsmLivePayload,(char *)getMachineDataString());
					}
				}

				/*Device Tampering Alert Generate*/
				if(g_stDeviceAccLiveData.u8DeviceTamperingDetectedFlag == 0)
				{
					if(((g_stDeviceAccLiveData.f32GradientAroundAxis >(ACC_DEVICE_TAMPR_ANGLE_MAX_LIMIT)) ||(g_stDeviceAccLiveData.f32GradientAroundAxis < -(ACC_DEVICE_TAMPR_ANGLE_MAX_LIMIT)))
							||((g_stDeviceAccLiveData.f32GradientAlongAxis >(ACC_DEVICE_TAMPR_ANGLE_MAX_LIMIT)) ||(g_stDeviceAccLiveData.f32GradientAlongAxis < -(ACC_DEVICE_TAMPR_ANGLE_MAX_LIMIT))))
					{
						g_stDeviceAccLiveData.u8DeviceTamperingDetectedFlag = 1;
						//Create Immdiate Payload for Device Tampering
//						Queue_LiveEnequeue(&gsmLivePayload,(char *)getMachineDataString());
//						Queue_LiveEnequeue(&gsmLivePayload,(char *)getDiagDataString(ENUM_DIG_DEVICE_TAMPRING));
					}
				}
				else
				{
					if(((g_stDeviceAccLiveData.f32GradientAlongAxis < (ACC_XY_VEH_NORMAL_GRADIENT_LIMIT-ACC_XY_VEH_GRADIEN_HYST_VALUE))
							&&(g_stDeviceAccLiveData.f32GradientAlongAxis > -(ACC_XY_VEH_NORMAL_GRADIENT_LIMIT+ACC_XY_VEH_GRADIEN_HYST_VALUE)))
						&&((g_stDeviceAccLiveData.f32GradientAroundAxis < (ACC_XY_VEH_NORMAL_GRADIENT_LIMIT-ACC_XY_VEH_GRADIEN_HYST_VALUE))
								&&(g_stDeviceAccLiveData.f32GradientAroundAxis > -(ACC_XY_VEH_NORMAL_GRADIENT_LIMIT+ACC_XY_VEH_GRADIEN_HYST_VALUE))))
					{
						g_stDeviceAccLiveData.u8DeviceTamperingDetectedFlag = 0;
					}
				}


/************************************* Vehicle In  Motion Flag *****************************************************************/
				/*Check is Vehicle in Motion*/
//				if((g_stDeviceAccLiveData.i32LongAxisAvgValue > ACC_VEHICLE_IN_MOTION_LIMIT_M_PER_SEC_SQ)||
//					(g_stDeviceAccLiveData.i32LatAxisAvgValue > ACC_VEHICLE_IN_MOTION_LIMIT_M_PER_SEC_SQ)||
//					(g_stDeviceAccLiveData.i32VerticleAxisAvgValue > ACC_VEHICLE_IN_MOTION_LIMIT_M_PER_SEC_SQ)||
//					(g_stDeviceAccLiveData.i32LongAxisAvgValue < (-ACC_VEHICLE_IN_MOTION_LIMIT_M_PER_SEC_SQ))||
//					(g_stDeviceAccLiveData.i32LatAxisAvgValue <  (-ACC_VEHICLE_IN_MOTION_LIMIT_M_PER_SEC_SQ))||
//					(g_stDeviceAccLiveData.i32VerticleAxisAvgValue <  (-ACC_VEHICLE_IN_MOTION_LIMIT_M_PER_SEC_SQ)))
				if((g_stDeviceAccLiveData.i32LongAxisAvgValue > g_DeviceConfigCalibData.i32LongAxisMaxValue)||
				(g_stDeviceAccLiveData.i32LatAxisAvgValue > g_DeviceConfigCalibData.i32LongAxisMaxValue)||
				(g_stDeviceAccLiveData.i32VerticleAxisAvgValue > g_DeviceConfigCalibData.i32VerticleAxisMaxValue)||
				(g_stDeviceAccLiveData.i32LongAxisAvgValue < (-g_DeviceConfigCalibData.i32LongAxisMinValue))||
				(g_stDeviceAccLiveData.i32LatAxisAvgValue <  (-g_DeviceConfigCalibData.i32LatAxisMinValue))||
				(g_stDeviceAccLiveData.i32VerticleAxisAvgValue <  (-g_DeviceConfigCalibData.i32VerticleAxisMinValue)))
				{
					if(g_stDeviceAccLiveData.u8EngineStausFlag  != 0)
					{
						if(g_stDeviceAccLiveData.u8VheicleInMotionFlag == 0)
						{
							g_stDeviceAccLiveData.u8VheicleInMotionFlag  =1;
//							Queue_LiveEnequeue(&gsmLivePayload,(char *)getMachineDataString());
//							Queue_LiveEnequeue(&gsmLivePayload,(char *)getDiagDataString(ENUM_DIG_VHEICLE_IN_MOTION));
						}
						gu32SensorVheInMotionTmr = THIRTY_SEC;
					}
				}
				else
				{
					/*Check Is Vehicle Stopped*/
					if((gu32SensorVheInMotionTmr == 0)&&(g_stDeviceAccLiveData.u8VheicleInMotionFlag != 0))
					{
						g_stDeviceAccLiveData.u8VheicleInMotionFlag  = 0;
//						Queue_LiveEnequeue(&gsmLivePayload,(char *)getMachineDataString());
					}

				}
/********************************************************************************************************************************/

				if((g_DeviceConfigCalibData.i32LatAxisMinValue <= g_stDeviceAccLiveData.i32LatAxisAvgValue)&&
					(g_stDeviceAccLiveData.i32LatAxisAvgValue <= g_DeviceConfigCalibData.i32LatAxisMaxValue))
				{
					g_stDeviceAccLiveData.i32LatAxisAvgValue = 0;
				}

				if((g_DeviceConfigCalibData.i32LongAxisMinValue <= g_stDeviceAccLiveData.i32LongAxisAvgValue)&&
					(g_stDeviceAccLiveData.i32LongAxisAvgValue <= g_DeviceConfigCalibData.i32LongAxisMaxValue))
				{
					g_stDeviceAccLiveData.i32LongAxisAvgValue = 0;
				}

				if((g_DeviceConfigCalibData.i32VerticleAxisMinValue <= g_stDeviceAccLiveData.i32VerticleAxisAvgValue)&&
					(g_stDeviceAccLiveData.i32VerticleAxisAvgValue <= g_DeviceConfigCalibData.i32VerticleAxisMaxValue))
				{
					g_stDeviceAccLiveData.i32VerticleAxisAvgValue = 0;
				}

				/*If Tilt angle less than 5 degree then consider respective axis value as Zero*/
//				if((g_stDeviceAccLiveData.f32GradientAlongAxis < 5.0)&&(g_stDeviceAccLiveData.f32GradientAlongAxis > (-5.0)))
//				{
//					g_stDeviceAccLiveData.i32LongAxisAvgValue = 0;
//				}
//
//				if((g_stDeviceAccLiveData.f32GradientAroundAxis < 5.0)&&(g_stDeviceAccLiveData.f32GradientAroundAxis > (-5.0)))
//				{
//					g_stDeviceAccLiveData.i32LatAxisAvgValue = 0;
//				}
//
//				if((g_stDeviceAccLiveData.i32LatAxisAvgValue==0)&&(g_stDeviceAccLiveData.i32LongAxisAvgValue==0))
//				{
//					g_stDeviceAccLiveData.i32VerticleAxisAvgValue = 0;//9806;
//				}

	#if(ACC_DATA_SERIAL_DEBUG_EN != 0)
				GenrateAccPayload(&g_stDeviceAccLiveData);
	#endif
				g_stDeviceAccLiveData.i32LongAxisMinValue = g_i32LongAxisValue;	g_stDeviceAccLiveData.i32LatAxisMinValue = g_i32LatAxisValue;	g_stDeviceAccLiveData.i32VerticleAxisMinValue = g_i32VerticleAxisValue;
				g_stDeviceAccLiveData.i32LongAxisMaxValue = g_i32LongAxisValue; g_stDeviceAccLiveData.i32LatAxisMaxValue = g_i32LatAxisValue; g_stDeviceAccLiveData.i32VerticleAxisMaxValue =g_i32VerticleAxisValue;
			}
			gu32SensorFsmMotionTmr = ONEHUNDRED_MS;
			s_egState = E_G_SENSOR_STATE_IDLE;
		}break;
		default:
		{
			s_egState = E_G_SENSOR_STATE_IDLE;
		}break;
	}
}

/**********************************************************************************
 Function Name: 	sensor_convert_data
 Purpose:			Convert Sensor raw data to actual data in m/s square
 Input:				1)Raw value Higher byte.
 	 	 	 	 	1)Raw value Lower byte.
 Return value: 		Signed 16 bit Calculate value in m/s square with 1000 multiplier.

 Change History:
 Author: Anil More         Date:   06/01/2023           Remarks

************************************************************************************/
static int32_t sensor_convert_data(uint8_t high_byte, uint8_t low_byte)
{

	uint16_t u16Result;
	int64_t result;
	double_t f64tempVar = 0;


	u16Result = (uint16_t)((uint16_t)high_byte << ((uint16_t)MXC3420AL_PRECISION - (uint16_t)8)) |
		 ((uint16_t)low_byte >> ((uint16_t)16 - (uint16_t)MXC3420AL_PRECISION));

	/*Signed Conversion*/
	if (u16Result < MXC3420AL_BOUNDARY)
	{
		/*Positive number*/
		result = u16Result ;
	}
	else
	{
		/*negative number*/
		result = ~((~u16Result & (0x7fff >> (16 - MXC3420AL_PRECISION))) + 1) + 1;
	}

	/*for Eg
	 * Let’s use FS ±2 g as an example sensitivity level.  As the range is -2 to +2, this would be a total of 4g.  Or 4,000 Milli-Gs.
	 * The output is 16 bits. 16 bits equals 65,535.   This means we can get 65,535 different readings for the range  between -2 and +2. (or -2,000 MilliGs and +2,000 MilliGs)
	 *  4,000 MilliGs / 65,535 = 0.061
	 *
	 *    */

	f64tempVar = (double_t)result*ACC_FSR;
	f64tempVar *= 9.80665;
	result = (int32_t)f64tempVar;
	return (int32_t)result;
}


// Using x y and z from accelerometer, calculate x and y angles

/**********************************************************************************
 Function Name: Acc_CalculateTiltAngles
 Purpose:		Caculate the tilt angle of X and Y and  calculate g force
 Input:			stAccData_t Pointer.

 Return value: None.

 Change History:
 Author           Date                Remarks

************************************************************************************/
static void Acc_CalculateTiltAngles(stAccData_t *pst_AccData)
{
	float x2, y2, z2;
	float x_val, y_val, z_val, result;
	float accel_angle_x,accel_angle_y;

	pst_AccData->f32Gforce = 0;
	pst_AccData->f32GradientAlongAxis = 0;
	pst_AccData->f32GradientAroundAxis = 0;

	x_val =(float) pst_AccData->i32LongAxisAvgValue/(float)1000;
//
//	if(g_DeviceConfigCalibData.e_DeviceMountingPos == E_G_SENSOR_VERTICAL)
//	{
//		/*Orientation changed*/
//		y_val =(float) (pst_AccData->i32LatAxisAvgValue)/(float)1000;
//		z_val =(float) (pst_AccData->i32VerticleAxisAvgValue+9806)/(float)1000;
//
//		x2 = (float)(x_val*x_val);
//		y2 = (float)(y_val*y_val);
//		z2 = (float)(z_val*z_val);
//	}
//	else
	{
		y_val =(float) (pst_AccData->i32LatAxisAvgValue)/(float)1000;
		z_val =(float) (pst_AccData->i32VerticleAxisAvgValue+ 9806)/(float)1000;

		x2 = (float)(x_val*x_val);
		y2 = (float)(y_val*y_val);
		z2 = (float)(z_val*z_val);
	}


	pst_AccData->f32Gforce = sqrt(x2+y2+z2);

	//X Axis
	result=sqrt(y2+z2);
	result=x_val/result;
	accel_angle_x = atan(result);
	accel_angle_x = accel_angle_x*(float)180/(3.1415926535); //(3.142857143);
	accel_angle_x *= (-1.0);


	pst_AccData->f32GradientAlongAxis= accel_angle_x;

	//Y Axis
	result=sqrt(x2+z2);
	result=y_val/result;
	accel_angle_y = atan(result);
	accel_angle_y = accel_angle_y*(float)180/(3.1415926535);  //(3.142857143);
	accel_angle_y *= (-1.0);

	pst_AccData->f32GradientAroundAxis= accel_angle_y;


}
/****************************************************************************
 Function HAL_I2C_MemRxCpltCallback
 Purpose: Memory Rx Transfer completed callback.
 Input:	  hi2c Pointer to a I2C_HandleTypeDef structure that contains
          the configuration information for the specified I2C.
 Return value: None.

 Note(s)(if-any) :

 Change History:
 Author           	Date                Remarks
 System Generated   11-04-18
******************************************************************************/

//void HAL_I2C_MemRxCpltCallback (I2C_HandleTypeDef *hi2c)
//{
//	g_u8I2CRxCmpltFlag = 1;
//}



/**
  * @brief  EXTI line detection callback.
  * @param  GPIO_Pin Specifies the port pin connected to corresponding EXTI line.
  * @retval None
  */
/*void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{

   Prevent unused argument(s) compilation warning

   NOTE: This function should not be modified, when the callback is needed,
           the HAL_GPIO_EXTI_Callback could be implemented in the user file



}*/



//******************************* End of File *******************************************************************
