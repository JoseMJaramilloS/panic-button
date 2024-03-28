#include "lorawan.h"

uart_inst_t *uart_port_g_lora;

// LoRa RX interrupt handler (DESACTIVADA POR EL MOMENTO)
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
    uart_set_irq_enables(uart_port_g_lora, false, false); //-----------------------
}

bool lora_send(uint8_t conf, uint8_t trials, uint8_t length, char *payload){
    char sendCmd[64];
    char loraCmd[] ="AT+DTRX="; //"AT+DTRX=0,2,62,"
    
    char strConf[2];
    char strTrials[3];
    char strLength[3];
    sprintf(strConf, "%u", conf); // Convertimos el uint8_t a string
    sprintf(strTrials, "%u", trials);
    sprintf(strLength, "%u", length);

    printf("sending unconfirmed message %s\n", payload);

    strcpy(sendCmd, loraCmd); // Copia loraCmd a senCmd
    strcat(sendCmd, strConf); // Concatena la segunda cadena a sendCmd
    strcat(sendCmd, ",");
    strcat(sendCmd, strTrials);
    strcat(sendCmd, ",");
    strcat(sendCmd, strLength);
    strcat(sendCmd, ",");
    strcat(sendCmd, payload); 
    strcat(sendCmd, "\n"); // Concatena LF
    printf("%s\n", sendCmd); // Imprime el resultado
    uart_puts(uart_port_g_lora, sendCmd);

    return true;
    
    // printf("sending unconfirmed message\n");
    // uart_puts(uart_port_g_lora, "AT+DTRX=0,2,62,412C20362E3231353231302C202D37352E353833333935\n");
}
