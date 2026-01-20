/*

Devuelve el mínimo espacio libre que ha tenido el stack de la tarea desde que
empezó.

En otras palabras: te dice cuántas palabras quedaron sin usar en el peor
momento.

Si el valor es muy bajo (cercano a 0), significa que la tarea estuvo a punto de
desbordar su stack.

Si el valor es alto, significa que le sobra memoria y puedes reducir el tamaño
asignado.

*/
#include "soc/gpio_num.h"
#include <driver/gpio.h>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <stdbool.h>
#include <stdio.h>
static const char *TAG = "STACK";

void tarea1(void *pvParameters) {
  gpio_reset_pin(GPIO_NUM_13);
  gpio_set_direction(GPIO_NUM_13, GPIO_MODE_OUTPUT);

  while (true) {
    gpio_set_level(GPIO_NUM_13, 1);
    vTaskDelay(pdMS_TO_TICKS(1000));
    gpio_set_level(GPIO_NUM_13, 0);
    vTaskDelay(pdMS_TO_TICKS(1000));
    printf("Hola Mundo\n");
    UBaseType_t stackLibre = uxTaskGetStackHighWaterMark(NULL);
    ESP_LOGI(TAG, "Tarea 1 - Stack minimo libre: %u\n", stackLibre);
  }
}
void app_main() { xTaskCreate(tarea1, "Tarea 1", 2048, NULL, 1, NULL); }
