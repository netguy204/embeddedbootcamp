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
  uint16_t tankChangeInLiters = 0;
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
   
    // check SW2 air changes
    tankChangeInLiters =   getTankChange_inLiters();
    if ((calcState.air_ml / 1000) + tankChangeInLiters > 2000) {
      calcState.air_ml = 2000 * 1000;  // max value = 2000 L
    } else {
       // calculate air_ml assuming 500ms have passed since last update
      calcState.air_ml += gas_rate_in_cl(calcState.depth_mm);     
    }

    // calculate  elapsed time (always a delta of 500 ms)
    calcState.elapsed_time_s = get_dive_time_in_seconds();
  

    // calculate DEPTH  int32_t depth_mm;
    calcState.depth_mm += depth_change_in_mm(calcState.rate_mm_per_m); 
    
    // no flying divers
    if(calcState.depth_mm < 0) {
        calcState.depth_mm = 0;
    }
 
    if (calcState.depth_mm > 0)
    {
        if (is_timer_off()) 
        {
            start_timer(b_is_new_timer);
            // This will only trigger the first time we dive
            if (b_is_new_timer)
            {
                b_is_new_timer = 0;
            }
        }
    }
    else
    {
        if (!b_is_new_timer)
        {
            stop_timer();
        }
    }

    // calculate  uint32_t air_ml;
    calcState.air_ml -= gas_rate_in_cl(calcState.depth_mm);
    // calculate  elapsed time (always a delta of 500 ms)
    calcState.elapsed_time_s = get_dive_time_in_seconds();

    // determine  DisplayUnits - check if SW2 has been toggled

    // determine alarm state  enum CurrentAlarm current_alarm;
    calculator_lcd_update(&calcState);

    // sleep 500 ms
    OSTimeDlyHMSM(0, 0, 0, 500, OS_OPT_TIME_HMSM_STRICT, &err);
  }
}
