#include "esp_attr.h"
#include "freertos/projdefs.h"
#include "hal/gpio_types.h"
#include "reent.h"
#include "stdlib.h"
#include "sys/types.h"
#include <driver/gpio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/task.h>
#include <stdbool.h>
#include <stdio.h>

#define PIN_SENSOR_EMERGENCIA GPIO_NUM_39
#define PIN_MOTOR GPIO_NUM_13

static volatile bool sistema_seguro = true;
static QueueHandle_t xQueueEmergencia = NULL;
static void IRAM_ATTR isr_emergencia_handler(void *arg) {
  sistema_seguro = false;
  int numro_emergencia = 911;
  xQueueSendFromISR(xQueueEmergencia, &numro_emergencia, NULL);
}
void tarea_motor(void *pvParameters) {
  gpio_config_t hw_conf = {.intr_type = GPIO_INTR_DISABLE,
                           .mode = GPIO_MODE_OUTPUT,
                           .pin_bit_mask = (1ULL << PIN_MOTOR),
                           .pull_down_en = 0,
                           .pull_up_en = 0};
  gpio_config(&hw_conf);
  while (true) {

    if (sistema_seguro) {
      printf("Motor Girando OK\n");
      gpio_set_level(PIN_MOTOR, 1);
      vTaskDelay(pdMS_TO_TICKS(1000));
      gpio_set_level(PIN_MOTOR, 0);
      vTaskDelay(pdMS_TO_TICKS(1000));
    } else {
      // BLoqueo de seguirdad se apaga el motor directamente
      gpio_set_level(PIN_MOTOR, 0);
      printf("Motor Detenido de Emergencia!!!!\n");
      int msg;
      if (xQueueReceive(xQueueEmergencia, &msg, portMAX_DELAY)) {
        printf("Alerta Recibida: Codigo de Error %d\n", msg);
      }
      sistema_seguro = true;
      printf("Iniciando SIstema");
      vTaskDelay(pdMS_TO_TICKS(2000));
    }
  }
}
void app_main() {

  xQueueEmergencia = xQueueCreate(5, sizeof(int));
  gpio_config_t io_conf = {.intr_type = GPIO_INTR_NEGEDGE,
                           .mode = GPIO_MODE_INPUT,
                           .pin_bit_mask = (1ULL << PIN_SENSOR_EMERGENCIA),
                           .pull_down_en = GPIO_PULLDOWN_DISABLE,
                           .pull_up_en = GPIO_PULLUP_DISABLE};
  gpio_config(&io_conf);
  gpio_install_isr_service(0);
  gpio_isr_handler_add(PIN_SENSOR_EMERGENCIA, isr_emergencia_handler, NULL);
  xTaskCreate(tarea_motor, "Tarea Motor", 4096, NULL, 5, NULL);
}
