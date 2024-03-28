#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "hardware/gpio.h"
#include "hardware/irq.h"
#include <stdint.h> // Para uint8_t
#include <string.h> // Para strcat
#include <stdio.h>
#include "gps.h"

#define BAUD_RATE 9600
#define DATA_LEN 8
#define STOP_BITS 1
#define PARITY UART_PARITY_NONE
#define GPS_TX 8 // Se conecta al RX del GPS
#define GPS_RX 9 // Se conecta al TX del GPS
#define UART_ID_GPS uart1
#define LORA_TX 12
#define LORA_RX 13
#define UART_ID_LORA uart0
#define BUTTON_GPIO 10  // Define interrupt button
#define DEBOUNCE_STABLE_MS 50

volatile uint32_t last_change_time = 0;
bool last_button_state = false;
char cmd[16]; // Commands monitor serial

char cmdGPS[64];
char sendCmd[] ="AT+DTRX=0,2,62,";

uint8_t gps_data[MAX_GPS_DATA_SIZE];
double latitude, longitude;
bool button_pressed = true;

// RX interrupt handler
void on_uart_rx() {
    while (uart_is_readable(UART_ID_LORA)) {
        char received_char = uart_getc(UART_ID_LORA);
        printf("%c", received_char);
    }
}

// Button interrupt handler
void gpio_callback(uint gpio, uint32_t events) {
    uint32_t current_time = to_ms_since_boot(get_absolute_time());
    bool current_button_state = gpio_get(BUTTON_GPIO);
    // Comprobar si ha pasado suficiente tiempo desde el último cambio para considerarlo estable
    if ((current_time - last_change_time) > DEBOUNCE_STABLE_MS && current_button_state != last_button_state) {
        last_change_time = current_time;
        last_button_state = current_button_state;

        if (!current_button_state) {  // Botón presionado (considerando pull-up)
            printf("Botón presionado!\n");
            if (read_gps_coor(&latitude, &longitude)){
                printf("%f,%f\n",latitude, longitude);

                uint8_t len = snprintf(NULL,0, "1, %f, %f", latitude, longitude);
                char message[len];
                sprintf(message, "%d, %f, %f", button_pressed, latitude, longitude);
                
                printf("sending unconfirmed message %s\n", message);

                char hex_payload[100];
                int i;
                for (i = 0; message[i] != '\0'; i++) {
                    sprintf(&hex_payload[i * 2], "%02x", message[i]);
                }
                hex_payload[i * 2] = '\0'; // Agrega el carácter nulo al final de la cadena
                // printf("%s\n", hex_payload);
                // sprintf(str, "%x", message);
                // printf("%s\n", str);

                strcpy(cmdGPS, sendCmd); // Copia la primera cadena a resultado
                strcat(cmdGPS, hex_payload); // Concatena la segunda cadena a resultado
                strcat(cmdGPS,"\n"); // Concatena la segunda cadena a resultado
                printf("%s\n", cmdGPS); // Imprime el resultado
                uart_puts(UART_ID_LORA, cmdGPS);
                
            }
            // uart_puts(UART_ID_LORA, "AT+DTRX=0,2,62,412C20362E3231353231302C202D37352E353833333935\n");

        }
    }
}

int main() {
    stdio_init_all();

    // Initialize UART pin
    uart_init(UART_ID_LORA, BAUD_RATE);
    gpio_set_function(LORA_TX, GPIO_FUNC_UART);
    gpio_set_function(LORA_RX, GPIO_FUNC_UART);
    uart_set_hw_flow(UART_ID_LORA, false, false);
    uart_set_format(UART_ID_LORA, DATA_LEN, STOP_BITS, PARITY);

    // And set up and enable the interrupt handlers
    irq_set_exclusive_handler(UART0_IRQ, on_uart_rx);
    irq_set_enabled(UART0_IRQ, true);

    // Now enable the UART to send interrupts - RX only
    uart_set_irq_enables(UART_ID_LORA, false, false); //-----------------------

    // Initialize GPIO for button with pull-up
    gpio_init(BUTTON_GPIO);
    gpio_set_dir(BUTTON_GPIO, GPIO_IN);
    gpio_pull_up(BUTTON_GPIO);

    // GPIO IRQ callback
    gpio_set_irq_enabled_with_callback(BUTTON_GPIO, GPIO_IRQ_EDGE_FALL, true, &gpio_callback);

    // GPS Initialization
    gps_init(GPS_TX, GPS_RX, UART_ID_GPS, 9600);
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
        tight_loop_contents();
        
    }
    return 0;
}
