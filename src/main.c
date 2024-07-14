#include <stdint.h> // Para uint8_t
#include <string.h> // Para strcat
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "hardware/gpio.h"
#include "hardware/irq.h"
#include "pico/sleep.h" // pico_sleep: dormant mode
#include "gps.h"
#include "lorawan.h"
#include "events.h"
#include "main.h"

volatile uint32_t last_change_time = 0;
bool last_button_state = false;
uint32_t debounce_time = 0;

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
                printf("Boton presionado!\n");
                EV_BUTTON = 1;
            }
        }
    }
    if(gpio == OPEN_GPIO){
        // To be implemented
    }
}

// void gpio_int_handler(uint gpio, uint32_t events)
// {
//     if (gpio == BUTTON_PIN && (events & GPIO_IRQ_EDGE_RISE))
//     {
//         if ((time_us_32() - debounce_time) > 500000)
//         {
//             EV_BUTTON = 1;
//             gpio_put(PICO_DEFAULT_LED_PIN, 1);
//             debounce_time = time_us_32();
//         }
//     }

//     if(gpio == OPEN_GPIO){
//         // To be implemented
//     }
// }

int main() {
    stdio_init_all();

    // Initialize GPIO for button with pull-up
    gpio_init(BUTTON_GPIO);
    gpio_set_dir(BUTTON_GPIO, GPIO_IN);
    gpio_pull_up(BUTTON_GPIO);

    // GPIO IRQ callback
    gpio_set_irq_enabled_with_callback(BUTTON_GPIO, GPIO_IRQ_EDGE_FALL, true, &gpio_callback);
    // gpio_set_irq_enabled_with_callback(BUTTON_PIN, GPIO_IRQ_EDGE_RISE, true, &gpio_int_handler);

    // LoRaWAN module initialization
    lorawan_init(CHANNEL, DR);

    // GPS Initialization
    gps_init(GPS_TX, GPS_RX, GPS_UART_ID, GPS_BAUD_RATE);

    // Built-in LED initialization
    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);

    while (1) {
        if (uart_is_readable(LORA_UART_ID)) {
            char received_char = uart_getc(LORA_UART_ID);
            printf("%c", received_char);
        }

        if (!PENDING_EVENTS){
            WAITFORINT();
        }

        if (EV_BUTTON){
            // gpio_put(PICO_DEFAULT_LED_PIN,1);
            EV_BUTTON = 0;
            button_pressed = 1;
            // Para no usar GPS, comentar la siguiente linea, y descomentar los valores arbitrarios de latitud y longitud
            if (read_gps_coor(&latitude, &longitude)){
                // latitude = 6.2152100;
                // longitude = -75.5833950;
                printf("%f,%f\n", latitude, longitude);

                char payload[64];
                int len = snprintf(payload, sizeof(payload),"%d, %f, %f", button_pressed, latitude, longitude);
                lora_send(0, 2, len, payload);
                gpio_put(PICO_DEFAULT_LED_PIN, 0);
            }

            else{
                printf("No GPS data\n");
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
