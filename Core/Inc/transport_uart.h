#ifndef __TRANSPORT_UART_H__
#define __TRANSPORT_UART_H__

#include "main.h"

#include <stdbool.h>
#include <stddef.h>

#define TRANSPORT_MAX_LINE_LEN 128U

void Transport_Init(UART_HandleTypeDef *huart);
HAL_StatusTypeDef Transport_Start(void);
void Transport_RxEventCallback(UART_HandleTypeDef *huart, uint16_t size);
bool Transport_ReadLine(char *out, size_t out_size);
void Transport_SendLine(const char *line);

#endif /* __TRANSPORT_UART_H__ */
