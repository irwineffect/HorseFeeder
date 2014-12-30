/*
* queue.c
*
* Created: 12/29/2014 2:14:45 PM
*  Author: james
*/
#include "queue.h"
#include <util/atomic.h>

Queue create_queue(uint8_t* buffer, uint16_t buffer_size) {
	Queue queue;

	queue.buffer = buffer;
	queue.buffer_size = buffer_size;

	queue.QueueEnd = 0;
	queue.QueueStart = 0;
	queue.numStored = 0;

	return queue;
}

uint8_t enqueue(Queue* queue, uint8_t* data, uint16_t data_size) {
	uint16_t i, j;
	uint8_t status;

	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) //queue access should be atomic
	{
		//check to see if there is room in the queue for the data
		if ((queue->buffer_size - queue->numStored) < data_size) {
			status = 1; //no room, return fail
		}
		else { //there is room, copy the data
			//copy the memory
			for (i = 0, j = queue->QueueEnd; i < data_size; ++i, ++j) {
				if (j == (queue->buffer_size)) //if we have reached end of buffer
				{
					j = 0; //wrap the buffer back around
				}

				queue->buffer[j] = data[i];
			}

			//update queue information
			queue->QueueEnd = j;
			queue->numStored += data_size;
			status = 0;
		}
	}
	return status;
}

uint8_t dequeue(Queue* queue, uint8_t* output_data, uint16_t data_size) {
	uint16_t i, j;
	uint8_t status;
	
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) //queue access should be atomic
	{
		//check if there is that much data in queue
		if (data_size > queue->numStored) {
			status = 1;
		}
		else {
			//copy the memory
			for (i = 0, j = queue->QueueStart; i < data_size; ++i, ++j) {
				if (j == (queue->buffer_size)) //if we have reached end of buffer
				{
					j = 0; //wrap the buffer back around
				}

				output_data[i] = queue->buffer[j];
			}

			queue->QueueStart = j;
			queue->numStored -= data_size;

			status = 0;
		}
	}
	
	return status;
}