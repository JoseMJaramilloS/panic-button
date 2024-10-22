#include "alarm.h"

// Start on Wednesday 13th January 2021 11:20:00
datetime_t t = {
    .year  = 2020,
    .month = 01,
    .day   = 13,
    .dotw  = 3, // 0 is Sunday, so 3 is Wednesday
    .hour  = 11,
    .min   = 20,
    .sec   = 00
};

void alarm_init(void){
    // Start the RTC
    rtc_init();
    rtc_set_datetime(&t);
}

void alarm_period(int minutes, rtc_callback_t alarm_callback){
    t.min += minutes;  // Añadir el intervalo configurado en minutos
    if (t.min >= 60) {
        t.min -= 60;
        t.hour++;
        if (t.hour >= 24) {
            t.hour = 0;
            t.day++;
            // Manejo simple del cambio de día (no maneja el cambio de mes/año)
        }
    }

    rtc_set_alarm(&t, alarm_callback);  // Configurar la próxima alarma
}

void print_current_datetime(void) {
    datetime_t _t = {0};
    rtc_get_datetime(&_t);  // Obtener la fecha y hora actual
    char datetime_buf[256];
    char *datetime_str = &datetime_buf[0];
    datetime_to_str(datetime_str, sizeof(datetime_buf), &_t);  // Convertir la fecha y hora a una cadena
    safe_printf("Alarm Fired At %s\n", datetime_str);  // Imprimir el mensaje junto con la fecha y hora
    stdio_flush();  // Asegurar que la salida se imprima inmediatamente
}