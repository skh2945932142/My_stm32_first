/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    stm32f1xx_it.c
  * @brief   Interrupt Service Routines.
  ******************************************************************************
  */
/* USER CODE END Header */
#include "main.h"
#include "stm32f1xx_it.h"

/* ── 改动：外部变量改为 USART2 / DMA Ch6 Ch7 ── */
extern DMA_HandleTypeDef hdma_usart2_rx;
extern DMA_HandleTypeDef hdma_usart2_tx;
extern UART_HandleTypeDef huart2;

/* ============================================================
 * Cortex-M3 异常处理（不变）
 * ============================================================ */
void NMI_Handler(void)          { while (1) {} }
void HardFault_Handler(void)    { while (1) {} }
void MemManage_Handler(void)    { while (1) {} }
void BusFault_Handler(void)     { while (1) {} }
void UsageFault_Handler(void)   { while (1) {} }
void SVC_Handler(void)          {}
void DebugMon_Handler(void)     {}
void PendSV_Handler(void)       {}

void SysTick_Handler(void) {
  HAL_IncTick();
}

/* ============================================================
 * ── 改动：DMA 中断换成 Channel6/7（USART2 对应通道）──
 *
 *   USART2_TX → DMA1_Channel7
 *   USART2_RX → DMA1_Channel6
 * ============================================================ */
void DMA1_Channel7_IRQHandler(void) {
  HAL_DMA_IRQHandler(&hdma_usart2_tx);
}

void DMA1_Channel6_IRQHandler(void) {
  HAL_DMA_IRQHandler(&hdma_usart2_rx);
}

/* ── 改动：串口中断换成 USART2 ── */
void USART2_IRQHandler(void) {
  HAL_UART_IRQHandler(&huart2);
}
