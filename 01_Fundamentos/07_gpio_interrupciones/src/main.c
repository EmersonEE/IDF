#include "esp_attr.h"
#include "hal/gpio_types.h"
#include "stdbool.h"
#include <driver/gpio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/task.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#define BUTTON_GPIO GPIO_NUM_39
static QueueHandle_t gpio_evt_queue = NULL;
// IRAM_ATTR esta macro le dice al esp32 que guarde esta funcion en la memoria
// RAM intenra y no en el flash Esto es vital porque las interrupciones deben
// ejecutarse extramedamente rapido
static void IRAM_ATTR gpio_isr_handler(void *arg) {

  uint32_t gpio_num = (uint32_t)arg;
  xQueueSendFromISR(gpio_evt_queue, &gpio_num, NULL);
}
// xQueueReceive aqui es donde ocurre la magia. La tarea se queda dormida in
// consumir CPU gracias a portMAX_DELAY hasta que llegue algo a la cola
// Cuando la ISR envia un dato esta tarea se despierta, guarda el valor en
// io_num e imprime el mensaje
void button_task(void *pvParameters) {
  uint32_t io_num;
  while (true) {
    if (xQueueReceive(gpio_evt_queue, &io_num, portMAX_DELAY)) {
      printf("Interrupcion En GPIO: %" PRIu32 "\n", io_num);
    }
  }
}
void app_main() {
  gpio_config_t io_conf = {.pin_bit_mask = (1ULL << BUTTON_GPIO),
                           .mode = GPIO_MODE_INPUT,
                           .pull_up_en = GPIO_PULLUP_DISABLE,
                           .pull_down_en = GPIO_PULLDOWN_DISABLE,
                           .intr_type = GPIO_INTR_NEGEDGE};

  gpio_config(&io_conf);

  // Crea una cola para 10 elementos
  gpio_evt_queue = xQueueCreate(10, sizeof(uint32_t));

  xTaskCreate(button_task, "button_task", 2048, NULL, 10, NULL);
  // Instalar el servicio de interrupciones
  gpio_install_isr_service(0);
  // Vincular el pin con la funcion ISR
  gpio_isr_handler_add(BUTTON_GPIO, gpio_isr_handler, (void *)BUTTON_GPIO);
}
