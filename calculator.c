#include "calculator.h"

OS_FLAG_GRP g_alarm_flags;
OS_SEM g_sw1_sem;

void post_alarms(CalculationState *currState){	  
  OSFlagPost(&g_alarm_flags, currState->current_alarm, OS_OPT_POST_FLAG_SET,&err);
  assert(OS_ERR_NONE == err);
}

uint16_t getTankChange(){
  OSSemPost(&g_sw1_sem, OS_OPT_POST_1, &err);
}

void calculator_task(void* vptr) {
  
  OSFlagCreate(&g_alarm_flags, "Alarm Flag", 0, &err);
  assert(OS_ERR_NONE == err);
  
}
