/*
 * error_handling.c
 *
 *  Created on: Mar 25, 2022
 *      Author: admin
 */

/*
  *****************************************************************************
  * @file    errorhandlers.c
  * @author  KloudQ Team
  * @version
  * @date
  * @brief   Application Error Handling Functions
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

  KloudQ Technologies Limited

*******************************************************************************
*/

#include "main.h"
#include "stm32l4xx_hal.h"
#include "user_applicationDefines.h"
#include "error_handler.h"

strctSystemErrors strsystemErrorLog;

/******************************************************************************
 Function Name: assertError
 Purpose: Will be used to notify error to the application code

 Input:	enmSystemErrorType errorType ,enmErrorStatus errorStatus.

 Return value: None.

 Notes:

 Change History:
 Author           	Date                Remarks
 KTL   				12-2-19				Initial Draft
******************************************************************************/
void assertError(enmSystemErrorType errorType ,enmErrorStatus errorStatus)
{
	switch(errorType)
	{
		case enmTORERRORS_TIMER7:
			strsystemErrorLog.u32Timer7Error = errorStatus;
		break;

		case enmTORERRORS_CAN1_CONFIGFILTER:
			strsystemErrorLog.u32Can1ConfigFilterError = errorStatus;
		break;

		case enmTORERRORS_CAN1_INIT:
			strsystemErrorLog.u32Can1InitError = errorStatus;
		break;

		case enmTORERRORS_CAN1_START:
			strsystemErrorLog.u32Can1StartError = errorStatus;
		break;

		case enmTORERRORS_CAN1_NOTIFYFIFO0:
			strsystemErrorLog.u32Can1NotifyFIFO0Error = errorStatus;
		break;

		break;

		case enmTORERRORS_CAN1_TXERROR:
			strsystemErrorLog.u32Can1TxError = errorStatus;
		break;

		case enmTORERRORS_CAN1_RXERROR:
			strsystemErrorLog.u32Can1RxError = errorStatus;
		break;

		case enmTORERRORS_ENQUEUE:
			strsystemErrorLog.u32EnqueueError = errorStatus;
		break;

		case enmTORERRORS_MEM_I2C:
			strsystemErrorLog.u32MemI2CError = errorStatus;
		break;

		case enmTORERRORS_MEM_QUEUE:
			strsystemErrorLog.u32MemQueueError = errorStatus;
		break;

		default :
			/* Unwanted / Undefined Error Occured
			 * System Will Halt Completely */
			//systemReset();
			break;
	}
}

//******************************* End of File *******************************************************************

