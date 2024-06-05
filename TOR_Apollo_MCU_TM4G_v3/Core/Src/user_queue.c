/*
 * user_queue.c
 *
 *  Created on: Jan 12, 2024
 *      Author: abhisheks
 */


#include "main.h"
#include "user_ApplicationDefines.h"
#include <string.h>
#include <stdlib.h>
#include "stdint.h"
#include "user_queue.h"
#include "externs.h"
#include "payload.h"


/****************************************************************************
 Function: Queue_InitQ
 Purpose: Initialize Queue
 Input:	strctQUEUE *queue
 Return value: None

 Note(s)(if-any) :

 Change History:
 Author            	Date                Remarks
 KloudQ Team        22/05/19			initial code
******************************************************************************/
void Queue_InitQ(strctQUEUE *queue)
{
//    queue->head = 0;
//    queue->tail = 0;
	queue->head = queue->tail = -1;
}

/****************************************************************************
 Function: Queue_Enequeue
 Purpose: Add element to Queue
 Input:	strctQUEUE *queue
 Return value: None


 Note(s)(if-any) :


 Change History:
 Author            	Date                Remarks
 KloudQ Team        22/05/19			initial code
******************************************************************************/
void Queue_Enequeue(strctQUEUE * queue,char * data)
{

	if (data == NULL) {
		return;
	}
  if (Queue_IsQueueEmpty(queue))
      queue->tail = 0;

  queue->head = ((queue->head + 1) % MAX_BUFF_QUEUE_SIZE);
  queue->data[queue->head] = data;

//	/* if data is completely transfered, reset the queue */
//	if(queue->head <= queue->tail)
//	{
//		queue->head = 0;
//		queue->tail = 0;
//	}
//
//	/* If queue has data clear it */
//	if(queue->data[queue->head] != NULL)
//	{
//		free(queue->data[queue->head]);
//		queue->data[queue->head] = NULL;
//	}
//
//	if(data != NULL) /*malloc successfully done*/
//	{
//		/* Write data to queue head */
//		queue->data[queue->head]= data;
//		/* move to head to next location */
//		queue->head++;
//	}
//
//	/* If queue is full */
//	if(queue->head > (MAX_QUEUE_SIZE - 1))
//	{
//		/*Raise Queue Overflow flag */
////		queue->head = (MAX_QUEUE_SIZE - 1);
//	}
}

/****************************************************************************
 Function: Queue_Dequeue
 Purpose: Remove element from Queue
 Input:	strctQUEUE *queue
 Return value: None


 Note(s)(if-any) :


 Change History:
 Author            	Date                Remarks
 KloudQ Team        22/05/19			initial code
******************************************************************************/
void Queue_Dequeue(strctQUEUE * queue)
{
  char* temp = queue->data[queue->tail];
  queue->data[queue->tail] = NULL;

  if (queue->head == queue->tail)
  	Queue_InitQ(queue);
  else
  	queue->tail = (queue->tail + 1) % MAX_BUFF_QUEUE_SIZE;

  free(temp);

//    /* After data is uploaded free the pointer in queue->data */
//        if(((queue->tail == 0) && (queue->head == 0)) || (queue->tail == queue->head))
//        {
//
//            if((queue->tail >= queue->head) && (queue->tail != 0)  && (queue->head != 0))
//            {
//            	if(queue->data[queue->tail] != NULL)
//				{
//					free(queue->data[queue->tail]);
//					queue->data[queue->tail] = NULL;
//				}
//
//            	queue->head = 0;
//    			queue->tail = 0;
//            }
//        }
//        else
//        {
//            free(queue->data[queue->tail]);
//            queue->data[queue->tail] = NULL;
//            queue->tail++;
//            if(queue->tail > (MAX_QUEUE_SIZE-1))
//						{
//							queue->tail=0;
//						}
//        }
}

/****************************************************************************
 Function: displayQueue
 Purpose: Check is queue is empty
 Input:	strctQUEUE *queue
 Return value: uint32_t status,TRUE if queue is empty


 Note(s)(if-any) :

 Change History:
 Author            	Date                Remarks
 KloudQ Team        22/05/19			initial code
******************************************************************************/
uint32_t Queue_IsQueueEmpty(strctQUEUE * queue)
{
//	if((queue->head == queue->tail) && (queue->data[queue->tail] == NULL))
//		return 1;
//	else
//		return 0;
	//	return((queue->head) == (queue->tail));
	return (queue->head == -1) ||(queue->tail == -1);

}

uint32_t Queue_IsQueueFull(strctQUEUE * queue)
{
//	return(queue->head == (MAX_QUEUE_SIZE));
	return (queue->tail == ((queue->head + 1) % MAX_QUEUE_SIZE));
}

/********************************************************************************/

void Queue_BuffQInit(strctBuffQUEUE* queue) {
    queue->head = queue->tail = -1;
}

// Function to check if the queue is empty
uint8_t Queue_IsBuffQEmpty(strctBuffQUEUE* queue) {
	return (queue->head == -1) ||(queue->tail == -1);
}

// Function to check if the queue is full
uint8_t Queue_IsBuffQFull(strctBuffQUEUE* queue) {
    return (queue->tail == ((queue->head + 1) % MAX_BUFF_QUEUE_SIZE));
}

// Function to enqueue an element
void Queue_BuffQEnqueue(strctBuffQUEUE* queue, char* data) {
		if (data == NULL) {
			return;
		}
    if (Queue_IsBuffQEmpty(queue))
        queue->tail = 0;

    queue->head = ((queue->head + 1) % MAX_BUFF_QUEUE_SIZE);
    queue->data[queue->head] = data;
}

// Function to dequeue an element
void Queue_BuffQDequeue(strctBuffQUEUE* queue) {
//    if (Queue_IsBuffQEmpty(queue)) {
//        return;
//    }
    char* temp = queue->data[queue->tail];
    queue->data[queue->tail] = NULL;

    if (queue->head == queue->tail)
    	Queue_BuffQInit(queue);
    else
    	queue->tail = (queue->tail + 1) % MAX_BUFF_QUEUE_SIZE;

    free(temp);
}
