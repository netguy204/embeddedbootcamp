#ifndef CALCULATOR_LCD_H
#define CALCULATOR_LCD_H

#include "calculator.h"

#define LCD_CHARS_PER_LINE 16

void calculator_lcd_init();

void calculator_lcd_update(CalculationState* state);

#endif
