/*
 * user_UART.h
 *
 *  Created on: Nov 3, 2023
 *      Author: abhisheks
 */

#ifndef INC_USER_UART_H_
#define INC_USER_UART_H_

typedef enum{
	UART_IDLE = 0,
	UART_INPROGRESS = 1,
}enmUART_State;

struct UART_Buff{
	char * data;
	struct UART_Buff *next;
};

void DPS_ConfigRcvCallback(void);
void UART_Transmit(void);
void UART_Routine(void);

#endif /* INC_USER_UART_H_ */
