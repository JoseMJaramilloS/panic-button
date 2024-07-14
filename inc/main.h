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
#define BUTTON_PIN  16 // Define interrupt button
#define DEBOUNCE_TIME    500000

#define WAITFORINT() __asm volatile ("wfi")
#define BUTTON_GPIO 10  // Define interrupt button
#define OPEN_GPIO 14 // Define open case interrupt button
#define DEBOUNCE_STABLE_MS 50



#endif