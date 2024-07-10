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
// #define GPS_TX 8 // Se conecta al RX del GPS
// #define GPS_RX 9 // Se conecta al TX del GPS
// #define GPS_UART_ID uart1
// #define LORA_TX 12
// #define LORA_RX 13
// #define LORA_UART_ID uart0
#define LORA_TX 8
#define LORA_RX 9
#define LORA_UART_ID uart1
#define BUTTON_GPIO 10  // Define interrupt button
#define OPEN_GPIO 14 // Define open case interrupt button
#define DEBOUNCE_STABLE_MS 50

volatile uint32_t last_change_time = 0;
bool last_button_state = false;
char cmd[16]; // Send Commands monitor serial

double latitude, longitude;
bool button_pressed = false;
bool asleep = false;

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

void recover_from_sleep(uint scb_orig, uint clock0_orig, uint clock1_orig){ // UNUSED

    //Re-enable ring Oscillator control
    rosc_write(&rosc_hw->ctrl, ROSC_CTRL_ENABLE_BITS);

    //reset procs back to default
    scb_hw->scr = scb_orig;
    clocks_hw->sleep_en0 = clock0_orig;
    clocks_hw->sleep_en1 = clock1_orig;

    //reset clocks
    clocks_init();
    stdio_init_all();
    stdio_uart_init();
    // stdio_usb_init();

    return;
}

// FUNCION IMPLEMENTADA PARA CONFIGURAR CLK_SYS
void clock_sys_configure(){
    hw_clear_bits(&clocks_hw->clk[clk_sys].ctrl, 0x00E0UL);
    tight_loop_contents();
    tight_loop_contents();
    tight_loop_contents();
    tight_loop_contents();
    hw_set_bits(&clocks_hw->clk[clk_sys].ctrl, 1);
    while (!(clocks_hw->clk[clk_sys].selected))
            tight_loop_contents();
    // configured_freq[clk_sys] = (uint32_t)(((uint64_t) SYS_CLK_KHZ*KHZ << 8) / 1);
    clock_set_reported_hz(clk_sys, SYS_CLK_KHZ*KHZ);
}

void recover_fromDormant(uint scb_orig, uint clock0_orig, uint clock1_orig){
    
    rosc_write(&rosc_hw->ctrl, ROSC_CTRL_ENABLE_BITS);

    // Before we touch PLLs, switch sys and ref cleanly away from their aux sources.
    // hw_clear_bits(&clocks_hw->clk[clk_sys].ctrl, CLOCKS_CLK_SYS_CTRL_SRC_BITS);
    // while (clocks_hw->clk[clk_sys].selected != 0x1)
    //     tight_loop_contents();
    // hw_clear_bits(&clocks_hw->clk[clk_ref].ctrl, CLOCKS_CLK_REF_CTRL_SRC_BITS);
    // while (clocks_hw->clk[clk_ref].selected != 0x1)
    //     tight_loop_contents();

    /// pll_init[]
    pll_init(pll_sys, PLL_COMMON_REFDIV, PLL_SYS_VCO_FREQ_KHZ * KHZ, PLL_SYS_POSTDIV1, PLL_SYS_POSTDIV2);
    pll_init(pll_usb, PLL_COMMON_REFDIV, PLL_USB_VCO_FREQ_KHZ * KHZ, PLL_USB_POSTDIV1, PLL_USB_POSTDIV2);

    clock_configure(clk_ref,
                    CLOCKS_CLK_REF_CTRL_SRC_VALUE_XOSC_CLKSRC,
                    0, // No aux mux
                    XOSC_KHZ * KHZ,
                    XOSC_KHZ * KHZ);

    clock_configure(clk_sys, // FALLA
                    CLOCKS_CLK_SYS_CTRL_SRC_VALUE_CLKSRC_CLK_SYS_AUX,
                    0, // CLOCKS_CLK_SYS_CTRL_AUXSRC_VALUE_CLKSRC_PLL_SYS,
                    SYS_CLK_KHZ * KHZ,
                    SYS_CLK_KHZ * KHZ);
    // clock_sys_configure(); // LLamada a funcion implementada
    

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

    // reset procs back to default
    // scb_hw->scr = scb_orig;
    // clocks_hw->sleep_en0 = clock0_orig;
    // clocks_hw->sleep_en1 = clock1_orig;
    
    // clocks_init();
    stdio_init_all();
    stdio_uart_init();

    // Reconfigure uart with new clocks
    // setup_default_uart();

    return;
}

