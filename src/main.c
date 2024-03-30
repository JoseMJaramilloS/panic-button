#include <stdint.h> // Para uint8_t
#include <string.h> // Para strcat
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "hardware/gpio.h"
#include "hardware/irq.h"
#include "pico/sleep.h" // pico_sleep: dormant mode
#include "gps.h"
#include "lorawan.h"
#include "events.h"

#include "hardware/clocks.h"
#include "hardware/rosc.h"
#include "hardware/structs/scb.h"
#include "hardware/pll.h"
#include "hardware/xosc.h"
#include "hardware/watchdog.h"
#include "hardware/regs/clocks.h"
#define RTC_CLOCK_FREQ_HZ       (USB_CLK_KHZ * KHZ / 1024)


#define BAUD_RATE 9600
#define GPS_TX 8 // Se conecta al RX del GPS
#define GPS_RX 9 // Se conecta al TX del GPS
#define GPS_UART_ID uart1
#define LORA_TX 12
#define LORA_RX 13
#define LORA_UART_ID uart0
#define BUTTON_GPIO 10  // Define interrupt button
#define OPEN_GPIO 14 // Define open case interrupt button
#define DEBOUNCE_STABLE_MS 50

volatile uint32_t last_change_time = 0;
bool last_button_state = false;
char cmd[16]; // Send Commands monitor serial

double latitude, longitude;
bool button_pressed = false;

volatile _events_str _events; // Events structure

// Button interrupt handler
void gpio_callback(uint gpio, uint32_t events) {
    if(gpio == BUTTON_GPIO){
        uint32_t current_time = to_ms_since_boot(get_absolute_time());
        bool current_button_state = gpio_get(BUTTON_GPIO);
        // Comprobar si ha pasado suficiente tiempo desde el último cambio para considerarlo estable
        if ((current_time - last_change_time) > DEBOUNCE_STABLE_MS && current_button_state != last_button_state) {
            last_change_time = current_time;
            last_button_state = current_button_state;

            if (!current_button_state) {  // Botón presionado (considerando pull-up)
                // printf("Botón presionado!\n");
                EV_BUTTON = 1;
            }
        }
    }
    if(gpio == OPEN_GPIO){
        // To be implemented
    }
}

void recover_from_sleep(uint scb_orig, uint clock0_orig, uint clock1_orig){

    //Re-enable ring Oscillator control
    rosc_write(&rosc_hw->ctrl, ROSC_CTRL_ENABLE_BITS);

    //reset procs back to default
    scb_hw->scr = scb_orig;
    clocks_hw->sleep_en0 = clock0_orig;
    clocks_hw->sleep_en1 = clock1_orig;

    //reset clocks
    clocks_init();
    // stdio_init_all();
    // stdio_uart_init();
    // stdio_usb_init();

    return;
}

void my_clocks_init(void) {
    // Start tick in watchdog, the argument is in 'cycles per microsecond' i.e. MHz
    watchdog_start_tick(XOSC_KHZ / KHZ);

    // Disable resus that may be enabled from previous software
    clocks_hw->resus.ctrl = 0;

    // Enable the xosc
    xosc_init();

    //Re-enable ring Oscillator control
    rosc_write(&rosc_hw->ctrl, ROSC_CTRL_ENABLE_BITS);

    // Before we touch PLLs, switch sys and ref cleanly away from their aux sources.
    // hw_clear_bits(&clocks_hw->clk[clk_sys].ctrl, CLOCKS_CLK_SYS_CTRL_SRC_BITS);
    // while (clocks_hw->clk[clk_sys].selected != 0x1)
    //     tight_loop_contents();
    // hw_clear_bits(&clocks_hw->clk[clk_ref].ctrl, CLOCKS_CLK_REF_CTRL_SRC_BITS);
    // while (clocks_hw->clk[clk_ref].selected != 0x1)
    //     tight_loop_contents();

    /// \tag::pll_init[]
    pll_init(pll_sys, PLL_COMMON_REFDIV, PLL_SYS_VCO_FREQ_KHZ * KHZ, PLL_SYS_POSTDIV1, PLL_SYS_POSTDIV2);
    pll_init(pll_usb, PLL_COMMON_REFDIV, PLL_USB_VCO_FREQ_KHZ * KHZ, PLL_USB_POSTDIV1, PLL_USB_POSTDIV2);
    /// \end::pll_init[]

    // Configure clocks
    // CLK_REF = XOSC (usually) 12MHz / 1 = 12MHz
    clock_configure(clk_ref,
                    CLOCKS_CLK_REF_CTRL_SRC_VALUE_XOSC_CLKSRC,
                    0, // No aux mux
                    XOSC_KHZ * KHZ,
                    XOSC_KHZ * KHZ);

    /// \tag::configure_clk_sys[]
    // CLK SYS = PLL SYS (usually) 125MHz / 1 = 125MHz
    clock_configure(clk_sys,
                    CLOCKS_CLK_SYS_CTRL_SRC_VALUE_CLKSRC_CLK_SYS_AUX,
                    CLOCKS_CLK_SYS_CTRL_AUXSRC_VALUE_CLKSRC_PLL_SYS,
                    SYS_CLK_KHZ * KHZ,
                    SYS_CLK_KHZ * KHZ);
    /// \end::configure_clk_sys[]

    // CLK USB = PLL USB 48MHz / 1 = 48MHz
    clock_configure(clk_usb,
                    0, // No GLMUX
                    CLOCKS_CLK_USB_CTRL_AUXSRC_VALUE_CLKSRC_PLL_USB,
                    USB_CLK_KHZ * KHZ,
                    USB_CLK_KHZ * KHZ);

    // CLK ADC = PLL USB 48MHZ / 1 = 48MHz
    clock_configure(clk_adc,
                    0, // No GLMUX
                    CLOCKS_CLK_ADC_CTRL_AUXSRC_VALUE_CLKSRC_PLL_USB,
                    USB_CLK_KHZ * KHZ,
                    USB_CLK_KHZ * KHZ);

    // CLK RTC = PLL USB 48MHz / 1024 = 46875Hz
    clock_configure(clk_rtc,
                    0, // No GLMUX
                    CLOCKS_CLK_RTC_CTRL_AUXSRC_VALUE_CLKSRC_PLL_USB,
                    USB_CLK_KHZ * KHZ,
                    RTC_CLOCK_FREQ_HZ);

    // CLK PERI = clk_sys. Used as reference clock for Peripherals. No dividers so just select and enable
    // Normally choose clk_sys or clk_usb
    clock_configure(clk_peri,
                    0,
                    CLOCKS_CLK_PERI_CTRL_AUXSRC_VALUE_CLK_SYS,
                    SYS_CLK_KHZ * KHZ,
                    SYS_CLK_KHZ * KHZ);
}


