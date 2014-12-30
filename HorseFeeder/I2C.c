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
    static uint8 data_index;

    //get status of I2C bus
    status = (TWSR & I2C_STATUS_MASK);

    switch (status)
    {
        case I2C_START:
        TWDR = get_write_addr(current_node.device_address);
        TWCR = (1 << TWINT);
        data_index = 0;
        break;

        case I2C_RESTART:
        if (current_node.mode == READ) {
            TWDR = get_read_addr(current_node.device_address);
            }else{
            TWDR = get_write_addr(current_node.device_address);
        }
        TWCR = (1 << TWINT);
        break;

        case I2C_MT_SLA_ACK: //send the sub address
        TWDR = current_node.sub_address;
        TWCR = (1 << TWINT);
        break;

        case I2C_MT_DATA_ACK: //send write data
        if (data_index != current_node.data_size)
        {
            //send more data
            TWDR = current_node.data_buffer[data_index];
            TWCR = (1 << TWINT);

        }
        else { //we are done writing, time for next transaction
            //check for more nodes
            if (dequeue(&(i2c_1_handle.Tx_queue), (uint8_t*) & current_node, sizeof (current_node))) //load next node from the queue
            {
                TWCR = (1 << TWINT) | (1 << TWSTO); //send stop bit
                i2c_1_handle.is_idle = TRUE; //flag that the bus is idle (nothing in the send queue)
            }
            else { //there is a new transaction to start
                TWCR = (1 << TWINT) | (1 <<TWSTA); //send restart signal
            }
        }
        ++data_index;
        break;


        default:
        //some sort of error handling?

    }
    //send start
    TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN); //initiate an I2C transaction


    //send device address+w
    if (continue_transaction == TRUE)
    {
        TWDR = (device_address << 1) | 0x00; //load the device address, plus the write bit
        
        TWCR = (1 << TWINT) | (1 << TWEN); // send the device address+w

        while (!(TWCR & (1<<TWINT))); //wait for transaction to complete

        if ( (TWSR & I2C_STATUS_MASK) != I2C_MT_SLA_ACK ) //confirm the transaction was completed
        {
            continue_transaction = FALSE;
        }
    }


    //send sub address
    if (continue_transaction == TRUE && sub_address_en == TRUE)
    {
        TWDR = sub_address; //load the sub address
        
        TWCR = (1 << TWINT) | (1 << TWEN); // send the sub address

        while (!(TWCR & (1<<TWINT))); //wait for transaction to complete
        
        if ( (TWSR & I2C_STATUS_MASK) != I2C_MT_DATA_ACK ) //confirm the transaction was completed
        {
            continue_transaction = FALSE;
        }
    }


    //send data
    if (continue_transaction == TRUE)
    {
        TWDR = data;
        
        TWCR = (1 << TWINT) | (1 << TWEN); // send the data

        while (!(TWCR & (1<<TWINT))); //wait for transaction to complete


        if ( (TWSR & I2C_STATUS_MASK) != I2C_MT_DATA_ACK ) //confirm the transaction was completed
        {
            continue_transaction = FALSE;
        }
    }

    TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWSTO); //send the stop signal
}