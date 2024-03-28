#include "gps.h"

uart_inst_t *uart_port_g;
// uint8_t gps_data[256];
uint8_t i = 0;

/**
* @brief Funcion que inicializa el protocolo UART para la lectura de los datos del GPS
* @param tx_pin pin de configuracion tx
* @param rx_pin pin de configuracion 
* @param uart_inst_t puerto UART de la Raspberry usada
* @param baud_rate numero de baudios
*/
void gps_init(uint8_t tx_pin, uint8_t rx_pin, uart_inst_t *uart_port, uint baud_rate)
{
    uart_port_g = uart_port;
    uart_init(uart_port_g, baud_rate);
    gpio_set_function(tx_pin, GPIO_FUNC_UART);
    gpio_set_function(rx_pin, GPIO_FUNC_UART);
    uart_set_hw_flow(uart_port_g, false, false);
    uart_set_format(uart_port_g, DATA_LEN, STOP_BITS, PARITY);
}
/**
* @brief Funcion que lee los datos del GPS
*/
bool read_raw_line(uint8_t gps_data[MAX_GPS_DATA_SIZE]){
    // static uint8_t i = 0; // si es static, es persistente
    
    while (uart_is_readable(uart_port_g)) {
        char c = uart_getc(uart_port_g);
        // printf("%c", c);
        // putchar(c); // Envía el carácter al USB
        gps_data[i++]= c;
        if (c == '\n') {
            gps_data[i] = '\0';  // Agrega un terminador nulo al final de la cadena
            // printf("--%s", gps_data);
            i = 0;
            return true; // Trama completa recibida
        }
        if (i >= MAX_GPS_DATA_SIZE - 1) {
            i = 0; // Reinicia el índice si se alcanza el límite del tamaño del búfer
        }
    }    
    return false; // Trama no completa
}

bool read_gps_coor(double *latitude, double *longitude){
    bool finding = true;
    uint8_t gps_data[256];
    uint16_t lat_degrees, lon_degrees;
    float lat_minutes, lon_minutes;
    uint8_t *lat_direction, *lon_direction;

    while(finding){
        if (read_raw_line(gps_data)) {
            printf("%s", gps_data); // Imprime solo si se recibe una trama completa
            if (strncmp(gps_data, "$GNGGA", strlen("$GNGGA")) == 0){ //|| strncmp(gps_data, "$GNRMC", strlen("$GNRMC")) == 0){
                finding = false;
                // printf("%s", gps_data);
                uint8_t delim[] = ",";
                uint8_t *token = strtok(gps_data, delim);
                if(token != NULL){
                    for (uint8_t i = 0; token != NULL; i++){
                        // printf("Token (%d): %s\n", i, token);
                        if (i == 2) {
                            lat_degrees = atoi((char*)token) / 100;
                            lat_minutes = atof((char*)token + 2);
                            *latitude = lat_degrees + (lat_minutes / 60.0);
                            // printf("Latitud: %f\n", latitude);
                        }
                        else if (i == 3){
                            lat_direction = token;
                            // printf("%s",lat_direction);
                            if (*lat_direction == 'S') {
                                *latitude = -*latitude;
                            }
                        }
                        else if (i == 4){
                            lon_degrees = atoi((char*)token) / 100;
                            lon_minutes = atof((char*)token + 3);
                            *longitude = lon_degrees + (lon_minutes / 60.0);
                            // printf("Longitud: %f\n", longitude);
                        }
                        else if (i == 5){
                            lon_direction = token;
                            // printf("%s",lon_direction);
                            if (*lon_direction == 'W') {
                                *longitude = -*longitude;
                            }
                        }
                        token = strtok(NULL, delim);
                    }
                    // printf("%f,%f\n",*latitude, *longitude);
                    return true;
                }
            }
        }
    }
    return false;
}

