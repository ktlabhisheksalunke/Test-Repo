/*
 * user_modbus_config.h
 *
 *  Created on: Dec 4, 2023
 *      Author: pratikshaw
 */

#ifndef INC_USER_MODBUS_CONFIG_H_
#define INC_USER_MODBUS_CONFIG_H_

#include "externs.h"
#include "main.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "user_modbus_rs485.h"
#include "user_modbus_config.h"


/*
 This enumeration is a list of peripheral(s) supported by configuration stack
*/

typedef enum
{
	enmCONFIG_MB485DATAPOINTS = 0,
	enmCONFIG_MB485FUCNTIONCODE,
	enmCONFIG_MB485UARTBR,
	enmCONFIG_MB485UARTPARITY,
	enmCONFIG_MB485UARTSTOPBIT,
	enmCONFIG_MB485UARTDATABIT,
	enmCONFIG_MB485POLLTIME,
	enmCONFIG_MB485RESPTIME,
}enmModbus485ConfigErrors;

/*
 This enumeration is a list of DI Configuration(s)

 */


//void parseRemoteconfig(void);
//void checkConfigurationAvailable(void);
//void extractDiConfiguration(void);
void extractModbus485Configuration(void);
void Modbus485_Init();
//char * getSystemConfig(void);
#endif /* INC_USER_MODBUS_CONFIG_H_ */
