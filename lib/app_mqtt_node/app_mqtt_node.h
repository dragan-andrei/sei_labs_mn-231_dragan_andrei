#ifndef APP_MQTT_NODE_H
#define APP_MQTT_NODE_H

// ============================================================================
//  app_mqtt_node — Orchestratorul aplicației.
// ============================================================================
//  Pornește toate task-urile FreeRTOS:
//   - task_net       (prio 3): menține conexiunile WiFi + MQTT, face reconnect
//                              automat, rulează `dd_mqtt_loop()`.
//   - task_sensor    (prio 2): la fiecare SENSOR_SAMPLE_PERIOD_MS citește DHT22
//                              și publică un JSON pe TOPIC_TELEMETRY.
//   - task_diag      (prio 1): statistici periodice (uptime, ok, failed...).
//  Tot aici e configurat callback-ul MQTT care mapează mesajele din topic-ul
//  de LED pe acționarea actuatorului.
void app_mqtt_node_start();

#endif // APP_MQTT_NODE_H
