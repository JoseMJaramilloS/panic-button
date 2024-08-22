#include "debug.h"

/**
 * @brief Safe printf that avoids UART conflicts.
 *
 * Temporarily disables the UART driver during printf to prevent 
 * interference with UART communications, then re-enables it.
 *
 * @param format The format string for printing.
 * @param ...    Arguments corresponding to the format string.
 */
void safe_printf(const char *format, ...) {
    stdio_set_driver_enabled(&stdio_uart, false);

    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);

    stdio_set_driver_enabled(&stdio_uart, true);
}

