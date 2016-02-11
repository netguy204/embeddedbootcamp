#ifndef CALCULATIONS_H
#define CALCULATIONS_H

#include "alarm.h"
#include <stdint.h>

enum DisplayUnits {
  CALC_UNITS_METRIC,
  CALC_UNITS_IMPERIAL
};

typedef struct {
  int32_t depth_mm;
  int32_t rate_mm_per_m;
  uint32_t air_ml;
  uint32_t elapsed_time_s;
  enum DisplayUnits display_units;
  uint8_t current_alarms;
}CalculationState;

void calculator_task(void* vptr);

#endif
