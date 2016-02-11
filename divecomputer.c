/** \file main.c
*
* @brief Embedded Software Boot Camp
*
* @par
* COPYRIGHT NOTICE: (C) 2014 Barr Group, LLC.
* All rights reserved.
*/

/*
*********************************************************************************************************
*                                             INCLUDE FILES
*********************************************************************************************************
*/

#include  "cpu_cfg.h"
#include  "bsp_cfg.h"
#include  "assert.h"
#include  "stdio.h"

#include  <cpu_core.h>
#include  <os.h>
#include  <bsp_glcd.h>

#include  "bsp.h"
#include  "bsp_int_vect_tbl.h"
#include  "bsp_led.h"
#include  "os_app_hooks.h"

#include "protectedled.h"
#include "pushbutton.h"
#include "common.h"
#include "adc.h"
#include "calculator.h"

/*
*********************************************************************************************************
*                                            LOCAL DEFINES
*********************************************************************************************************
*/
#define HEALTH_LED   4

/*
*********************************************************************************************************
*                                           LOCAL CONSTANTS
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                          LOCAL DATA TYPES
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                       LOCAL GLOBAL VARIABLES
*********************************************************************************************************
*/

// Relative Task Priorities (0 = highest; 15 = idle task)
#define  STARTUP_PRIO           1   // Highest priority, to launch others.
#define  DEBOUNCE_PRIO          7   // Every 50 ms, in a timed loop.
#define  SW1_PRIO               8   // Up to every 50 ms, when held down.
#define  ADC_PRIO               9   // Every 125 ms, in a timed loop.
#define  CALC_PRIO             10   // Priority for calculor_task
#define  SW2_PRIO              12   // Up to every 150 ms, if retriggered.
#define  LED6_PRIO             13   // Every 167 ms, in a timed loop.
#define  LED5_PRIO             14   // Every 500 ms, in a timed loop.

// Allocate Task Stacks
#define  TASK_STACK_SIZE      128

static CPU_STK  g_startup_stack[TASK_STACK_SIZE];
static CPU_STK  g_debounce_stack[TASK_STACK_SIZE];
static CPU_STK  g_calc_stack[TASK_STACK_SIZE];

// Allocate Task Control Blocks
static OS_TCB   g_startup_tcb;
static OS_TCB   g_debounce_tcb;
static OS_TCB   g_calc_tcb;

// Timers
static OS_TMR   g_health_timer;
/*
*********************************************************************************************************
*                                            LOCAL MACRO'S
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                      LOCAL FUNCTION PROTOTYPES
*********************************************************************************************************
*/

/*!
* @brief Scuba health task
*/
void
scuba_health_task (void * p_tmr, void * p_arg)
{
    (void) p_arg;
    (void) p_tmr;
    BSP_LED_Toggle(HEALTH_LED);
}

void
startup_task (void * p_arg)
{
    OS_ERR  err;


    (void)p_arg;    // NOTE: Silence compiler warning about unused param.

    // Perform hardware initializations that should be after multitasking.
    BSP_Init();
    CPU_Init();
    Mem_Init();

    // Set the font for the LCD
    BSP_GraphLCD_SetFont(GLYPH_FONT_8_BY_8);
    
    // Initialize the reentrant LED driver.
    protectedLED_Init();
    
    // Create flags shared by alarm and calculator tasks
    OSFlagCreate(&g_alarm_flags, "Alarm Flag", 0, &err);
    assert(OS_ERR_NONE == err);

    // Create the semaphores signaled by the button debouncer.
    OSSemCreate(&g_sw1_sem, "Switch 1", 0, &err);
    assert(OS_ERR_NONE == err);	

    OSSemCreate(&g_sw2_sem, "Switch 2", 0, &err);
    assert(OS_ERR_NONE == err);	

    // Create the button debouncer.
    OSTaskCreate((OS_TCB     *)&g_debounce_tcb,
                 (CPU_CHAR   *)"Button Debouncer",
                 (OS_TASK_PTR ) debounce_task,
                 (void       *) 0,
                 (OS_PRIO     ) DEBOUNCE_PRIO,
                 (CPU_STK    *)&g_debounce_stack[0],
                 (CPU_STK_SIZE) TASK_STACK_SIZE / 10u,
                 (CPU_STK_SIZE) TASK_STACK_SIZE,
                 (OS_MSG_QTY  ) 0u,
                 (OS_TICK     ) 0u,
                 (void       *) 0,
                 (OS_OPT      ) 0,
                 (OS_ERR     *)&err);
    assert(OS_ERR_NONE == err);

    // Create health timer task
    OSTmrCreate(&g_health_timer,
                "Scuba Health Timer",
                0,
                167 / 4u,
                OS_OPT_TMR_PERIODIC,
                scuba_health_task,
                NULL,
                &err);
    assert(OS_ERR_NONE == err);
    OSTmrStart(&g_health_timer, &err);
    assert(OS_ERR_NONE == err);
    
     // Create the dive calculator task.
    OSTaskCreate((OS_TCB     *)&g_calc_tcb,
                 (CPU_CHAR   *)"Dive Calculations",
                 (OS_TASK_PTR ) calculator_task,
                 (void       *) 0,
                 (OS_PRIO     ) CALC_PRIO,
                 (CPU_STK    *)&g_calc_stack[0],
                 (CPU_STK_SIZE) TASK_STACK_SIZE / 10u,
                 (CPU_STK_SIZE) TASK_STACK_SIZE,
                 (OS_MSG_QTY  ) 0u,
                 (OS_TICK     ) 0u,
                 (void       *) 0,
                 (OS_OPT      ) 0,
                 (OS_ERR     *)&err);
    assert(OS_ERR_NONE == err);
    
    // Delete the startup task (or enter an infinite loop like other tasks).
    OSTaskDel((OS_TCB *)0, &err);

    // We should never get here.
    assert(0); 
}


/*
*********************************************************************************************************
*                                               main()
*
* Description : Entry point for C code.
*
* Arguments   : none.
*
* Returns     : none.
*
* Note(s)     : (1) It is assumed that your code will call main() once you have performed all necessary
*                   initialization.
*********************************************************************************************************
*/

void  main (void)
{
    OS_ERR  err;



    CPU_IntDis();                                               /* Disable all interrupts.                              */

    BSP_IntVectSet(27, (CPU_FNCT_VOID)OSCtxSwISR);              /* Setup kernel context switch                          */

    OSInit(&err);                                               /* Init uC/OS-III.                                      */
    assert(OS_ERR_NONE == err);

    // Install application-specific OS hooks.
    App_OS_SetAllHooks();

    // Create the startup task.
    OSTaskCreate((OS_TCB     *)&g_startup_tcb,
                 (CPU_CHAR   *)"Startup Task",
                 (OS_TASK_PTR ) startup_task,
                 (void       *) 0,
                 (OS_PRIO     ) STARTUP_PRIO,
                 (CPU_STK    *)&g_startup_stack[0],
                 (CPU_STK_SIZE) TASK_STACK_SIZE / 10u,
                 (CPU_STK_SIZE) TASK_STACK_SIZE ,
                 (OS_MSG_QTY  ) 0u,
                 (OS_TICK     ) 0u,
                 (void       *) 0,
                 (OS_OPT      ) 0,
                 (OS_ERR     *)&err);
    assert(OS_ERR_NONE == err);

    OSStart(&err);                                              /* Start multitasking (i.e. give control to uC/OS-III). */

    // We should never get here.
    assert(0);
}

