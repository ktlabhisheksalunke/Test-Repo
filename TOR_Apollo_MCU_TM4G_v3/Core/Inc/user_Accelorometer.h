/*
 * Accelorometer.h
 *
 *  Created on: Nov 25, 2022
 *      Author: anilm
 */

#ifndef INC_USER_ACCELOROMETER_H_
#define INC_USER_ACCELOROMETER_H_


#include <math.h>
#include <stdint.h>


#define G_SESN_CALIB_DONE			1
#define G_SESN_CALIB_FAIL			0
#define G_SESN_CALIB_START			2
#define G_SESN_CALIB_INPROGRESS		3




/*Macros*/
#define ACC_SLAVE_ID_MXC3420AL	((uint8_t)0x4C<<1) //VPP level upon power-up Pin Connected to Ground
//#define ACC_SLAVE_ID_MXC3420AL	((uint8_t)0x6C<<1)//VPP level upon power-up Pin Connected to VDD

/*MXC3420AL Registers*/
#define MXC3420AL_MODE_REG_WR_ADD		0x07
#define MXC3420AL_FSR_REG_WR_ADD		0x20

/********** Axis Read Address values **************/

#define MXC3420AL_XOUT_EX_L_RD_ADD		0x0D
#define MXC3420AL_XOUT_EX_H_RD_ADD		0x0E
#define MXC3420AL_YOUT_EX_L_RD_ADD		0x0F
#define MXC3420AL_YOUT_EX_H_RD_ADD		0x10
#define MXC3420AL_ZOUT_EX_L_RD_ADD		0x11
#define MXC3420AL_ZOUT_EX_H_RD_ADD		0x12



/*********************/
#define ACC_FSR_SEL_2G			1
#define ACC_FSR_SEL_4G			2
#define ACC_FSR_SEL_8G			3
#define ACC_FSR_SEL_12G			4
#define ACC_FSR_SEL_16G			5

#define ACC_FSR_SEL     (ACC_FSR_SEL_4G)

/*for Eg
 * Let’s use FS ±2 g as an example sensitivity level.  As the range is -2 to +2, this would be a total of 4g.  Or 4,000 Milli-Gs.
 * The output is 16 bits. 16 bits equals 65,535.   This means we can get 65,535 different readings for the range  between -2 and +2. (or -2,000 MilliGs and +2,000 MilliGs)
 *  4,000 MilliGs / 65,535 = 0.061
 *
 *    */

/*FSR Range selection and Sensitivity*/
#if(ACC_FSR_SEL == ACC_FSR_SEL_2G)
	#define ACC_FSR   					(double_t)0.061035156
	#define ACC_MXC3420AL_CNTRL_REG_VAL			(uint8_t)0x09
#elif(ACC_FSR_SEL == ACC_FSR_SEL_4G)
	#define ACC_FSR	    				(double_t)0.122070313
	#define ACC_MXC3420AL_CNTRL_REG_VAL			(uint8_t)0x19
#elif(ACC_FSR_SEL == ACC_FSR_SEL_8G)
	#define ACC_FSR						(double_t)0.244140625
	#define ACC_MXC3420AL_CNTRL_REG_VAL			(uint8_t)0x29
#elif(ACC_FSR_SEL == ACC_FSR_SEL_12G)
	#define ACC_FSR						(double_t)0.366300366
	#define ACC_MXC3420AL_CNTRL_REG_VAL			(uint8_t)0x49
#elif(ACC_FSR_SEL == ACC_FSR_SEL_16G)
	#define ACC_FSR						(double_t)0.48828125
	#define ACC_MXC3420AL_CNTRL_REG_VAL			(uint8_t)0x39
#endif



/********************************************************/




#define MXC3420AL_PRECISION		(int32_t)16
#define MXC3420AL_BOUNDARY	    (int32_t)(0x1 << (MXC3420AL_PRECISION - 1))


#define ACC_DATA_READ_TIME_MS			(uint32_t)100
#define ACC_SAMPLE_LEN     			 	(uint32_t)10  //10*100 = 1000ms = 1Second
#define ACC_CALIB_SAMPLE_LEN     		(uint32_t)15  //15*1 Second = 15Second






/*Vehicle In motion Limit in m/s^2 */
#define ACC_VEHICLE_IN_MOTION_LIMIT_M_PER_SEC_SQ     (int32_t)4903  //==0.5g

/*Angle Limits are in Degree*/
#define ACC_XY_VEH_NORMAL_GRADIENT_LIMIT     	(float_t)15.0
#define ACC_XY_VEH_EXTREME_GRADIENT_LIMIT    	(float_t)30.0
#define ACC_XY_VEH_GRADIEN_HYST_VALUE      		(float_t)5.0

/*Sudden acceleration in m/s^2 */
#define ACC_SUDDEN_ACC_HIGH_LIMIT_M_PER_SEC_SQ    					(int32_t)4903  //==0.5g
#define ACC_SUDDEN_ACC_EXTREME_LIMIT_M_PER_SEC_SQ      				(int32_t)14709  //==1.5g
#define ACC_SUDDEN_ACC_HYST_VALUE	      							(int32_t)980  //==0.1g

/*Sudden Braking in m/s^2 */
#define ACC_SUDDEN_BRAKING_HIGH_LIMIT_M_PER_SEC_SQ    					(int32_t)-4903  //==0.5g
#define ACC_SUDDEN_BRAKING_EXTREME_LIMIT_M_PER_SEC_SQ      				(int32_t)-14709  //==1.5g
#define ACC_SUDDEN_BRAKING_HYST_VALUE	      							(int32_t)-980  //==0.1g

