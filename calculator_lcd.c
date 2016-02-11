#include "calculator_lcd.h"

#include <bsp_glcd.h>
#include <stdio.h>
#include <stdarg.h>

void calculator_lcd_init() {
  BSP_GraphLCD_SetFont(GLYPH_FONT_8_BY_8);
}

static void lcd_printf(uint8_t line, const char* msg, ...) {
  va_list args;
  char  p_str[LCD_CHARS_PER_LINE+1];
  
  va_start(args, msg);
  
  vsprintf(p_str, msg, args);
  BSP_GraphLCD_String(line, (char const *) p_str);
  
  va_end(args);
}

#define MM_TO_M(mm) (mm / 1000)
#define MM_TO_FT(mm) (mm / 305) // 304.8
#define ML_TO_L(ml) (ml / 1000)

void calculator_lcd_update(CalculationState* state) {
  lcd_printf(0, "SCUBIE DUUBA");
  if(state->display_units == CALC_UNITS_METRIC) {
    lcd_printf(3, "DEPTH: %4u M", MM_TO_M(state->depth_mm));
    lcd_printf(4, "RATE: %4u M", MM_TO_M(state->rate_mm_per_m));
    
  } else {
    lcd_printf(3, "DEPTH: %4u FT", MM_TO_FT(state->depth_mm));
    lcd_printf(4, "RATE: %4u FT", MM_TO_FT(state->rate_mm_per_m));
  }
  
  lcd_printf(5, "AIR: %4u L", ML_TO_L(state->depth_mm));
  lcd_printf(6, "EDT: ");
  lcd_printf(8, "Alarm: ");
}


