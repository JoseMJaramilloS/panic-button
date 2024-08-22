#ifndef EVENTSLIB_H
#define EVENTSLIB_H

#include "pico/stdlib.h"

typedef union {
	uint8_t reg;
	struct {
		uint8_t flag0 : 1;
		uint8_t flag1 : 1;
		uint8_t flag2 : 1;
		uint8_t flag3 : 1;
		uint8_t flag4 : 1;
		uint8_t flag5 : 1;
		uint8_t flag6 : 1;
		uint8_t flag7 : 1;
	}flags;
}_events_str;

extern volatile _events_str _events;

#define PENDING_EVENTS	_events.reg
#define EV_BUTTON		_events.flags.flag0
#define EV_OPEN			_events.flags.flag1
#define EV_UART_RX		_events.flags.flag2

// void eventsController(void);

#endif // EVENTSLIB_H