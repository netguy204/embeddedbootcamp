/** \file dive_time.h
*
* @brief Dive Time Tracker
*
* @par
* COPYRIGHT NOTICE: (c) 2016 Sam Rhody, No Liability Corp.
* All rights reserved.
*/

#ifndef _DIVE_TIME_H
#define _DIVE_TIME_H

typedef enum tmr_error {
    TMR_ERR_NONE            =   0u,
    TMR_ERR_TMR_STILL_ON    =   100u
} TMR_ERR;

void start_timer(uint8_t b_is_new_timer);
void stop_timer(void);
uint32_t get_dive_time_in_seconds(void);
TMR_ERR reset_timer(void);

#endif /* _DIVE_TIME_H*/
