#include "calculator.h"
#include "calculator_lcd.h"
#include "pushbutton.h"
#include "alarm.h"
#include "adc.h"

#include "scuba.h"
#include "assert.h"
#include "dive_time.h"
#include  <os.h>

void updateAlarms(CalculationState *currState){
  uint8_t any_alarms = 0;
  
  if(gas_to_surface_in_cl(currState->depth_mm)>currState->air_ml) {
    currState->current_alarms |= ALARM_HIGH;
    any_alarms = 1;
  }
  
  if(-15000>=currState->rate_mm_per_m) {
    currState->current_alarms |= ALARM_MEDIUM;
    any_alarms = 1;
  }
  
  if(40000<currState->depth_mm) {
    currState->current_alarms |= ALARM_LOW;
    any_alarms = 1;
  }
  
  if(!any_alarms) {
    currState->current_alarms = ALARM_NONE;
  }
}

void postAlarms(CalculationState *currState){	
  OS_ERR err;

  OSFlagPost(&g_alarm_flags, (OS_FLAGS)currState->current_alarms, OS_OPT_POST_FLAG_SET,&err);
  assert(OS_ERR_NONE == err);
}

uint16_t getTankChange_ml(){
  uint16_t buttonPresses=0;
  // Wait for a signal from the button debouncer.
  OS_ERR err;
  while(OSSemPend(&g_sw1_sem, 0, OS_OPT_PEND_NON_BLOCKING, 0, &err), OS_ERR_PEND_WOULD_BLOCK!=err){
    buttonPresses++;
  }
  
  return buttonPresses*5000;
}

uint8_t g_b_is_new_timer;

void timer_init() {
    g_b_is_new_timer = 1u;
}

void timer_update(CalculationState *state) {
    if (state->depth_mm > 0)
    {
        if (is_timer_off()) 
        {
            start_timer(g_b_is_new_timer);
            // This will only trigger the first time we dive
            if (g_b_is_new_timer)
            {
                g_b_is_new_timer = 0;
            }
        }
    }
    else
    {
        if (!g_b_is_new_timer)
        {
            stop_timer();
        }
    }
}

void calculator_task(void* vptr) {


  CalculationState calcState; 
  uint16_t tankChange_ml = 0;
  uint16_t adc = 0;
  OS_ERR err;
  

  calculator_lcd_init();
  adc_init();
  timer_init();
  
  // init values
  calcState.depth_mm = 0;
  calcState.rate_mm_per_m = 0;
  calcState.air_ml = 50 * 1000;  // init air in ml
  calcState.elapsed_time_s = 0;
  calcState.current_alarms = ALARM_NONE;
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
  

    /* RATE and DEPTH */
    // calculate ASCENT RATE  int32_t rate_mm_per_m;
    int32_t descent_rate = ADC2RATE(adc);
    
    if(calcState.depth_mm > 0 || (calcState.depth_mm == 0 && descent_rate > 0)) {
        calcState.rate_mm_per_m = 1000 * descent_rate;
    } else {
        calcState.rate_mm_per_m = 0;
    }
    
    // calculate DEPTH  int32_t depth_mm;
    calcState.depth_mm += depth_change_in_mm(calcState.rate_mm_per_m / 1000); 
    
    // no flying divers
    if(calcState.depth_mm < 0) {
        calcState.depth_mm = 0;
    }
    
    
    /* UPDATE AIR */
   
    // check SW2 air changes
    if(calcState.depth_mm == 0) {
        tankChange_ml =   getTankChange_ml();
        calcState.air_ml = (calcState.air_ml + tankChange_ml > 2000000) ? 2000000 : calcState.air_ml + tankChange_ml;
    } else {
        // calculate  uint32_t air_ml;
        uint32_t gas_rate = gas_rate_in_cl(calcState.depth_mm) * 10; // cl -> ml
        if(gas_rate < calcState.air_ml) {
          calcState.air_ml -= gas_rate;
        } else {
          calcState.air_ml = 0;
        }
    }
    
    /* UPDATE TIMER */

    // apply the timer logic
    timer_update(&calcState);
    
    // get value from timer
    calcState.elapsed_time_s = get_dive_time_in_seconds();
    
    // alarms
    updateAlarms(&calcState);
    postAlarms(&calcState);

    /* DISPLAY STATE */
    
    // determine alarm state  enum CurrentAlarm current_alarm;
    calculator_lcd_update(&calcState);

    // sleep 500 ms
    OSTimeDlyHMSM(0, 0, 0, 500, OS_OPT_TIME_HMSM_STRICT, &err);
  }
}
