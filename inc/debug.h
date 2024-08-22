#ifndef DEBUG_H
#define DEBUG_H

#include <stdio.h>
#include <stdarg.h>
#include "pico/stdio_uart.h"

void safe_printf(const char *format, ...);

#endif // DEBUG_H
