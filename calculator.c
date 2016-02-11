#include "calculator.h"

OS_FLAG_GRP g_alarm_flags;

void post_alarms(CalculationState *currState){	  
  OSFlagPost(&g_alarm_flags, currState->current_alarm, OS_OPT_POST_FLAG_SET,&err);
  assert(OS_ERR_NONE == err);
}

void calculator_task(void* vptr) {
  
  OSFlagCreate(&g_alarm_flags, "Alarm Flag", 0, &err);
  assert(OS_ERR_NONE == err);
  
}