int main() {
    stdio_init_all();

    // Initialize GPIO for button with pull-up
    gpio_init(BUTTON_GPIO);
    gpio_set_dir(BUTTON_GPIO, GPIO_IN);
    gpio_pull_up(BUTTON_GPIO);

    // GPIO IRQ callback
    gpio_set_irq_enabled_with_callback(BUTTON_GPIO, GPIO_IRQ_EDGE_FALL, true, &gpio_callback);

    // LoRaWAN module initialization
    lorawan_init(LORA_TX, LORA_RX, LORA_UART_ID, BAUD_RATE);

    // GPS Initialization
    gps_init(GPS_TX, GPS_RX, GPS_UART_ID, BAUD_RATE);

    // Dormant state
    //save values for later
    // uint scb_orig = scb_hw->scr;
    // uint clock0_orig = clocks_hw->sleep_en0;
    // uint clock1_orig = clocks_hw->sleep_en1;
    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);
    sleep_run_from_xosc(); // UART will be reconfigured by sleep_run_from_xosc

    // sleep_goto_dormant_until_edge_high(BUTTON_GPIO); // Solo para flancos de subida
    gpio_put(PICO_DEFAULT_LED_PIN,1);
    sleep_goto_dormant_until_pin(BUTTON_GPIO, true, false);
    my_clocks_init();
    stdio_init_all();
    gpio_put(PICO_DEFAULT_LED_PIN,0);
    
    // clocks_init();
    // recover_from_sleep(scb_orig, clock0_orig, clock1_orig);
    
    while (1) {
        // printf("Command input: ");
        // fgets(cmd, sizeof(cmd), stdin);  // Read command from stdin (enable LF)

        // printf("Sending command\n");
        // uart_puts(UART_ID_LORA, cmd);
        //-------------------------------------------
        // while (uart_is_readable(uart0)) {
        //     char received_char = uart_getc(uart0);
        //     printf("%c", received_char);
        // }
        //-------------------------------------------

        if (!PENDING_EVENTS){
            // gpio_put(PICO_DEFAULT_LED_PIN,1);
            // sleep_goto_dormant_until_pin(BUTTON_GPIO, true, false);
            // gpio_put(PICO_DEFAULT_LED_PIN,0);
        }

        if (EV_BUTTON){
            gpio_put(PICO_DEFAULT_LED_PIN,1);
            EV_BUTTON = 0;
            button_pressed = 1;
            if (read_gps_coor(&latitude, &longitude)){
                // latitude = 6.2152100;
                // longitude = -75.5833950;
                // printf("%f,%f\n", latitude, longitude);

                char message[64];
                int len = snprintf(message, sizeof(message),"%d, %f, %f", button_pressed, latitude, longitude);

                int i;
                char hex_payload[100];
                for (i = 0; message[i] != '\0'; i++){
                    sprintf(&hex_payload[i*2], "%02x", message[i]);
                }
                hex_payload[i*2] = '\0'; // Agrega el carácter nulo al final de la cadena

                lora_send(0, 2, len, hex_payload);
            }
            gpio_put(PICO_DEFAULT_LED_PIN,0);
        }

        if (EV_OPEN){
            EV_OPEN = 0;
            button_pressed = 0;
            // To be implemented
        }
        
        
    }
    return 0;
}
