#include "calculator.h"
#include "calculator_lcd.h"

#include "scuba.h"
#include  "assert.h"
#include  <os.h>



OS_FLAG_GRP g_alarm_flags;

void post_alarms(struct CalculationState *currState){	
  OS_ERR err;
  
  OSFlagPost(&g_alarm_flags, currState->current_alarm, OS_OPT_POST_FLAG_SET,&err);
  assert(OS_ERR_NONE == err);
}

void calculator_task(void* vptr) {


  struct  CalculationState calcState;
  uint16_t adc = 0;
  OS_ERR err;
  
  
  calculator_lcd_init();
  adc_init();
  
  
  // init values
  calcState.depth_mm = 0;
  calcState.rate_mm_per_m = 0;
  calcState.air_ml = 50 * 1000;  // init air in ml
  calcState.elapsed_time_s = 0;
  calcState.current_alarm = CALC_ALARM_NONE;
  calcState.display_units = CALC_UNITS_METRIC;
  
  OSFlagCreate(&g_alarm_flags, "Alarm Flag", 0, &err);
  assert(OS_ERR_NONE == err);
  
  for (;;) 
  {
    // get adc from queue
    adc = 0; // TEMP
// calculate DEPTH  int32_t depth_mm;
  calcState.depth_mm += 2 * 0.5 * depth_change_in_mm(calcState.rate_mm_per_m);
// calculate DIVE RATE  int32_t rate_mm_per_m;
    calcState.rate_mm_per_m =  1000 * ADC2RATE(adc);
// calculate  uint32_t air_ml;
  calcState.air_ml += 2 * 0.5 * gas_rate_in_cl(calcState.depth_mm);
// calculate  elapsed time (always a delta of 500 ms)
  calcState.elapsed_time_s += 0.5;
// determine  DisplayUnits - check if SW2 has been toggled
  
// determine alarm state  enum CurrentAlarm current_alarm;
      calculator_lcd_update(&calcState);
      
      // sleep 500 ms
      OSTimeDlyHMSM(0, 0, 0, 500, OS_OPT_TIME_HMSM_STRICT, &err);
  }
}
