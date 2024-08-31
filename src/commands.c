#include "commands.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void hex_to_ascii(const char *hex_payload, char *ascii_string) {
    // Find the position of ':' in the hex string
    const char *hex_string = strchr(hex_payload, ':');
    if (hex_string != NULL) {
        hex_string++; // Move past the ':'
    } else {
        safe_printf("Payload format error.\n");
        return;
    }
    
    size_t len = strlen(hex_string);
    if (len % 2 != 0) {
        safe_printf("Invalid hexadecimal string length.\n");
        return;
    }

    for (size_t i = 0; i < len; i += 2) {
        // Convert each pair of hexadecimal digits to a byte and store in ASCII string
        char hex_byte[3] = { hex_string[i], hex_string[i + 1], '\0' };
        ascii_string[i / 2] = (char)strtol(hex_byte, NULL, 16);
    }
    ascii_string[len / 2] = '\0'; // Null-terminate the ASCII string
}

// Implementación de la función para procesar el comando
void process_command(const char* command) {
    // char cmd_copy[CMD_BUFFER_SIZE];
    // strncpy(cmd_copy, command, CMD_BUFFER_SIZE - 1); // Copiar el comando con espacio para el nulo
    // cmd_copy[CMD_BUFFER_SIZE - 1] = '\0'; // Asegurar el carácter nulo final
    char ascii_string[256];
    hex_to_ascii(command, ascii_string);
    safe_printf("ASCII: %s\n", ascii_string);
    
    char *delimiter = ";";

    char *token = strtok(ascii_string, delimiter);
    if (token == NULL) {
        safe_printf("Comando inválido\n");
        return;
    }

    // Verificar el tipo de comando
    if (strcmp(token, "CFG") == 0) {
        // Leer parámetros
        token = strtok(NULL, delimiter);
        if (token != NULL) {
            int param1 = atoi(token); // Convertir a entero
            safe_printf("param1: %d\n", param1);
        } else {
            safe_printf("Falta param1\n");
        }

        token = strtok(NULL, delimiter);
        if (token != NULL) {
            float param2 = atof(token); // Convertir a float
            safe_printf("param2: %.6f\n", param2);
        } else {
            safe_printf("Falta param2\n");
        }

        token = strtok(NULL, delimiter);
        if (token != NULL) {
            float param3 = atof(token); // Convertir a entero
            safe_printf("param3: %.6f\n", param3);
        } else {
            safe_printf("Falta param3\n");
        }

    } else {
        safe_printf("Comando no reconocido: %s\n", token);
    }
}
