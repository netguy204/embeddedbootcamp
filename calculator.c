#include "calculator.h"
#include "calculator_lcd.h"
#include "pushbutton.h"
#include "alarm.h"
#include "adc.h"

#include "scuba.h"
#include "adc.h"
#include  "assert.h"

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

void updateAlarms(CalculationState *currState){
  if(gas_to_surface_in_cl(currState->depth_mm)>currState->air_ml)
    currState->current_alarm|=ALARM_HIGH;
  if(15<currState->rate_mm_per_m)
    currState->current_alarm|=ALARM_MEDIUM;
  if(40000<currState->depth_mm)
    currState->current_alarm|=ALARM_LOW;  
}

void calculator_task(void* vptr) {


  int32_t adc = 0;
  CalculationState calcState;
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
  
  for (;;) 
  {

  while (1)
  {
      // determine  DisplayUnits - check if SW2 has been toggled
      OSSemPend(&g_sw2_sem, 0, OS_OPT_PEND_NON_BLOCKING, 0, &err);
      // Check for change
      if (OS_ERR_NONE == err)
      {
        calcState.display_units = (calcState.display_units == CALC_UNITS_METRIC ? CALC_UNITS_IMPERIAL : CALC_UNITS_METRIC);
      } else if (OS_ERR_PEND_WOULD_BLOCK == err) 
      {
        // no update 
        break;
      } else 
      {
        assert(0); // error state
      }
  }
    
    adc = adc_read();
  
    // calculate ASCENT RATE  int32_t rate_mm_per_m;
    int32_t descent_rate = adc2rate(adc);
    
    if(calcState.depth_mm > 0 || (calcState.depth_mm == 0 && descent_rate > 0)) {
        calcState.rate_mm_per_m = 1000 * descent_rate;
    } else {
        calcState.rate_mm_per_m = 0;
    }
    
    // calculate DEPTH  int32_t depth_mm;
    calcState.depth_mm += depth_change_in_mm(calcState.rate_mm_per_m);
       
    // no flying divers
    if(calcState.depth_mm < 0) {
        calcState.depth_mm = 0;
    }
    
    // calculate  uint32_t air_ml;
    calcState.air_ml += 2 * 0.5 * gas_rate_in_cl(calcState.depth_mm);
    // calculate  elapsed time (always a delta of 500 ms)
    calcState.elapsed_time_s += 0.5;
    // determine  DisplayUnits - check if SW2 has been toggled

    // determine alarm state  enum CurrentAlarm current_alarm;
    calculator_lcd_update(&calcState);
    
    // post alarm flags
    updateAlarms(&calcState);
    post_alarms(&calcState);    

    // sleep 500 ms
    OSTimeDlyHMSM(0, 0, 0, 500, OS_OPT_TIME_HMSM_STRICT, &err);
  }
}
