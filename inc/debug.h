#ifndef DEBUG_H
#define DEBUG_H

#include <stdio.h>
#include <stdarg.h>
#include "pico/stdio_uart.h"

#define ENABLE_DEBUG 1  // 0 para desactivar el debug por consola

#if ENABLE_DEBUG
void safe_printf(const char *format, ...);
#else
// Se desactiva debug
#define safe_printf(format, ...) ((void)0)
#endif

#endif // DEBUG_H
