/*
 * queue.h
 *
 * Created: 12/29/2014 2:15:01 PM
 *  Author: james
 */ 

#ifndef QUEUE_H_
#define QUEUE_H_

#include "system.h"


typedef struct QUEUE {
	uint8_t *buffer; //pointer to the queue memory
	uint16_t buffer_size; //size of the supplied buffer
	uint16_t QueueStart; //location of first data point (start of queue)
	uint16_t QueueEnd; //location of the last data point (end of queue)
	uint16_t numStored; //amount of data within the queue
} Queue;

/*Queue Functions*/
Queue create_queue(uint8_t* buffer, uint16_t buffer_size);
uint8_t enqueue(Queue* queue, uint8_t* data, uint16_t data_size);
uint8_t dequeue(Queue* queue, uint8_t* output_data, uint16_t data_size);


#endif /* QUEUE_H_ */