#include "lorawan.h"

uart_inst_t *uart_port_g_lora;

// LoRa RX interrupt handler (DESACTIVADA POR EL MOMENTO)
// Se usaría para capturar datos entrantes al modulo mediante interrupcion.
void on_uart_rx() {
    while (uart_is_readable(uart_port_g_lora)) {
        char received_char = uart_getc(uart_port_g_lora);
        printf("%c", received_char);
    }
}

void lorawan_init(uint8_t tx_pin, uint8_t rx_pin, uart_inst_t *uart_port, uint baud_rate){
    uart_port_g_lora = uart_port;

    // Initialize UART pin
    uart_init(uart_port_g_lora, baud_rate);
    gpio_set_function(tx_pin, GPIO_FUNC_UART);
    gpio_set_function(rx_pin, GPIO_FUNC_UART);
    uart_set_hw_flow(uart_port_g_lora, false, false);
    uart_set_format(uart_port_g_lora, DATA_LEN, STOP_BITS, PARITY);

    // And set up and enable the interrupt handlers
    irq_set_exclusive_handler(UART0_IRQ, on_uart_rx);
    irq_set_enabled(UART0_IRQ, true);

    // Now enable the UART to send interrupts - RX only
    uart_set_irq_enables(uart_port_g_lora, false, false); // Interrupcion desactivada
}

// Construye el comando AT para enviar un payload a través del modulo lorawan
bool lora_send(uint8_t conf, uint8_t trials, uint8_t length, char *payload){
    char sendCmd[64];
    // Construye el comando 
    int written = snprintf(sendCmd, sizeof(sendCmd), "AT+DTRX=%u,%u,%u,%s\n", conf, trials, length, payload);
    if (written < 0 || written >= sizeof(sendCmd)) {
        // printf("Error: sending buffer exceeded or error formating .\n");
        return false; 
    }

    // printf("sending unconfirmed message %s\n", payload);
    printf("%s\n", sendCmd); // Imprime el resultado
    uart_puts(uart_port_g_lora, sendCmd); // Pone el comando construido en el UART

    return true;
    // uart_puts(uart_port_g_lora, "AT+DTRX=0,2,62,412C20362E3231353231302C202D37352E353833333935\n"); // only testing
}
