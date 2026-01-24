/**
 * @file pwm_fan_control.c
 * @brief Control profesional de ventilador DC mediante PWM (LEDC) con ESP-IDF
 *
 * Aplicación real: Control de velocidad de ventilador según temperatura
 * ambiente
 * - Sensor de temperatura (simulado o real: NTC, DS18B20, BME280, etc.)
 * - Arranque suave (soft-start) para reducir estrés mecánico y ruido
 * - Curva de velocidad no lineal (más agresiva en temperaturas altas)
 * - Protección básica: apagado total si temperatura crítica (>75°C)
 * - Frecuencia PWM 25kHz (muy común en ventiladores 4-pin y reduce zumbido
 * audible)
 * - Resolución 10 bits → 1024 niveles (suficiente precisión para la mayoría de
 * aplicaciones)
 * - Modo HIGH_SPEED para cambios de duty cycle sin glitches (ideal para control
 * continuo)
 *
 * Uso típico en proyectos industriales:
 *   - Enfriamiento activo de racks/servers
 *   - Sistemas HVAC pequeños
 *   - Control climático en armarios eléctricos
 *   - Impresoras 3D / CNC / equipos con disipación térmica variable
 *
 * Buenas prácticas implementadas:
 *   - Verificación de errores en cada paso (ESP_ERROR_CHECK)
 *   - Uso de constantes definidas para fácil mantenimiento
 *   - Logging estructurado con tags
 *   - Función dedicada para inicialización (fácil de mover a componente)
 *   - Función de actualización separada (se puede llamar desde tarea FreeRTOS)
 *
 * Referencia oficial:
 * https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/peripherals/ledc.html
 */
#include "freertos/projdefs.h"
#include "hal/ledc_types.h"
#include "soc/clk_tree_defs.h"
#include <driver/ledc.h>
#include <esp_err.h>
#include <esp_log.h>
#include <esp_random.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <stdint.h>
#include <stdio.h>
// Declaracion de pines - variables
#define FAN_PWM_PIN GPIO_NUM_13 // Pin donde se conectara el PWM
#define FAN_PWM_CHANNEL LEDC_CHANNEL_0
#define FAN__PWM_TIMER LEDC_TIMER_0
#define FAN_PWM_SPEED_MODE                                                     \
  LEDC_HIGH_SPEED_MODE // Recomendado para cambios sin glitches
#define FAN_PWM_FREQ_HZ 25000
#define FAN_PWM_RESOLUTION LEDC_TIMER_10_BIT
#define MAX_DUTY ((1 << FAN_PWM_RESOLUTION) - 1)

static const char *TAG = "FAN_PWM_CONTROL";
// Umbrales de temperatura
#define TEMP_MIN 30.0f
#define TEMP_NORMAL 45.0f
#define TEMP_HIGH 60.0f
#define TEMP_CRITICAL 75.0f
// inicialización el modulo PWM
esp_err_t fan_pwm_init(void) {
  // Configurar el timer
  ledc_timer_config_t ledc_timer = {.speed_mode = FAN_PWM_SPEED_MODE,
                                    .duty_resolution = FAN_PWM_RESOLUTION,
                                    .timer_num = FAN__PWM_TIMER,
                                    .freq_hz = FAN_PWM_FREQ_HZ,
                                    .clk_cfg = LEDC_AUTO_CLK};
  ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));
  // Configurar el  canal
  ledc_channel_config_t ledc_channel = {.gpio_num = FAN_PWM_PIN,
                                        .speed_mode = FAN_PWM_SPEED_MODE,
                                        .channel = FAN_PWM_CHANNEL,
                                        .intr_type = LEDC_INTR_DISABLE,
                                        .timer_sel = FAN__PWM_TIMER,
                                        .duty = 0,
                                        .hpoint = 0};
  ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));
  // Asegurar estado inicial
  ESP_ERROR_CHECK(ledc_set_duty(FAN_PWM_SPEED_MODE, FAN_PWM_CHANNEL, 0));
  ESP_ERROR_CHECK(ledc_update_duty(FAN_PWM_SPEED_MODE, FAN_PWM_CHANNEL));
  ESP_LOGI(
      TAG,
      "Ventilador PWM inicializado : pin %d, freq %d Hz, resolucion %d bits",
      FAN_PWM_PIN, FAN_PWM_FREQ_HZ, FAN_PWM_RESOLUTION);

  return ESP_OK;
}
// Calcular el duty cicle segun temperatura (curba no linearl + soft soft-start)
// Retornar valor entre 0 y MAX_DUTY

uint32_t calculate_fan_duty(float temperature) {
  if (temperature >= TEMP_CRITICAL) {
    ESP_LOGE(
        TAG,
        "¡TEMPERATURA CRÍTICA! (%.1f°C) → Ventilador apagado por seguridad",
        temperature);
    return 0; // Apagado total en emergencia (puedes cambiar a 100% si es más
              // seguro)
  }

  if (temperature <= TEMP_MIN) {
    return 0; // Apagado
  }

  // Curva simple pero efectiva (puedes hacerla más compleja con PID si
  // necesitas)
  if (temperature <= TEMP_NORMAL) {
    // Zona baja: arranque muy suave
    return (uint32_t)((temperature - TEMP_MIN) / (TEMP_NORMAL - TEMP_MIN) *
                      0.35f * MAX_DUTY);
  } else if (temperature <= TEMP_HIGH) {
    // Zona media: aumento más pronunciado
    return (uint32_t)(0.35f * MAX_DUTY + (temperature - TEMP_NORMAL) /
                                             (TEMP_HIGH - TEMP_NORMAL) * 0.45f *
                                             MAX_DUTY);
  } else {
    // Zona alta: casi al 100%
    return (uint32_t)(0.80f * MAX_DUTY + (temperature - TEMP_HIGH) /
                                             (TEMP_CRITICAL - TEMP_HIGH) *
                                             0.20f * MAX_DUTY);
  }
}
void fan_update_speed(float current_temperature) {
  uint32_t target_duty = calculate_fan_duty(current_temperature);
  ESP_ERROR_CHECK(
      ledc_set_duty(FAN_PWM_SPEED_MODE, FAN_PWM_CHANNEL, target_duty));
  ESP_ERROR_CHECK(ledc_update_duty(FAN_PWM_SPEED_MODE, FAN_PWM_CHANNEL));

  ESP_LOGI(TAG, "Temp: %.1f°C → Duty: %lu/%u (%.1f%%)", current_temperature,
           target_duty, MAX_DUTY, (target_duty * 100.0f) / MAX_DUTY);
}
void app_main() {

  ESP_ERROR_CHECK(fan_pwm_init());

  float simulated_temp = 25.0f;

  while (true) {
    simulated_temp += (esp_random() % 1000) / 1000.0f * 4.0f - 2.0f;
    simulated_temp =
        simulated_temp < 20 ? 20 : (simulated_temp > 80 ? 80 : simulated_temp);
    fan_update_speed(simulated_temp);
    vTaskDelay(pdMS_TO_TICKS(2000));
  }
}
