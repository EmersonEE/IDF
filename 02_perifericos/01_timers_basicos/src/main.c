
#include "esp_timer.h"
#include "freertos/projdefs.h"
#include "hal/gpio_types.h"
#include "stdbool.h"
#include <driver/gpio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <stdio.h>
#define LED GPIO_NUM_12
void timer_callback(void *arg) { printf("Timer ejecutado\n"); }

void app_main(void) {
  gpio_config_t io_conf = {.intr_type = GPIO_INTR_DISABLE,
                           .mode = GPIO_MODE_OUTPUT,
                           .pin_bit_mask = (1ULL << LED),
                           .pull_down_en = 0,
                           .pull_up_en = 0};
  gpio_config(&io_conf);
  const esp_timer_create_args_t timer_args = {.callback = &timer_callback,
                                              .name = "mi_timer"};

  esp_timer_handle_t timer_handle;
  esp_timer_create(&timer_args, &timer_handle);

  esp_timer_start_periodic(timer_handle, 1000000);
  while (true) {
    gpio_set_level(LED, 1);
    vTaskDelay(pdMS_TO_TICKS(1000));
    // Timer periódico cada 1 segundo (1 000 000 us)
    gpio_set_level(LED, 0);
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}
