/**
FreeRTOS es un sistema operativo en tiempo real RTOS diseñado para
micorontoladores Permite:
- Multitarea real
- Priorizacion
- Comunicacion entre tareas
- Temporizacion precisa
### Que es una tarea:
Es como un "mini programa" que:
- tiene su propia pila
- Tiene prioridad
- Puede ejecutarse en paralelo con otras
 Estructura basica de una tarea
void tarea(void *pvParameters)
{
    while (1) {
        // código
    }
}
*/

// Crear Tarea con xTaskCreate()
#include "freertos/projdefs.h"
#include "reent.h"
#include "stdbool.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <stdio.h>

// Tarea 1
void tarea1(void *pvParameters) {
  while (true) {
    printf("Tarea 1 \n ");
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}
void tarea2(void *pvParameters) {
  while (true) {
    printf("Tarea 2\n");
    vTaskDelay(pdMS_TO_TICKS(500));
  }
}
void app_main() {

  xTaskCreate(tarea1, "Tarea 1", 2048, NULL, 1, NULL);
  xTaskCreate(tarea2, "Tarea 2", 2048, NULL, 1, NULL);
}
// COmo se manda a llamar y sus parametros
/*
    xTaskCreate(
        tarea1,        // Función
        "Tarea 1",     // Nombre
        2048,          // Stack
        NULL,          // Parámetros
        1,             // Prioridad
        NULL            // Handle
    );
*/
