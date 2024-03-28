#ifndef LORALIB_H
#define LORALIB_H

// #include "hardware/uart.h"
// #include "hardware/gpio.h"
// #include "hardware/irq.h"
#include <stdint.h> // Para uint8_t
#include <string.h> // Para strcat
#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/stdio_uart.h"

#define DATA_LEN 8
#define STOP_BITS 1
#define PARITY UART_PARITY_NONE

void lorawan_init(uint8_t tx_pin, uint8_t  rx_pin, uart_inst_t *uart_port, uint baud_rate);
bool lora_send(uint8_t conf, uint8_t trials, uint8_t length, char *payload);

#endif // LORALIB_H