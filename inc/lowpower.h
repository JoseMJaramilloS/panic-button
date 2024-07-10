#ifndef LOWPOWERLIB_H
#define LOWPOWERLIB_H

#include <stdio.h>
#include "hardware/uart.h"
#include "hardware/clocks.h"
#include "hardware/rosc.h"

void measure_freqs(void);
void rosc_enable(void);

#endif // LOWPOWERLIB_H