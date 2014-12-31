/*
 * RTCC.c
 *
 * Created: 12/29/2014 2:12:15 PM
 *  Author: james
 */ 

#include "RTCC.h"
#include "I2C.h"

volatile bool get_time_wait;
volatile bool enable_alarm_wait;
volatile uint8_t enable_alarm_reg;


void RTCC_set_time(DATE_TIME current_time)
{
    send_I2C(0, RTCC_ADDR, RTCC_SECONDS_ADDR, (uint8_t*) & current_time, sizeof(current_time), WRITE, NULL);
}

DATE_TIME RTCC_get_time(void)
{
    DATE_TIME current_time;
    
    get_time_wait = TRUE;
    
    send_I2C(0, RTCC_ADDR, RTCC_SECONDS_ADDR, (uint8_t*) & current_time, sizeof(current_time), READ, RTCC_get_time_callback);
    
    while (get_time_wait); //wait for callback to reset the flag
    
    return current_time;
}

void RTCC_program_alarm(ALARM_SETTING alarm)
{
    
}

void RTCC_enable_alarm(RTCC_ALARM which, bool on)
{
    
    
}


/***************************************************************************************************************************
 Internal Callback Functions
***************************************************************************************************************************/
void RTCC_get_time_callback(I2C_Node data)
{
    get_time_wait = FALSE;   
    
}
void RTCC_enable_alarm_callback(I2C_Node data)
{
    enable_alarm_reg = data.data_buffer[0];
    enable_alarm_wait = FALSE;    
}