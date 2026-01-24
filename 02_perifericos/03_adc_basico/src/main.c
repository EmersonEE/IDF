// Aprendieondo ADC
#include "driver/adc_types_legacy.h"
#include "freertos/projdefs.h"
#include "hal/adc_types.h"
#include "stdbool.h"
#include "stdio.h"
#include <driver/adc.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <stdio.h>

#define ADC_CHANNEL ADC1_CHANNEL_4

void app_main() {
  adc1_config_width(ADC_WIDTH_BIT_12);
  // COnfigurar la atenuacion
  adc1_config_channel_atten(ADC_CHANNEL, ADC_ATTEN_DB_12);

  while (true) {
    int adc_value = adc1_get_raw(ADC_CHANNEL);
    printf("ADC raw: %d\n", adc_value);
    vTaskDelay(pdMS_TO_TICKS(400));
  }
}
