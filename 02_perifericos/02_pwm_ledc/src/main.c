#include "freertos/projdefs.h"
#include "hal/ledc_types.h"
#include "soc/clk_tree_defs.h"
#include "stdbool.h"
#include <driver/ledc.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <stdio.h>

#define LED_GPIO GPIO_NUM_12

void app_main() {
  // COnfigurar el timer
  ledc_timer_config_t ledc_timer = {.speed_mode = LEDC_LOW_SPEED_MODE,
                                    .timer_num = LEDC_TIMER_0,
                                    .duty_resolution = LEDC_TIMER_8_BIT,
                                    .freq_hz = 5000,
                                    .clk_cfg = LEDC_AUTO_CLK};
  ledc_timer_config(&ledc_timer);
  // COnfigurar el canal
  ledc_channel_config_t ledc_channel = {.speed_mode = LEDC_LOW_SPEED_MODE,
                                        .channel = LEDC_CHANNEL_0,
                                        .timer_sel = LEDC_TIMER_0,
                                        .intr_type = LEDC_INTR_DISABLE,
                                        .gpio_num = LED_GPIO,
                                        .duty = 0,
                                        .hpoint = 0};
  ledc_channel_config(&ledc_channel);

  // Cambiar el duty
  while (true) {
    for (int duty = 0; duty <= 255; duty += 5) {
      ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, duty);
      ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
      vTaskDelay(pdMS_TO_TICKS(20));
    }
  }
}
