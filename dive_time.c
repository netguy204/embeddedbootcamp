/** \file dive_time.c
*
* @brief Dive Time Tracker
*
* @par
* COPYRIGHT NOTICE: (c) 2016 Sam Rhody, No Liability Corp.
* All rights reserved.
*/

#include <stdint.h>
#include <os.h>
#include <assert.h>

#include "os_cfg_app.h"
#include "dive_time.h"


// Local defines
#define ONE_SECOND  1000
#define NO_ARGS     0

// Local inlines
#pragma inline
static OS_TICK period(uint32_t milliseconds) {
    uint32_t adjustment = ONE_SECOND / OS_CFG_TMR_TASK_RATE_HZ;
    return (milliseconds / adjustment);
}


// Local Variables
static OS_TMR g_dive_timer;
static volatile uint32_t g_elapsed_dive_time;
static uint8_t gb_is_timer_stopped = 1;

void 
increment_timer_callback(void * p_tmr, void * p_arg) {
    ++g_elapsed_dive_time;
}

/*!
* @brief Start the timer (possibly again). This is called when depth != 0.
*/
void 
start_timer(uint8_t b_is_new_timer)
{
    OS_ERR err;
    
    if (b_is_new_timer) 
    {
        g_elapsed_dive_time = 0;
        OSTmrCreate(&g_dive_timer,
                    "Elapsed Dive Time",
                    0,
                    period(ONE_SECOND),
                    OS_OPT_TMR_PERIODIC,
                    increment_timer_callback,
                    NO_ARGS,
                    &err);
        assert(OS_ERR_NONE == err);
    }
    
    // We will just start the timer (possibly again)
    OSTmrStart(&g_dive_timer, &err);
    assert(OS_ERR_NONE == err);
    gb_is_timer_stopped = 0;
}

/*!
* @brief Stop but don't delete the timer
*/
void
stop_timer(void)
{
    OS_ERR err;
    if(OS_TMR_STATE_RUNNING == OSTmrStateGet(&g_dive_timer, &err)) {     
      OSTmrStop(&g_dive_timer,
                OS_OPT_TMR_NONE,
                NO_ARGS,
                &err);
      assert(OS_ERR_NONE == err);
    } else {
      // make sure the status call didn't fail
      assert(OS_ERR_NONE == err);
    }
    gb_is_timer_stopped = 1;
}

/*!
* @brief Return the value of the timer.
* @return 32-bit unsigned integer representing time in seconds
*/
uint32_t
get_dive_time_in_seconds(void)
{
    return g_elapsed_dive_time;
}

/*!
* @brief Reset the timer.
* @warning The timer must have been stopped for this to work. Otherwise it will
*            return TMR_ERR_TMR_STILL_ON
* @return Error code
*/
TMR_ERR
reset_timer(void)
{
    TMR_ERR err = TMR_ERR_NONE;
    if (gb_is_timer_stopped) 
    {
        g_elapsed_dive_time = 0;
    }
    else
    {
        err = TMR_ERR_TMR_STILL_ON;
    }
    return err;
}

uint8_t
is_timer_off(void)
{
    return gb_is_timer_stopped;
}