/*Sudden Braking in m/s^2 */
#define ACC_IMPACT_LIMIT_M_PER_SEC_SQ    					(int32_t)19613   //==2g

/**/
#define ACC_DEVICE_TAMPR_ANGLE_MAX_LIMIT     (float_t)30.0

#define ACC_DATA_SERIAL_DEBUG_EN	0
#define SERIAL_DEBUG_MAND_EN	 	0

/*ACCELOROMETER REG ADD*/

typedef enum
{
	ENUM_V_NOT_ON_SLOPE,
	ENUM_V_NORMAL_GRADIENT,
	ENUM_V_EXTREME_GRADIENT
}enumVeicleOnSlope;

typedef enum
{
	ENUM_ACC_NORMAL_ACCLRATN,
	ENUM_ACC_HIGH_ACCLRATN,
	ENUM_ACC_EXTREME_ACCLRATN
}enumSuddenAcclratn;

typedef enum
{
	ENUM_BRAKING_NORMAL_BRAKING,
	ENUM_BRAKING_HIGH_BRAKING,
	ENUM_BRAKING_EXTREME_BRAKING
}enumSuddenBraking;

typedef enum
{
	ENUM_DEVICE_NO_TRUN,
	ENUM_DEVICE_TURN_N,
	ENUM_DEVICE_TURN_P
}enumDeviceTurn;


typedef struct
{
	uint16_t u16SampleIndex;
	/*1g = 9.80665 m/s^2*/
	/*X axis Values in m/s^2. 1000 Multiplier */
	int32_t i32LongAxisRawVAlue[ACC_SAMPLE_LEN];
	int32_t i32LongAxisAvgValue;
	int32_t i32LongAxisMinValue;
	int32_t i32LongAxisMaxValue;
	/*Y axis Values in m/s^2. 1000 Multiplier */
	int32_t i32LatAxisRawVAlue[ACC_SAMPLE_LEN];
	int32_t i32LatAxisAvgValue;
	int32_t i32LatAxisMinValue;
	int32_t i32LatAxisMaxValue;
	/*Z axis Values in m/s^2. 1000 Multiplier */
	int32_t i32VerticleAxisRawVAlue[ACC_SAMPLE_LEN];
	int32_t i32VerticleAxisAvgValue;
	int32_t i32VerticleAxisMinValue;
	int32_t i32VerticleAxisMaxValue;


//	int32_t i32LongAxisCalibRawVAlue[ACC_CALIB_SAMPLE_LEN];
//	int32_t i32LongAxisCalibAvgValue;
//	int32_t i32LongAxisCalibMinValue;
//	int32_t i32LongAxisCalibMaxValue;
//	/*Y axis Values in m/s^2. 1000 Multiplier */
//	int32_t i32LatAxisCalibRawVAlue[ACC_CALIB_SAMPLE_LEN];
//	int32_t i32LatAxisCalibAvgValue;
//	int32_t i32LatAxisCalibMinValue;
//	int32_t i32LatAxisCalibMaxValue;
//
//	/*Z axis Values in m/s^2. 1000 Multiplier */
//	int32_t i32VerticleAxisCalibRawVAlue[ACC_CALIB_SAMPLE_LEN];
//	int32_t i32VerticleAxisCalibAvgValue;
//	int32_t i32VerticleAxisCalibMinValue;
//	int32_t i32VerticleAxiCalibsMaxValue;

	float_t f32Gforce;
	float_t f32GradientAlongAxis;
	float_t f32GradientAroundAxis;

	uint8_t u8VheicleInMotionFlag;
	uint8_t u8EngineStausFlag;
	enumVeicleOnSlope enumDeviceOnSlopeFlag;
	enumSuddenAcclratn enumSuddenAccFlag;
	enumSuddenBraking enumSuddenBrakingFlag;
	uint8_t u8DeviceImpactFlag;
	enumDeviceTurn enumDeviceTrunFlag; //0-Turn Normal, 1
	uint8_t u8DeviceTamperingDetectedFlag;

}stAccData_t;

typedef enum
{
	E_G_SENSOR_HORIZONTICAL =0,
	E_G_SENSOR_VERTICAL
}e_GSensMountPos;


typedef struct
{
	/*G Sensor Cailbration Data*/
	//e_GSensMountPos e_DeviceMountingPos;
	e_GSensMountPos e_DeviceMountingPos ;

	int32_t i32LatAxisOffset;
	int32_t i32LatAxisMinValue;
	int32_t i32LatAxisMaxValue;


	int32_t i32LongAxisOffset;
	int32_t i32LongAxisMinValue;
	int32_t i32LongAxisMaxValue;

	int32_t i32VerticleAxisOffset;
	int32_t i32VerticleAxisMinValue;
	int32_t i32VerticleAxisMaxValue;


	uint32_t u32CheckSum;
}st_DeviceConfigCalibData;

/*Typedef*/
typedef enum
{
	E_G_SENSOR_STATE_IDLE=0,
	E_G_SENSOR_STATE_CONFIGURE,
	E_G_SENSOR_STATE_READDATA,
	E_G_SENSOR_STATE_CALIBRATE,
	E_G_SENSOR_STATE_WAIT_READ_CMPLT,
	E_G_SENSOR_STATE_WAIT_READ_CONFIG_CMPLT,

}e_GSensorState;

void G_SensorFSM(void);

#endif /* INC_USER_ACCELOROMETER_H_ */
