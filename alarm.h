/** \file alarm.h
*
* @brief Alarm Manager
*
* @par
* COPYRIGHT NOTICE: (c) 2011 Netrino, LLC.
* All rights reserved.
*/

#ifndef _ALARM_H
#define _ALARM_H

#define BIT(n)  (1 << (n))

#define ALARM_NONE    BIT(0)
#define ALARM_LOW     BIT(1)
#define ALARM_MEDIUM  BIT(2)
#define ALARM_HIGH    BIT(3)

extern OS_FLAG_GRP g_alarm_flags;

void alarm_task(void * p_arg);

#endif /* _ALARM_H */
