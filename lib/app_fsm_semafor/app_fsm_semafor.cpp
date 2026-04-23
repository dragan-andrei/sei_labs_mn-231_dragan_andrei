#include "app_fsm_semafor.h"
#include "dd_led.h"       // Modulul tău vechi
#include "dd_button.h"    // Modulul tău vechi
#include "ctrl_stdio.h"   // Modulul tău vechi pentru Serial/LCD

// Semafor FreeRTOS pentru a semnaliza cererea
static SemaphoreHandle_t g_north_request_sem;

// Task pentru citirea butonului (Cerere Nord)
void vTaskButtonMonitor(void *pvParameters) {
    (void)pvParameters;
    bool wasPressed = false;

    for (;;) {
        bool isPressed = dd_button_is_pressed(PIN_BTN_NORD);

        // Semnalăm cererea doar pe frontul de apăsare pentru a evita spam-ul.
        if (isPressed && !wasPressed) {
            // Trimite semnal către FSM
            xSemaphoreGive(g_north_request_sem);

            // Un mic delay pentru debouncing
            vTaskDelay(pdMS_TO_TICKS(BTN_DEBOUNCE_MS));
        }

        wasPressed = isPressed;
        vTaskDelay(pdMS_TO_TICKS(BTN_POLL_MS));
    }
}

// Task pentru Automatul Finit (FSM)
void vTaskTrafficLightFSM(void *pvParameters) {
    (void)pvParameters;
    TrafficLightState_t currentState = ST_EST_VERDE;

    for (;;) {
        switch (currentState) {
            case ST_EST_VERDE:
                dd_led_set(PIN_LED_EST_G, HIGH);
                dd_led_set(PIN_LED_EST_Y, LOW);
                dd_led_set(PIN_LED_EST_R, LOW);

                dd_led_set(PIN_LED_NORD_G, LOW);
                dd_led_set(PIN_LED_NORD_Y, LOW);
                dd_led_set(PIN_LED_NORD_R, HIGH);

                ctrl_stdio_printf("\n[%6lu] [TRAFIC]  AUTO(EST): (G) VERDE   ||  PIETON(NORD): (R) ROSU\n", millis());

                // Prioritate EST: rămâne verde până apare cerere Nord
                // sau expiră timeout-ul de siguranță.
                if (xSemaphoreTake(g_north_request_sem, pdMS_TO_TICKS(TRAFFIC_EST_FAILSAFE_TIMEOUT_MS)) == pdTRUE) {
                    ctrl_stdio_printf("[%6lu] [CERERE]  >>> Buton Pieton Apasat! Schimbam prioritatea...\n", millis());
                    // Mai ținem verdele pe Est 2 secunde inainte de a schimba, ca să nu fie brusc
                    vTaskDelay(pdMS_TO_TICKS(TRAFFIC_EST_REQUEST_HOLD_MS));
                } else {
                    ctrl_stdio_printf("[%6lu] [SISTEM]  ⏱️  Timeout siguranta expirat. Fortam NORD.\n", millis());
                }

                currentState = ST_EST_GALBEN;
                break;

            case ST_EST_GALBEN:
                dd_led_set(PIN_LED_EST_G, LOW);
                dd_led_set(PIN_LED_EST_Y, HIGH);
                ctrl_stdio_printf("[%6lu] [TRAFIC]  AUTO(EST): (Y) GALBEN  ||  PIETON(NORD): (R) ROSU\n", millis());

                vTaskDelay(pdMS_TO_TICKS(TRAFFIC_EST_GALBEN_MS));
                currentState = ST_ALL_RED_1;
                break;

            case ST_ALL_RED_1:
                dd_led_set(PIN_LED_EST_Y, LOW);
                dd_led_set(PIN_LED_EST_R, HIGH);
                ctrl_stdio_printf("[%6lu] [TRAFIC]  AUTO(EST): (R) ROSU    ||  PIETON(NORD): (R) ROSU  [SIGURANTA]\n", millis());

                vTaskDelay(pdMS_TO_TICKS(TRAFFIC_ALL_RED_MS));
                currentState = ST_NORD_VERDE;
                break;

            case ST_NORD_VERDE:
                dd_led_set(PIN_LED_NORD_R, LOW);
                dd_led_set(PIN_LED_NORD_G, HIGH);
                ctrl_stdio_printf("\n[%6lu] [TRAFIC]  AUTO(EST): (R) ROSU    ||  PIETON(NORD): (G) VERDE\n", millis());

                vTaskDelay(pdMS_TO_TICKS(TRAFFIC_NORD_VERDE_MS));
                currentState = ST_NORD_GALBEN;
                break;

            case ST_NORD_GALBEN:
                dd_led_set(PIN_LED_NORD_G, LOW);
                dd_led_set(PIN_LED_NORD_Y, HIGH);
                ctrl_stdio_printf("[%6lu] [TRAFIC]  AUTO(EST): (R) ROSU    ||  PIETON(NORD): (Y) GALBEN\n", millis());

                vTaskDelay(pdMS_TO_TICKS(TRAFFIC_NORD_GALBEN_MS));
                currentState = ST_ALL_RED_2;
                break;

            case ST_ALL_RED_2:
                dd_led_set(PIN_LED_NORD_Y, LOW);
                dd_led_set(PIN_LED_NORD_R, HIGH);
                ctrl_stdio_printf("[%6lu] [TRAFIC]  AUTO(EST): (R) ROSU    ||  PIETON(NORD): (R) ROSU  [SIGURANTA]\n", millis());

                vTaskDelay(pdMS_TO_TICKS(TRAFFIC_ALL_RED_MS));

                // Ne întoarcem la starea de bază
                currentState = ST_EST_VERDE;
                break;
        }
    }
}

void app_semafor_init() {
    // Creare semafor binar pentru sincronizare
    g_north_request_sem = xSemaphoreCreateBinary();

    if (g_north_request_sem == NULL) {
        ctrl_stdio_printf("[EROARE] Nu s-a putut crea semaforul pentru cerere Nord.\n");
        return;
    }

    // Creare task-uri FreeRTOS
    if (xTaskCreate(vTaskButtonMonitor, "BtnMonitor", FSM_SEMAFOR_TASK_STACK_SIZE, NULL, FSM_SEMAFOR_BTN_TASK_PRIORITY, NULL) != pdPASS) {
        ctrl_stdio_printf("[EROARE] Nu s-a putut crea task-ul BtnMonitor.\n");
        return;
    }

    if (xTaskCreate(vTaskTrafficLightFSM, "FSM_Semafor", FSM_SEMAFOR_TASK_STACK_SIZE, NULL, FSM_SEMAFOR_LOGIC_TASK_PRIORITY, NULL) != pdPASS) {
        ctrl_stdio_printf("[EROARE] Nu s-a putut crea task-ul FSM_Semafor.\n");
    }
}