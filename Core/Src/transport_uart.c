#include "transport_uart.h"

#include <string.h>

#define TRANSPORT_RX_DMA_SIZE 64U
#define TRANSPORT_QUEUE_DEPTH 4U

static UART_HandleTypeDef *s_uart;
static uint8_t s_rx_dma_buffer[TRANSPORT_RX_DMA_SIZE];
static char s_line_accumulator[TRANSPORT_MAX_LINE_LEN];
static volatile uint16_t s_line_length;
static char s_line_queue[TRANSPORT_QUEUE_DEPTH][TRANSPORT_MAX_LINE_LEN];
static volatile uint8_t s_queue_head;
static volatile uint8_t s_queue_tail;
static volatile uint8_t s_queue_count;
static volatile bool s_drop_until_newline;

static void transport_queue_line(void) {
  uint16_t copy_len;

  if (s_line_length == 0U) {
    return;
  }

  copy_len = s_line_length;
  if (copy_len > (TRANSPORT_MAX_LINE_LEN - 1U)) {
    copy_len = TRANSPORT_MAX_LINE_LEN - 1U;
  }

  if (s_queue_count < TRANSPORT_QUEUE_DEPTH) {
    memcpy(s_line_queue[s_queue_tail], s_line_accumulator, copy_len);
    s_line_queue[s_queue_tail][copy_len] = '\0';
    s_queue_tail = (uint8_t)((s_queue_tail + 1U) % TRANSPORT_QUEUE_DEPTH);
    s_queue_count++;
  }

  s_line_length = 0U;
}

static void transport_feed_byte(uint8_t ch) {
  if (s_drop_until_newline) {
    if (ch == '\n') {
      s_drop_until_newline = false;
      s_line_length = 0U;
    }
    return;
  }

  if (ch == '\r') {
    return;
  }

  if (ch == '\n') {
    transport_queue_line();
    return;
  }

  if (s_line_length < (TRANSPORT_MAX_LINE_LEN - 1U)) {
    s_line_accumulator[s_line_length++] = (char)ch;
  } else {
    s_line_length = 0U;
    s_drop_until_newline = true;
  }
}

void Transport_Init(UART_HandleTypeDef *huart) {
  s_uart = huart;
  s_line_length = 0U;
  s_queue_head = 0U;
  s_queue_tail = 0U;
  s_queue_count = 0U;
  s_drop_until_newline = false;
  memset(s_rx_dma_buffer, 0, sizeof(s_rx_dma_buffer));
  memset(s_line_accumulator, 0, sizeof(s_line_accumulator));
  memset(s_line_queue, 0, sizeof(s_line_queue));
}

HAL_StatusTypeDef Transport_Start(void) {
  HAL_StatusTypeDef status;

  if (s_uart == NULL) {
    return HAL_ERROR;
  }

  status = HAL_UARTEx_ReceiveToIdle_DMA(s_uart, s_rx_dma_buffer,
      sizeof(s_rx_dma_buffer));
  if ((status == HAL_OK) && (s_uart->hdmarx != NULL)) {
    __HAL_DMA_DISABLE_IT(s_uart->hdmarx, DMA_IT_HT);
  }

  return status;
}

void Transport_RxEventCallback(UART_HandleTypeDef *huart, uint16_t size) {
  uint16_t i;

  if ((huart != s_uart) || (huart == NULL)) {
    return;
  }

  for (i = 0U; i < size; i++) {
    transport_feed_byte(s_rx_dma_buffer[i]);
  }

  (void)Transport_Start();
}

bool Transport_ReadLine(char *out, size_t out_size) {
  bool has_line = false;

  if ((out == NULL) || (out_size == 0U)) {
    return false;
  }

  __disable_irq();
  if (s_queue_count > 0U) {
    strncpy(out, s_line_queue[s_queue_head], out_size - 1U);
    out[out_size - 1U] = '\0';
    s_queue_head = (uint8_t)((s_queue_head + 1U) % TRANSPORT_QUEUE_DEPTH);
    s_queue_count--;
    has_line = true;
  }
  __enable_irq();

  return has_line;
}

void Transport_SendLine(const char *line) {
  size_t line_len;
  char tx_buffer[TRANSPORT_MAX_LINE_LEN + 2U];

  if ((s_uart == NULL) || (line == NULL)) {
    return;
  }

  line_len = strlen(line);
  if (line_len > TRANSPORT_MAX_LINE_LEN) {
    line_len = TRANSPORT_MAX_LINE_LEN;
  }

  memcpy(tx_buffer, line, line_len);
  tx_buffer[line_len++] = '\n';

  (void)HAL_UART_Transmit(s_uart, (uint8_t*)tx_buffer, (uint16_t)line_len, 100U);
}
