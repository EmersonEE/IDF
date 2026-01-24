/*
Un módulo de monitoreo de temperatura y humedad para un invernadero inteligente.
Aquí, usamos esp_timer en modo periódico para leer un sensor DHT22 (vía GPIO
one-wire) cada 2 segundos, procesar los datos y enviarlos a una cola FreeRTOS
para que otra tarea los publique via MQTT (sin entrar en detalles de MQTT aún,
para mantener el foco en el timer). Este patrón lo he usado en proyectos con
clientes en manufactura, donde la precisión temporal es clave para evitar drifts
en logs o alertas. Asumimos integración con FreeRTOS (como en ESP-IDF por
default).
*/
#include "esp_err.h"
#include "esp_system.h"
#include "freertos/projdefs.h"
#include "reent.h"
#include "string.h"
#include <driver/gpio.h>
#include <esp_log.h>
#include <esp_random.h>
#include <esp_timer.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/task.h>
#include <stdint.h>
#include <stdio.h>

// Tags para el loggin
//
static const char *TAG = "MONITORING_TIMER";
// Handlre
static esp_timer_handle_t sensor_timer = NULL;

// Cola FreeRTOS para enviar datos leidos
static QueueHandle_t data_queue = NULL;

// Estrucutra de datos del sensor
typedef struct {
  float temperature;
  float humidity;
  uint64_t timestamp;
} sensor_data_t;

// SImulacion de lectura de un sensor DHT22
static esp_err_t read_dht22_sensor(sensor_data_t *data) {
  // Simulacion valores aleatorios para demo
  data->temperature = 25.0 + (float)(esp_random() % 50) / 10.0;
  data->humidity = 60.0 + (float)(esp_random() % 200) / 10.0;
  data->timestamp = esp_timer_get_time(); // Timestamp en us
  return ESP_OK;
}
// Callback del timer (dispatch en task para seguridad)
static void timer_callback(void *arg) {
  sensor_data_t data;
  esp_err_t err = read_dht22_sensor(&data);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Error Leyendo sensor : %s", esp_err_to_name(err));
    return;
  }
  // Enviar a cola FreeRTOS (no Bloquea)

  if (xQueueSendFromISR((QueueHandle_t)arg, &data, NULL) != pdPASS) {
    printf("Cola llena\n");
  }

  ESP_LOGI(TAG, "Datos Leidos: Temp=%.1f°C, Hum=%.1f%%, TS=%llu",
           data.temperature, data.humidity, data.timestamp);
}
// Inicializacion del timer y recursos
static esp_err_t init_sensor_monitoring(void) {
  // Crear cola FreeRTOS (para decouplin)
  data_queue = xQueueCreate(10, sizeof(sensor_data_t));
  if (data_queue == NULL) {
    ESP_LOGE(TAG, "Error creando cola");
    return ESP_FAIL;
  }
  // Configuracion del Timer
  const esp_timer_create_args_t timer_args = {.callback = &timer_callback,
                                              .arg = (void *)data_queue,
                                              .dispatch_method = ESP_TIMER_TASK,
                                              .name = "sensor_periodic_timer",
                                              .skip_unhandled_events = true};
  esp_err_t err = esp_timer_create(&timer_args, &sensor_timer);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Error creando el timer: %s", esp_err_to_name(err));
    return err;
  }
  // Iniciar periodico: cada 2 segundos
  err = esp_timer_start_periodic(sensor_timer, 2000000ULL);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Error iniciando el timer: %s", esp_err_to_name(err));
    return err;
  }
  ESP_LOGI(TAG, "Monitore de sensor iniciado correctamente");
  return ESP_OK;
  // FUncion de ejemplo para una tarea FreeRTOS que procesa la cola
}
static void procces_data_task(void *arg) {
  sensor_data_t data;
  for (;;) {
    if (xQueueReceive(data_queue, &data, portMAX_DELAY) == pdPASS) {
      // Procesar enviar, loggear, etc.
      ESP_LOGD(TAG, "Procesando: Temp=%.1f,Hum=%.1f", data.temperature,
               data.humidity);
      // En real: mqtt_publish("/sensor/data",&data)
    }
  }
}
// Limpieza (llamar en shutdown o error)
static void deinit_sensor_monitoring(void) {
  if (sensor_timer != NULL) {
    esp_timer_stop(sensor_timer);
    esp_timer_delete(sensor_timer);
    sensor_timer = NULL;
  }
  if (data_queue != NULL) {
    vQueueDelete(data_queue);
    data_queue = NULL;
  }
  ESP_LOGI(TAG, "Monitore detenido");
}
void app_main() {

  ESP_LOGI(TAG, "Iniciando proyecto monitore IoT");

  // Iniciazar (inculye el timer)
  if (init_sensor_monitoring() != ESP_OK) {
    ESP_LOGE(TAG, "Fallo en inicializacion - reiniciando");
    esp_restart();
  }
  xTaskCreate(procces_data_task, "process_task", 4096, NULL, 5, NULL);
  // Ejemplo: Detner despues de 1 minuto (para demo)
  vTaskDelay(pdMS_TO_TICKS(60000));
  deinit_sensor_monitoring();
}
