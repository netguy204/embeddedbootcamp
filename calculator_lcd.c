#include "calculator_lcd.h"

#include <bsp_glcd.h>
#include <stdio.h>

void calculator_lcd_init() {
  BSP_GraphLCD_SetFont(GLYPH_FONT_8_BY_8);
}

void lcd_printf(uint8_t line, const char* msg, ...) {
  va_list args;
  char  p_str[LCD_CHARS_PER_LINE+1];
  
  va_start(args, msg);
  
  vprintf(p_str, msg, args);
  BSP_GraphLCD_String(line, (char const *) p_str);
  
  va_end(args);
}

void calculator_lcd_update(CalculationState* state) {
  char  p_str[LCD_CHARS_PER_LINE+1];
  
  lcd_printf(0, "SCUBIE DUUBA");
}


