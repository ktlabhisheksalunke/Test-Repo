/*
 * user_ApplicationDefines.h
 *
 *  Created on: Nov 6, 2023
 *      Author: abhisheks
 */

#ifndef INC_USER_APPLICATIONDEFINES_H_
#define INC_USER_APPLICATIONDEFINES_H_

#define FALSE (0)
#define TRUE  (!FALSE)

#define SUCCESS TRUE
#define FAIL    FALSE

#define HIGH (1)
#define LOW  (0)

#define PAYLOAD_DATA_STRING_RADIX (10)
#define CAN_PAYLOADSTRING_RADIX	  (16)

#define ON (1)
#define OFF (!ON)
#define FLASH_ERASE (FALSE)

#define MODEL_NUMBER    		"DPS_Vitesse"
#define YEAR 	2023
#define MONTH	07
#define DATE	21

#define CAN_BUFFER_LENGTH     (200)
#define CAN_PAYLOADSTRING_RADIX	  (16)
#define CONFIG_CAN_ID		(MAX_CAN_IDS_SUPPORTED)
#define CONFIG_CAN_ID_Tx	(MAX_CAN_IDS_SUPPORTED)

#define DPS_PAYLOAD_MAX_SIZE	5120

#define DPS_DIAGNOSTIC_MAX_SIZE	512

#define DPS_UART_BUFF_SIZE 5120

#define MAX_QUEUE_SIZE         (10)
#define MAX_BUFF_QUEUE_SIZE         (6)

#define HOURMETER_NUM_DI 2

#define HOURMETER_COUNT 3

#define DPS_NUMOFDI 2

/*
 * Usage not recommended
 * Causing configuration clears
 * */
#define UART_DEBUGENABLE !TRUE

typedef enum
{
  CONFIG_PIN_RESET = 0U,
  CONFIG_PIN_SET
} CONFIG_PinState;

#endif /* INC_USER_APPLICATIONDEFINES_H_ */
