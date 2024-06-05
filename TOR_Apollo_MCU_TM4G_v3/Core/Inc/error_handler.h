/*
 * error_handler.h
 *
 *  Created on: Mar 25, 2022
 *      Author: admin
 */

#ifndef INC_ERROR_HANDLER_H_
#define INC_ERROR_HANDLER_H_

typedef enum{
	enmTORERRORS_TIMER7 = 0,
	enmTORERRORS_CAN1_INIT,
	enmTORERRORS_CAN1_CONFIGFILTER,
	enmTORERRORS_CAN1_START,
	enmTORERRORS_CAN1_NOTIFYFIFO0,
	enmTORERRORS_CAN1_TXERROR,
	enmTORERRORS_CAN1_RXERROR,
	enmTORERRORS_ENQUEUE,
	enmTORERRORS_MEM_I2C,
	enmTORERRORS_MEM_QUEUE
}enmSystemErrorType;

typedef enum
{
	enmERRORSTATE_NOERROR = 0,
	enmERRORSTATE_ACTIVE
}enmErrorStatus;

typedef struct
{
	uint32_t u32Timer7Error;
	uint32_t u32Can1InitError;
	uint32_t u32Can1ConfigFilterError;
	uint32_t u32Can1StartError;
	uint32_t u32Can1NotifyFIFO0Error;
	uint32_t u32Can1TxError;
	uint32_t u32Can1RxError;
	uint32_t u32EnqueueError;
	uint32_t u32MemI2CError;
	uint32_t u32MemQueueError;

	uint32_t u32UnknownError;
}strctSystemErrors;
/************************** End of System Error Defines ********************************/

void assertError(enmSystemErrorType errorType ,enmErrorStatus errorStatus);


#endif /* INC_ERROR_HANDLER_H_ */
