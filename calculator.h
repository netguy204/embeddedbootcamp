#ifndef CALCULATIONS_H
#define CALCULATIONS_H

#include "alarm.h"
#include <stdint.h>

enum DisplayUnits {
  CALC_UNITS_METRIC,
  CALC_UNITS_IMPERIAL
};

enum CurrentAlarm {
  CALC_ALARM_NONE = ALARM_NONE,
  CALC_ALARM_LOW  = ALARM_LOW,
  CALC_ALARM_MEDIUM = ALARM_MEDIUM,
  CALC_ALARM_HIGH = ALARM_HIGH
};

struct CalculationState {
  int32_t depth_mm;
  int32_t rate_mm_per_m;
  uint32_t air_ml;
  uint32_t elapsed_time_s;
  enum DisplayUnits display_units;
  enum CurrentAlarm current_alarm;
};

void calculator_task(void* vptr);

#endif
