/*
 * user_system.h
 *
 *  Created on: Nov 20, 2023
 *      Author: abhisheks
 */

#ifndef INC_USER_SYSTEM_H_
#define INC_USER_SYSTEM_H_

typedef enum{
	SYS_Boot = 0,
	SYS_Init,
	SYS_ModulePwrCycle,
	SYS_Routine,
	SYS_Config,
}enmSYS_State;

typedef enum{
	enmModule_PWRNOTSTARTED = 0,
	enmModule_PWRSTARTED,
	enmModule_PWRCOMPLETED
}enmModule_PWRSTATE;

void SYS_FSM(void);

#endif /* INC_USER_SYSTEM_H_ */
