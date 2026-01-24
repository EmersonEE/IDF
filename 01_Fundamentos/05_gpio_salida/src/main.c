#include "freertos/projdefs.h"
#include "hal/gpio_types.h"
#include "stdbool.h"
#include <driver/gpio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <stdio.h>

#define LED_GPIO GPIO_NUM_2
void app_main() {

  gpio_config_t gpio_conf = {.pin_bit_mask = (1ULL << LED_GPIO),
                             .mode = GPIO_MODE_OUTPUT,
                             .pull_down_en = GPIO_PULLDOWN_DISABLE,
                             .pull_up_en = GPIO_PULLUP_DISABLE,
                             .intr_type = GPIO_INTR_DISABLE};
  gpio_config(&gpio_conf);

  while (true) {
    gpio_set_level(LED_GPIO, 1);
    vTaskDelay(pdMS_TO_TICKS(1000));
    gpio_set_level(LED_GPIO, 0);
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}