void measure_freqs(void) {
    uint f_pll_sys = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_PLL_SYS_CLKSRC_PRIMARY);
    uint f_pll_usb = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_PLL_USB_CLKSRC_PRIMARY);
    uint f_rosc = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_ROSC_CLKSRC);
    uint f_clk_sys = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_CLK_SYS);
    uint f_clk_peri = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_CLK_PERI);
    uint f_clk_usb = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_CLK_USB);
    uint f_clk_adc = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_CLK_ADC);
    uint f_clk_rtc = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_CLK_RTC);

    printf("\npll_sys  = %dkHz\n", f_pll_sys);
    printf("pll_usb  = %dkHz\n", f_pll_usb);
    printf("rosc     = %dkHz\n", f_rosc);
    printf("clk_sys  = %dkHz\n", f_clk_sys);
    printf("clk_peri = %dkHz\n", f_clk_peri);
    printf("clk_usb  = %dkHz\n", f_clk_usb);
    printf("clk_adc  = %dkHz\n", f_clk_adc);
    printf("clk_rtc  = %dkHz\n", f_clk_rtc);
    uart_default_tx_wait_blocking();
    // Can't measure clk_ref / xosc as it is the ref
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
    // gps_init(GPS_TX, GPS_RX, GPS_UART_ID, BAUD_RATE);

    // Dormant state
    //save values for later
    uint scb_orig = scb_hw->scr;
    uint clock0_orig = clocks_hw->sleep_en0;
    uint clock1_orig = clocks_hw->sleep_en1;
    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);

    measure_freqs();
    sleep_run_from_xosc(); // UART will be reconfigured by sleep_run_from_xosc
    measure_freqs();

    // sleep_goto_dormant_until_edge_high(BUTTON_GPIO); // Solo para flancos de subida
    // gpio_put(PICO_DEFAULT_LED_PIN,1);
    // sleep_goto_dormant_until_pin(BUTTON_GPIO, true, false);
    // gpio_put(PICO_DEFAULT_LED_PIN,0);
    // recover_from_sleep(scb_orig, clock0_orig, clock1_orig);
    
    while (1) {
        
        // printf("Command input: ");
        // fgets(cmd, sizeof(cmd), stdin);  // Read command from stdin (enable LF)

        // printf("Sending command\n");
        // uart_puts(UART_ID_LORA, cmd);
        // -------------------------------------------
        // while (uart_is_readable(LORA_UART_ID)) {
        //     char received_char = uart_getc(LORA_UART_ID);
        //     printf("%c", received_char);
        // }
        // -------------------------------------------

        if (!PENDING_EVENTS){
            if (!asleep)
            {
                asleep = true;
                gpio_put(PICO_DEFAULT_LED_PIN,1);
                sleep_goto_dormant_until_pin(BUTTON_GPIO, true, false);
                // gpio_put(PICO_DEFAULT_LED_PIN,0);

                // recover_from_sleep(scb_orig, clock0_orig, clock1_orig);
                recover_fromDormant(scb_orig, clock0_orig, clock1_orig);
                measure_freqs();
                
            }
        }

        if (EV_BUTTON){
            gpio_put(PICO_DEFAULT_LED_PIN,0);
            EV_BUTTON = 0;
            button_pressed = 1;
            // if (read_gps_coor(&latitude, &longitude)){
            if(1){
                latitude = 6.2152100;
                longitude = -75.5833950;
                printf("%f,%f\n", latitude, longitude);

                char message[64];
                int len = snprintf(message, sizeof(message),"%d, %f, %f", button_pressed, latitude, longitude);

                int i;
                char hex_payload[100];
                for (i = 0; message[i] != '\0'; i++){
                    sprintf(&hex_payload[i*2], "%02x", message[i]);
                }
                hex_payload[i*2] = '\0'; // Agrega el carácter nulo al final de la cadena

                lora_send(0, 2, len, hex_payload);

                while (uart_is_readable(LORA_UART_ID)) {
                    char received_char = uart_getc(LORA_UART_ID);
                    printf("%c", received_char);
                }
            }
            // gpio_put(PICO_DEFAULT_LED_PIN,0);
            asleep = false;

            // gpio_put(PICO_DEFAULT_LED_PIN,1);
            // sleep_goto_dormant_until_pin(BUTTON_GPIO, true, false);
            // gpio_put(PICO_DEFAULT_LED_PIN,0);

            // recover_from_sleep(scb_orig, clock0_orig, clock1_orig);
        }

        if (EV_OPEN){
            EV_OPEN = 0;
            button_pressed = 0;
            // To be implemented
        }
        
        
    }
    return 0;
}
