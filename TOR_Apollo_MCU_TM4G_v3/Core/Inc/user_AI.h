/*
 * user_AI.h
 *
 *  Created on: Oct 19, 2023
 *      Author: abhisheks
 */

#ifndef INC_USER_AI_H_
#define INC_USER_AI_H_

#define AI_ACTIVATION_FLAG 0x0803FFD0

#define	AI_BATTVLT		0
#define	AI_AI1				2
#define AI_AI2				1
#define	AI_INPUTVLT		3

typedef enum{
	AI_ADC_READY = 0,
	AI_ADC_DATA,
	AI_ADC_TIMEOUT
}enmAI_ADC_State;

void AI_Routine(void);


#endif /* INC_USER_AI_H_ */
