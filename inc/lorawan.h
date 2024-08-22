#ifndef _LORAWAN_H_
#define _LORAWAN_H_

#include <stdio.h>
//#include <stdint.h>
//#include <string.h>
#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "debug.h"

#define LORA_UART_ID uart0
#define LORA_TX 0
#define LORA_RX 1
#define LORA_BAUD_RATE 9600

#define DATA_BITS 8
#define STOP_BITS 1
#define PARITY UART_PARITY_NONE
#define CHANNEL 2
#define DR 5



void lorawan_init(uint8_t che, uint8_t dr);
bool lora_send(uint8_t conf, uint8_t fPort, uint8_t length, char *payload);
#endif