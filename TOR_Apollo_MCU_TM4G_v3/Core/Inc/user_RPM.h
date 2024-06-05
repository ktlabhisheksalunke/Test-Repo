/*
 * user_RPM.h
 *
 *  Created on: Apr 16, 2024
 *      Author: abhisheks
 */

#ifndef INC_USER_RPM_H_
#define INC_USER_RPM_H_

#define RPM_HRM_TRIGGER_PIN_STATE		GPIO_PIN_RESET
#define RPM_HRM_NUM									3

void RPM_IntHrMtrProcess(void);

#endif /* INC_USER_RPM_H_ */
