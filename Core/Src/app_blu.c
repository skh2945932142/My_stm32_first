#include "app_blu.h"

#include "transport_uart.h"

void App_BLU_Init(void) {
}

void App_BLU_Task(void) {
}

void App_BLU_SendString(const char *str) {
  if (str != NULL) {
    Transport_SendLine(str);
  }
}
