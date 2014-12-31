/*
* RTCC.h
*
* Created: 12/29/2014 2:33:37 PM
*  Author: james
*/


#ifndef RTCC_H_
#define RTCC_H_

#include "system.h"
#include "I2C.h"

/***************************************************************************************************************************
 RTCC Addresses
***************************************************************************************************************************/
#define RTCC_ADDR			0b01101111

#define RTCC_SECONDS_ADDR	0x00
#define RTCC_MINUTES_ADDR	0x01
#define RTCC_HOURS_ADDR		0x02
#define RTCC_DAY_ADDR		0x03
#define RTCC_DATE_ADDR		0x04
#define RTCC_MONTH_ADDR		0x05
#define RTCC_YEAR_ADDR		0x06
#define RTCC_CONTROL_ADDR	0x07

#define ALARM0_OFFSET		0x0A
#define ALARM1_OFFSET		0x11

#define RTCC_SRAM_START		0x20
#define RTCC_SRAM_END		0x5F

/***************************************************************************************************************************
 RTCC Masks
***************************************************************************************************************************/
#define RTCC_SECONDS_MASK		0b00001111
#define RTCC_SECONDS_10_MASK	0b01110000
#define RTCC_MINUTES_MASK		0b00001111
#define RTCC_MINUTES_10_MASK	0b01110000
#define RTCC_HOURS_MASK			0b00001111
#define RTCC_HOURS_10_MASK		0b01110000
#define RTCC_DAY_MASK			0b00000111
#define RTCC_DATE_MASK			0b00001111
#define RTCC_DATE_10_MASK		0b00110000
#define RTCC_MONTH_MASK			0b00001111
#define RTCC_MONTH_10_MASK		0b00010000
#define RTCC_YEAR_MASK			0b00001111
#define RTCC_YEAR_10_MASK		0b11110000

#define RTCC_VBATEN_MASK		0b00001000
#define RTCC_ALARM0_MASK		0b00010000
#define RTCC_ALARM1_MASK		0b00100000

/***************************************************************************************************************************
 Enumerations
***************************************************************************************************************************/
typedef enum 
{
    ALARM_0,
    ALARM_1,
    ALARM_BOTH
}RTCC_ALARM;

/***************************************************************************************************************************
 Data Structures
***************************************************************************************************************************/
typedef struct
{
    uint8_t seconds;
    uint8_t minutes;
    uint8_t hours;
    uint8_t day;
    uint8_t date;
    uint8_t month;
    uint8_t year;
    
}DATE_TIME;

typedef struct  
{
    DATE_TIME current_time;
    RTCC_ALARM which_alarm;
}ALARM_SETTING;



/***************************************************************************************************************************
 Public Functions
***************************************************************************************************************************/
void RTCC_set_time(DATE_TIME current_time);
DATE_TIME RTCC_get_time(void);
void RTCC_program_alarm(ALARM_SETTING alarm);
void RTCC_enable_alarm(RTCC_ALARM which, bool on);

/***************************************************************************************************************************
 Internal Callback Functions
***************************************************************************************************************************/
void RTCC_get_time_callback(I2C_Node data);
void RTCC_enable_alarm_callback(I2C_Node data);

#endif /* RTCC_H_ */