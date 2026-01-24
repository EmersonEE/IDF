#include "hal/gpio_types.h"
#include <driver/gpio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <stdio.h>

#define BOTON GPIO_NUM_36
void app_main() {

  gpio_config_t gpio_conf = {.pin_bit_mask = (1ULL << BOTON),
                             .intr_type = GPIO_INTR_DISABLE,
                             .mode = GPIO_MODE_INPUT,
                             .pull_down_en = GPIO_PULLDOWN_DISABLE,
                             .pull_up_en = GPIO_PULLUP_DISABLE};

  gpio_config(&gpio_conf);

  while (true) {
    int boton = gpio_get_level(BOTON);
    if (boton == 0) {
      printf("Boton Presionado\n");
    } else {
      printf("Boton Soltado \n");
    }
  }
}
