/**
 * @file main.h
 * @brief This is a brief description of the main H file.
 *
 * Detailed description of the main C file.
 */
 
// Avoid duplication in code
#ifndef _MAIN_H_
#define _MAIN_H_

// Write your definitions and other macros here

#define WAITFORINT() __asm volatile ("wfi")
#define BUTTON_GPIO 10  // Define interrupt button
#define OPEN_GPIO 14 // Define open case interrupt button
#define DEBOUNCE_STABLE_MS 50
#define DEVICE_ID "0184D69C" //(Identifier)
//0184D695 (Primer boton) 
//0184D69C (Segundo boton) 


#endif