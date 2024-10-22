#ifndef ALARM_H
#define ALARM_H

#include "hardware/rtc.h"
#include "pico/util/datetime.h"
#include "debug.h"

extern datetime_t t;

void alarm_init(void);
void alarm_period(int minutes, rtc_callback_t user_callback);
void print_current_datetime(void);

#endif // ALARM_H