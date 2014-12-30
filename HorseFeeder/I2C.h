/*
* I2C.h
*
* Created: 12/29/2014 2:33:11 PM
*  Author: james
*/


#ifndef I2C_H_
#define I2C_H_

#include "system.h"
#include "queue.h"

#define I2C_SPEED (100000)

#define I2C_STATUS_MASK 0xF8
#define I2C_START       0x08
#define I2C_RESTART		0x10
#define I2C_MT_SLA_ACK  0x18
#define I2C_MR_SLA_ACK  0x40
#define I2C_MT_DATA_ACK 0x28
#define I2C_MR_DATA_ACK 0x58

typedef enum //reading or writing from the device
{
    READ,
    WRITE
} I2C_Mode;

#define get_read_addr(x) ((x << 1) | 0x01)
#define get_write_addr(x) ((x << 1))

typedef struct I2C_DATA {
    //Rx queue
    Queue Rx_queue;
    //Tx queue
    Queue Tx_queue;
    //idle information
    bool is_idle;
} I2C_Data;

typedef struct I2C_NODE {
    uint8_t device_id; //a unique identifier for the device
    uint8_t device_address; //I2C address for device
    uint8_t sub_address; //internal device address
    uint8_t* data_buffer; //data buffer to store the received data or write data
    uint8_t data_size; //how much data to send/read from device (must be <= buffer size)
    I2C_Mode mode; //reading or writing?
    void (*callback) (struct I2C_NODE); //callback function
} I2C_Node;

I2C_Data setup_I2C(uint32_t pb_clk, uint8_t *rx_buffer_ptr, uint16_t rx_buffer_size,
uint8_t *tx_buffer_ptr, uint16_t tx_buffer_size, void* callback);

uint8_t send_I2C(uint8_t device_id, uint8_t device_address,
uint8_t sub_address, uint8_t* data_buffer, uint8_t data_size,
I2C_Mode read_write, void* callback);

uint8_t bg_process_I2C(void);



#endif /* I2C_H_ */