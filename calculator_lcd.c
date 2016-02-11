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

void calculator_lcd_update(struct CalculationState* state) {
  lcd_printf(0, "SCUBIE DUUBA");
}


