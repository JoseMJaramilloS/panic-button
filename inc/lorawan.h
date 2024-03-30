#ifndef LORALIB_H
#define LORALIB_H

#include <stdint.h> // Para uint8_t
#include <string.h> // Para strcat
#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/stdio_uart.h"

#define LORA_BAUD_RATE 9600
#define DATA_LEN 8
#define STOP_BITS 1
#define LORA_TX 12
#define LORA_RX 13
#define LORA_UART_ID uart0
#define PARITY UART_PARITY_NONE

void lorawan_init(uint8_t tx_pin, uint8_t  rx_pin, uart_inst_t *uart_port, uint baud_rate);
bool lora_send(uint8_t conf, uint8_t trials, uint8_t length, char *payload);

#endif // LORALIB_H