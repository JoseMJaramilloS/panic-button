/**
 * @file main.h
 * @brief This is a brief description of the main H file.
 *
 * Detailed description of the main C file.
 */
 
#ifndef _MAIN_H_
#define _MAIN_H_

#define WAITFORINT() __asm volatile ("wfi")
#define BUTTON_GPIO 10  // Define interrupt button
#define OPEN_GPIO 11 // Define open case interrupt button
#define DEBOUNCE_STABLE_MS 50
#define BUFFER_SIZE 256 // RX data buffer size
#define DEVICE_ID "0184D69C" //(Identifier)
//0184D695 (Primer boton) 
//0184D69C (Segundo boton) 


#endif