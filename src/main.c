#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "hardware/gpio.h"
#include "hardware/irq.h"
#include <stdint.h> // Para uint8_t
#include <string.h> // Para strcat
#include <stdio.h>
#include "gps.h"
#include "lorawan.h"
#include "events.h"

#define BAUD_RATE 9600
#define GPS_TX 8 // Se conecta al RX del GPS
#define GPS_RX 9 // Se conecta al TX del GPS
#define GPS_UART_ID uart1
#define LORA_TX 12
#define LORA_RX 13
#define LORA_UART_ID uart0
#define BUTTON_GPIO 10  // Define interrupt button
#define OPEN_GPIO 14 // Define open case interrupt button
#define DEBOUNCE_STABLE_MS 50

volatile uint32_t last_change_time = 0;
bool last_button_state = false;
char cmd[16]; // Send Commands monitor serial

double latitude, longitude;
bool button_pressed = false;

volatile _events_str _events; // Events structure

// Button interrupt handler
void gpio_callback(uint gpio, uint32_t events) {
    if(gpio == BUTTON_GPIO){
        uint32_t current_time = to_ms_since_boot(get_absolute_time());
        bool current_button_state = gpio_get(BUTTON_GPIO);
        // Comprobar si ha pasado suficiente tiempo desde el último cambio para considerarlo estable
        if ((current_time - last_change_time) > DEBOUNCE_STABLE_MS && current_button_state != last_button_state) {
            last_change_time = current_time;
            last_button_state = current_button_state;

            if (!current_button_state) {  // Botón presionado (considerando pull-up)
                printf("Botón presionado!\n");
                EV_BUTTON = 1;
            }
        }
    }
    if(gpio == OPEN_GPIO){
        // To be implemented
    }
}

int main() {
    stdio_init_all();

    // Initialize GPIO for button with pull-up
    gpio_init(BUTTON_GPIO);
    gpio_set_dir(BUTTON_GPIO, GPIO_IN);
    gpio_pull_up(BUTTON_GPIO);

    // GPIO IRQ callback
    gpio_set_irq_enabled_with_callback(BUTTON_GPIO, GPIO_IRQ_EDGE_FALL, true, &gpio_callback);

    // LoRaWAN module initialization
    lorawan_init(LORA_TX, LORA_RX, LORA_UART_ID, BAUD_RATE);

    // GPS Initialization
    gps_init(GPS_TX, GPS_RX, GPS_UART_ID, BAUD_RATE);
    sleep_ms(3000);

    while (1) {
        // printf("Command input: ");
        // fgets(cmd, sizeof(cmd), stdin);  // Read command from stdin (enable LF)

        // printf("Sending command\n");
        // uart_puts(UART_ID_LORA, cmd);
        //-------------------------------------------
        // while (uart_is_readable(uart0)) {
        //     char received_char = uart_getc(uart0);
        //     printf("%c", received_char);
        // }
        //-------------------------------------------
        // if (read_gps_coor(&latitude, &longitude)){
        //     printf("%f,%f\n",latitude, longitude);
        // }
        // tight_loop_contents();
        if (!PENDING_EVENTS){
            continue; // wfi
        }

        if (EV_BUTTON){
            EV_BUTTON = 0;
            button_pressed = 1;
            if (read_gps_coor(&latitude, &longitude)){
                // latitude = 6.2152100;
                // longitude = -75.5833950;
                printf("%f,%f\n", latitude, longitude);

                char message[64];
                int len = snprintf(message, sizeof(message),"%d, %f, %f", button_pressed, latitude, longitude);

                int i;
                char hex_payload[100];
                for (i = 0; message[i] != '\0'; i++){
                    sprintf(&hex_payload[i*2], "%02x", message[i]);
                }
                hex_payload[i*2] = '\0'; // Agrega el carácter nulo al final de la cadena

                lora_send(0, 2, len, hex_payload); // se puede mejorar (?)
            }
        }

        if (EV_OPEN){
            EV_OPEN = 0;
            button_pressed = 0;
            // To be implemented
        }
        
        
    }
    return 0;
}
