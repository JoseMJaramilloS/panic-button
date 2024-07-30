#ifndef GPSLIB_H
#define GPSLIB_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "pico/stdio_uart.h"

#define GPS_BAUD_RATE 9600
#define DATA_LEN 8
#define STOP_BITS 1
#define GPS_TX 8 // Se conecta al RX del GPS 
#define GPS_RX 9 // Se conecta al TX del GPS
#define GPS_UART_ID uart1
#define PARITY UART_PARITY_NONE
#define MAX_GPS_DATA_SIZE 256

void gps_init(uint8_t tx_pin, uint8_t  rx_pin, uart_inst_t *uart_port, uint baud_rate);
static bool read_raw_line(uint8_t gps_data[MAX_GPS_DATA_SIZE]);
bool read_gps_coor(double *latitude, double *longitude);
bool is_Colombia(double * lat, double * lon);
#endif // GPSLIB_H