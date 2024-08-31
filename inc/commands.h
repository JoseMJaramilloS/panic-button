#ifndef COMMANDS_H
#define COMMANDS_H

// #include <stdint.h>  // Para tipos de datos como uint8_t
// #include <stdbool.h> // Para usar bool
#include "debug.h"

// Tamaño del buffer para la recepción de comandos
#define CMD_BUFFER_SIZE 256

void hex_to_ascii(const char *hex_payload, char *ascii_string);
void process_command(const char* command);

#endif // COMMANDS_H
