#include "calculator.h"
#include "calculator_lcd.h"
#include "pushbutton.h"
#include "alarm.h"
#include "adc.h"

#include "scuba.h"
#include "assert.h"
#include "dive_time.h"
#include  <os.h>

void post_alarms(CalculationState *currState){	
  OS_ERR err;
  
  OSFlagPost(&g_alarm_flags, currState->current_alarm, OS_OPT_POST_FLAG_SET,&err);
  assert(OS_ERR_NONE == err);
}

uint16_t getTankChange_inLiters(){
  uint16_t buttonPresses=0;
  // Wait for a signal from the button debouncer.
  OS_ERR err;
  while(OSSemPend(&g_sw1_sem, 0, OS_OPT_PEND_NON_BLOCKING, 0, &err), OS_ERR_PEND_WOULD_BLOCK==err){
    buttonPresses++;
  }
  
  return buttonPresses*5;
}

void calculator_task(void* vptr) {


  CalculationState calcState;
  uint16_t adc = 0;
  OS_ERR err;
  uint8_t b_is_new_timer = 1u;
  
  
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
    if (calcState.depth_mm < 0) 
    {
        start_timer(b_is_new_timer);
        // This will only trigger the first time we dive
        if (b_is_new_timer)
        {
            b_is_new_timer = 0;
        }
    }
    else
    {
        if (!b_is_new_timer)
        {
            stop_timer();
        }
    }
    // calculate DIVE RATE  int32_t rate_mm_per_m;
    calcState.rate_mm_per_m =  1000 * ADC2RATE(adc);
    // calculate  uint32_t air_ml;
    calcState.air_ml += 2 * 0.5 * gas_rate_in_cl(calcState.depth_mm);
    // calculate  elapsed time (always a delta of 500 ms)
    calcState.elapsed_time_s = get_dive_time_in_seconds();
    // determine  DisplayUnits - check if SW2 has been toggled

    // determine alarm state  enum CurrentAlarm current_alarm;
    calculator_lcd_update(&calcState);

    // sleep 500 ms
    OSTimeDlyHMSM(0, 0, 0, 500, OS_OPT_TIME_HMSM_STRICT, &err);
  }
}
