#include <Arduino.h>
#include "configs.h"
#include "dd_led.h"
#include "dd_button.h"
#include "ctrl_stdio.h"
#include "app_fsm_semafor.h"

namespace {

constexpr uint8_t kTrafficLedPins[] = {
    PIN_LED_EST_R,
    PIN_LED_EST_Y,
    PIN_LED_EST_G,
    PIN_LED_NORD_R,
    PIN_LED_NORD_Y,
    PIN_LED_NORD_G
};

void initTrafficLeds() {
    for (uint8_t pin : kTrafficLedPins) {
        dd_led_init(pin);
    }
}

void printBootBanner() {
    ctrl_stdio_print_text("\n========================================\n");
    ctrl_stdio_print_text("   SISTEM SEMAFOR INTELIGENT - ESP32\n");
    ctrl_stdio_print_text("Directii active:\n");
    ctrl_stdio_print_text("  - EST-VEST:  AUTO (Principal)\n");
    ctrl_stdio_print_text("  - NORD-SUD:  PIETONI (Secundar cu Buton)\n");
    ctrl_stdio_print_text("Control: FSM + FreeRTOS\n");
    ctrl_stdio_print_text("========================================\n");
}

} // namespace

void setup() {
    ctrl_stdio_init(SERIAL_BAUDRATE);

    printBootBanner();
    initTrafficLeds();
    dd_button_init(PIN_BTN_NORD);
    app_semafor_init();

    ctrl_stdio_print_text("Sistem pornit: asteptam pietonii (NORD) sa apese butonul.\n");
}

void loop() {
    // In FreeRTOS pe ESP32, logica este rulata in task-uri.
    vTaskDelete(NULL);
}

