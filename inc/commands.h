#ifndef COMMANDS_H
#define COMMANDS_H

#include "debug.h"

// Tamaño del buffer para la recepción de comandos
#define CMD_BUFFER_SIZE 256

// Parametros
extern int alarm_interval_minutes;

static void hex_to_ascii(const char *hex_payload, char *ascii_string);
void process_command(const char* command);

#endif // COMMANDS_H
