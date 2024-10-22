#include <stdint.h> // Para uint8_t
#include <string.h> // Para strcat
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "hardware/gpio.h"
#include "hardware/irq.h"
#include "gps.h"
#include "lorawan.h"
#include "events.h"
#include "main.h"
#include "debug.h"
#include "commands.h"
#include "alarm.h"


volatile uint32_t last_change_time = 0;
// bool last_button_state = 1;

double last_latitude, last_longitude = 0;
double latitude, longitude;
bool button_pressed = false; // deprecated soon

volatile _events_str _events; // Events structure

char rx_buffer[BUFFER_SIZE];
volatile int rx_index = 0;
volatile bool rxdata_available = 0;

int alarm_interval_minutes = 10; // Alarm period in minutes

void gpio_callback(uint gpio, uint32_t events) {
    if(gpio == BUTTON_GPIO){
        // Comprobar si ha pasado suficiente tiempo desde el último cambio para considerarlo estable
        if ((to_ms_since_boot(get_absolute_time()) - last_change_time) > DEBOUNCE_STABLE_MS) {
            last_change_time = to_ms_since_boot(get_absolute_time());

            // bool current_button_state = gpio_get(BUTTON_GPIO);
            // if (current_button_state != last_button_state){
            //     last_button_state = current_button_state;
            //     if (!current_button_state) {  // Botón presionado (considerando pull-up)
            //         printf("Boton presionado!\n");
            //         EV_BUTTON = 1;
            //     }
            // }

            safe_printf("Boton presionado!\n");
            EV_BUTTON = 1;
        }
    }
    if(gpio == OPEN_GPIO){
        // To be implemented
    }
}

void on_uart_rx() {
    gpio_put(PICO_DEFAULT_LED_PIN,1);
    while (uart_is_readable(LORA_UART_ID)) {
        char ch = uart_getc(LORA_UART_ID);

        rx_buffer[rx_index++] = ch;
        if (ch == '\n') {
            rx_buffer[rx_index] = '\0';  // Agrega un terminador nulo al final de la cadena
            // printf("--%s", gps_data);
            rx_index = 0;
            EV_UART_RX = 1; // Si se llena el buffer
        }

        if (rx_index >= BUFFER_SIZE - 1) {
            rx_index = 0;
        }
    }
    gpio_put(PICO_DEFAULT_LED_PIN,0);
}


static void alarm_callback(void) {
    EV_STATUS = 1;
}

int main() {
    stdio_init_all();

    // Initialize GPIO for button with pull-up
    gpio_init(BUTTON_GPIO);
    gpio_set_dir(BUTTON_GPIO, GPIO_IN);
    // gpio_pull_up(BUTTON_GPIO);
    gpio_set_irq_enabled_with_callback(BUTTON_GPIO, GPIO_IRQ_EDGE_FALL, true, &gpio_callback);

    // LoRaWAN module initialization
    lorawan_init(CHANNEL, DR);
    uart_set_irq_enables(LORA_UART_ID, true, false);
    irq_set_exclusive_handler(UART0_IRQ, on_uart_rx);
    irq_set_enabled(UART0_IRQ, true);
    
    sleep_ms(5000);
    uart_puts(LORA_UART_ID, "AT+CFG\n");

    // GPS Initialization
    gps_init(GPS_TX, GPS_RX, GPS_UART_ID, GPS_BAUD_RATE);

    // Built-in LED initialization
    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);

    // RTC initialization
    alarm_init();
    alarm_period(alarm_interval_minutes, alarm_callback);

    while (1) {
        // tight_loop_contents();

        if (!PENDING_EVENTS){
            WAITFORINT();
        }

        if(EV_UART_RX){
            EV_UART_RX = 0;
            safe_printf("%s", rx_buffer);

            if(rxdata_available){ // Para procesar un comando recibido desde el GW
                rxdata_available = 0;
                safe_printf("payload: %s\n", rx_buffer);
                process_command(rx_buffer);
            }

            if (strstr(rx_buffer, "AT+RECVB=?") != NULL) { // Para procesar cualquier dato recibido por UART
                safe_printf("Data received!\n");
                uart_puts(LORA_UART_ID, "AT+RECVB=?\n");
                rxdata_available = 1;
            }

        }

        if (EV_BUTTON){
            gpio_put(PICO_DEFAULT_LED_PIN,1);
            EV_BUTTON = 0;
            button_pressed = 1;
            // Para no usar GPS, comentar la siguiente linea, y descomentar los valores arbitrarios de latitud y longitud
            if (read_gps_coor(&latitude, &longitude)){


                // latitude = 6.171951;
                // longitude = -75.332695;
                // latitude = 6.171981;
                // longitude = -75.332585;
                safe_printf("%f,%f\n", latitude, longitude);

                // Si las coordenadas recibidas son correctas, se usan. Si no, se usan las ultimas que fueron correctas
                if (is_Colombia(&latitude, &longitude)){
                    last_latitude = latitude;
                    last_longitude = longitude;
                }
                else{
                    latitude = last_latitude;
                    longitude = last_longitude;
                }

                char payload[64];
                int len = snprintf(payload, sizeof(payload),"%d, %f, %f, %s", button_pressed, latitude, longitude, DEVICE_ID);
                lora_send(0, 2, len, payload);

                gpio_put(PICO_DEFAULT_LED_PIN, 0);
            }

            else{
                safe_printf("No GPS data\n");
            }
            gpio_put(PICO_DEFAULT_LED_PIN,0);
        }

        if (EV_OPEN){
            EV_OPEN = 0;
            button_pressed = 0;
            // To be implemented
        }

        if (EV_STATUS){
            gpio_put(PICO_DEFAULT_LED_PIN,1);
            EV_STATUS = 0;
            button_pressed = 0;
            print_current_datetime();
            alarm_period(alarm_interval_minutes, alarm_callback);
            // hasta el momento toca esperar que se dispare 
            // este evento para actualizar el periodo recibido

            if (read_gps_coor(&latitude, &longitude)){

                // latitude = 6.2152100;
                // longitude = -75.5833950;
                safe_printf("%f,%f\n", latitude, longitude);

                // Si las coordenadas recibidas son correctas, se usan. Si no, se usan las ultimas que fueron correctas
                if (is_Colombia(&latitude, &longitude)){
                    last_latitude = latitude;
                    last_longitude = longitude;
                }
                else{
                    latitude = last_latitude;
                    longitude = last_longitude;
                }

                char payload[64];
                int len = snprintf(payload, sizeof(payload),"%d, %f, %f, %s", button_pressed, latitude, longitude, DEVICE_ID);
                lora_send(0, 2, len, payload);

                gpio_put(PICO_DEFAULT_LED_PIN, 0);
            }

            else{
                safe_printf("No GPS data\n");
            }
            gpio_put(PICO_DEFAULT_LED_PIN, 0);
        }
        
        
        
    }
    return 0;
}
