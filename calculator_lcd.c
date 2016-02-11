#include "calculator_lcd.h"

#include <bsp_glcd.h>
#include <stdio.h>
#include <stdarg.h>

void calculator_lcd_init() {
  BSP_GraphLCD_SetFont(GLYPH_FONT_8_BY_8);
}

static char* blank_line = "                ";

static void lcd_printf(uint8_t line, const char* msg, ...) {
  va_list args;
  char  p_str[LCD_CHARS_PER_LINE+1];
  
  va_start(args, msg);
  
  vsprintf(p_str, msg, args);
  
  BSP_GraphLCD_String(line, blank_line);
  BSP_GraphLCD_String(line, (char const *) p_str);
  
  va_end(args);
}

#define MM_TO_M(mm) (mm / 1000)
#define MM_TO_FT(mm) (mm / 305) // 304.8
#define ML_TO_L(ml) (ml / 1000)

void calculator_lcd_update(CalculationState* state) {
  //BSP_GraphLCD_Clear();
    
  lcd_printf(0, "SCUBIE DUUBA");
  
  if(state->air_ml == 0) {
    BSP_GraphLCD_Clear();
    lcd_printf(3, "     YOU LOSE");
  } else {
    if(state->display_units == CALC_UNITS_METRIC) {
      lcd_printf(2, "DEPTH: %4d M", MM_TO_M(state->depth_mm));
      lcd_printf(3, "RATE: %+5d M", MM_TO_M(state->rate_mm_per_m));
      
    } else {
      lcd_printf(2, "DEPTH: %4d FT", MM_TO_FT(state->depth_mm));
      lcd_printf(3, "RATE: %+5d FT", MM_TO_FT(state->rate_mm_per_m));
    }
    
    lcd_printf(4, "AIR: %7u L", ML_TO_L(state->air_ml));
    
    uint32_t seconds = state->elapsed_time_s % 60;
    uint32_t remainder_minutes = state->elapsed_time_s / 60;
    uint32_t minutes = remainder_minutes % 60;
    uint32_t hours = remainder_minutes / 60;
    
    lcd_printf(5, "EDT:      %01u:%02u:%02u", hours, minutes, seconds);
  }
  
      
  char* alarm;
  if(ALARM_HIGH & state->current_alarms) {
    alarm = "HIGH";
  } else if(ALARM_MEDIUM & state->current_alarms) {
    alarm = "MEDIUM";
  } else if(ALARM_LOW & state->current_alarms) {
    alarm = "LOW";
  } else {
    alarm = "NONE";
  }
  
  lcd_printf(7, "Alarm:    %s", alarm);
}


