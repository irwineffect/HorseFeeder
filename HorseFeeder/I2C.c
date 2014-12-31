/*
* I2C.c
*
* Created: 12/29/2014 2:13:59 PM
*  Author: james
*/


#include "I2C.h"
#include <avr/interrupt.h>

//callback functions
void (*I2C_1_callback) (void);

//channel data structures
I2C_Data i2c_1_handle;

//current node that is running
I2C_Node current_node;


I2C_Data setup_I2C(uint32_t sys_clk, uint8_t *rx_buffer_ptr, uint16_t rx_buffer_size,
uint8_t *tx_buffer_ptr, uint16_t tx_buffer_size, void* callback) {

    TWBR = ((sys_clk/I2C_SPEED) - 16)/2; //calculate the proper divider

    //setup the rx and tx buffers
    i2c_1_handle.Rx_queue = create_queue(rx_buffer_ptr, rx_buffer_size);
    i2c_1_handle.Tx_queue = create_queue(tx_buffer_ptr, tx_buffer_size);

    I2C_1_callback = callback; //link the callback function

    i2c_1_handle.is_idle = true; //set the I2C state machine to idling

    return &i2c1;

}

uint8_t send_I2C(uint8_t device_id, uint8_t device_address,
uint8_t sub_address, uint8_t* data_buffer, uint8_t data_size,
I2C_Mode read_write, void* callback) {
    I2C_Node new_node;
    uint8_t status;


    //create a new node with the passed in parameters
    new_node.device_id = device_id;
    new_node.device_address = device_address;
    new_node.sub_address = sub_address;
    new_node.data_buffer = data_buffer;
    new_node.data_size = data_size;
    new_node.mode = read_write;
    new_node.callback = callback;

    //load the new node
    status = enqueue(&(i2c_1_handle.Tx_queue), (uint8*) & new_node, sizeof (new_node));

    //if the bus is idling, force-start it
    if (i2c_1_handle.is_idle) {
        IFS1bits.I2C1MIF = 1;
    }

    return status;
}

//run this background process in the main while loop to
//process the results of I2C transactions
uint8_t bg_process_I2C(void); {
    I2C_Node current_node;

    //process channel 1
    while (!dequeue(&(i2c_1_handle.Tx_queue), (uint8*) & current_node, sizeof (current_node))) {
        if (current_node.callback != NULL) {
            current_node.callback(current_node);
        }
    }
}

ISR(TWI_vect)
{
    uint8_t status;
    static uint8_t data_index;
    static bool sub_addr_sent;

    //get status of I2C bus
    status = (TWSR & I2C_STATUS_MASK);

    switch (status)
    {
        case I2C_START:
        if (dequeue(&(i2c_1_handle.Tx_queue), (uint8_t*) & current_node, sizeof (current_node)))
        { //no data in queue, this should never happen
            TWCR &= ~(1 << TWSTA); //clear the start bit
            TWCR |= (1 << TWINT) | (1 << TWEN) | (1 << TWSTO); //send the stop signal, clean the bus
            i2c_1_handle.is_idle = TRUE;
        }
        else
        {
            TWDR = get_write_addr(current_node.device_address); //initiate a write always when starting
            TWCR |= (1 << TWINT) | (1 << TWEN);
            data_index = 0;
            i2c_1_handle.is_idle = FALSE;
            sub_addr_sent = FALSE;
        }
        break;

        case I2C_RESTART:
        if (current_node.mode == READ && sub_addr_sent == TRUE) {
            TWDR = get_read_addr(current_node.device_address);
            }else{
            TWDR = get_write_addr(current_node.device_address);
        }
        data_index = 0;
        TWCR |= (1 << TWINT) | (1 << TWEN);
        break;

        case I2C_MT_SLA_ACK: //send the sub address
        TWDR = current_node.sub_address;
        TWCR |= (1 << TWINT) | (1 << TWEN);
        sub_addr_sent = TRUE;
        break;

        case I2C_MT_DATA_ACK: //sub address has been sent
        if (current_node.mode == READ) //if we are reading
        {
            TWCR |= (1 << TWINT) | (1 << TWEN) | (1 << TWSTO); //send the restart signal
        }
        else //we are writing
        {
            if (data_index != current_node.data_size)
            {
                //send more data
                TWDR = current_node.data_buffer[data_index];
                TWCR |= (1 << TWINT) | (1 << TWEN);

            }
            else { //we are done writing, time for next transaction
                //check for more nodes
                if (dequeue(&(i2c_1_handle.Tx_queue), (uint8_t*) & current_node, sizeof (current_node))) //load next node from the queue
                {
                    TWCR |= (1 << TWINT) | (1 << TWEN) | (1 << TWSTO); //send stop bit
                    i2c_1_handle.is_idle = TRUE; //flag that the bus is idle (nothing in the send queue)
                }
                else { //there is a new transaction to start
                    TWCR |= (1 << TWINT) | (1 << TWEN) | (1 <<TWSTA); //send restart signal
                }
            }            
        }
        
        ++data_index;
        break;
        
        case I2C_MR_SLA_ACK:
         if (data_index != current_node.data_size)
         {   //this is not the last byte, send ACK
             TWCR |= (1 << TWINT) | (1 << TWEN) | (1 << TWEA);
         }
         else //last byte, send NACK
         {
             TWCR |= (1 << TWINT) | (1 << TWEN) | (1 << TWEA);
         }
        break;
        
        case I2C_MR_DATA_ACK:
        break;
        
        case I2C_MR_DATA_NACK:
        break;        
        
        case I2C_BUSY: //bus is busy, we shouldn't have entered ISR
        break; //don't do anything


        default: //unhandled error code
        //some sort of error handling?
        TWCR |= (1 << TWINT) | (1 << TWEN) | (1 << TWSTO); //send the stop signal
        i2c_1_handle.is_idle = FALSE;
        sub_addr_sent = FALSE;
        break;

    }
}