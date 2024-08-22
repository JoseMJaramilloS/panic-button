#include "lorawan.h"

/**
 * @brief Initialize the LoRaWAN module.
 *
 * This function initializes the UART interface for communication with the LoRaWAN module
 * @param che Eight channel mode
 * @param dr Data rate
 */
void lorawan_init(uint8_t che, uint8_t dr)
{
    // UART initialization
    char sendCmd[64];
    uart_init(LORA_UART_ID, LORA_BAUD_RATE);
    gpio_set_function(LORA_TX, GPIO_FUNC_UART);
    gpio_set_function(LORA_RX, GPIO_FUNC_UART);
    uart_set_hw_flow(LORA_UART_ID, false, false);
    uart_set_format(LORA_UART_ID, DATA_BITS, STOP_BITS, PARITY);
    uart_set_fifo_enabled(LORA_UART_ID, false);

    uart_puts(LORA_UART_ID, "AT+NJM=0\n");
    sleep_ms(500);
    snprintf(sendCmd, sizeof(sendCmd), "AT+CHE=%u\n", che);
    //printf(sendCmd);
    uart_puts(LORA_UART_ID, sendCmd);
    sleep_ms(500);
    uart_puts(LORA_UART_ID, "ATZ\n");
    sleep_ms(2000);
	uart_puts(LORA_UART_ID, "AT+ADR=0\n");
    sleep_ms(500);
    snprintf(sendCmd, sizeof(sendCmd), "AT+DR=%u\n", dr);
    //printf(sendCmd);
    uart_puts(LORA_UART_ID, sendCmd);
    sleep_ms(500);

    uart_puts(LORA_UART_ID, "AT+SEND=0,2,5,CFGOK\n");
    sleep_ms(4000);
    uart_puts(LORA_UART_ID, "AT+ADR=1\n");
    // sleep_ms(500);
    // uart_puts(LORA_UART_ID, "AT+CFG\n");
}

/**
 * @brief Send data over the LoRaWAM network.
 * @param conf Confirmation status
 * @param fPort Communication port
 * @param length The number of bytes in the payload
 * @param payload A character array (string) containing the data to be sent
 * @return true if the transmission was successful, false otherwise.
*/  
bool lora_send(uint8_t conf, uint8_t fPort, uint8_t length, char* payload)
{
    char sendCmd[128];
    int i;
    char hex_payload[100];
    
    for (i = 0; payload[i] != '\0'; i++){
            sprintf(&hex_payload[i*2], "%02x", payload[i]);
        }
    hex_payload[i*2] = '\0';

    int written = snprintf(sendCmd, sizeof(sendCmd), "AT+SENDB=%u,%u,%u,%s\n", conf, fPort, length, hex_payload);
    if (written < 0 || written >= sizeof(sendCmd)) {
        safe_printf("Error: sending buffer exceeded or error formating.\n");
        return false; 
    }
    safe_printf("message: %s\n", payload);
    safe_printf("command: %s\n", sendCmd);
    uart_puts(LORA_UART_ID, sendCmd);
    return true;
    //AT+SENDB=0,2,21,302c362e3231353231302c2d37352e353833333935
}