/*
 * user_queue.h
 *
 *  Created on: Jan 12, 2024
 *      Author: abhisheks
 */

#ifndef INC_USER_QUEUE_H_
#define INC_USER_QUEUE_H_

#include "user_ApplicationDefines.h"

typedef struct{
    int head;
    int tail;
    char* data[MAX_QUEUE_SIZE];
}strctQUEUE;

typedef struct{
    int head;
    int tail;
    char* data[MAX_BUFF_QUEUE_SIZE];
}strctBuffQUEUE;

/* Queue Function Prototypes */
void Queue_InitQ(strctQUEUE *queue);
void Queue_Enequeue(strctQUEUE * queue,char * data);
void Queue_Dequeue(strctQUEUE * queue);
uint32_t Queue_IsQueueEmpty(strctQUEUE * queue);
uint32_t Queue_IsQueueFull(strctQUEUE * queue);

void Queue_BuffQInit(strctBuffQUEUE* queue);
void Queue_BuffQEnqueue(strctBuffQUEUE* queue, char* data);
void Queue_BuffQDequeue(strctBuffQUEUE* queue);
uint8_t Queue_IsBuffQEmpty(strctBuffQUEUE* queue);
uint8_t Queue_IsBuffQFull(strctBuffQUEUE* queue);

#endif /* INC_USER_QUEUE_H_ */
