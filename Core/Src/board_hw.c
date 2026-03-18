#include "board_hw.h"

#include "i2c.h"

#include <string.h>

#define AHT20_ADDRESS 0x70U
typedef struct {
  uint16_t servo_pulse_us;
  uint32_t servo_frame_start_us;
  uint32_t servo_pulse_end_us;
  uint8_t servo_pin_high;
  uint32_t buzzer_stop_tick;
  uint8_t buzzer_active;
  int16_t encoder_position;
  uint8_t last_encoder_ab;
} BoardHwState;

static BoardHwState s_board_hw;

static void boardhw_enable_dwt(void) {
  CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
  DWT->CYCCNT = 0U;
  DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

static uint32_t boardhw_micros(void) {
  return DWT->CYCCNT / (SystemCoreClock / 1000000U);
}

static void boardhw_delay_us(uint32_t delay_us) {
  uint32_t start = boardhw_micros();

  while ((boardhw_micros() - start) < delay_us) {
  }
}

static void boardhw_init_gpio(void) {
  GPIO_InitTypeDef gpio = {0};

  __HAL_RCC_AFIO_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_11, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, GPIO_PIN_RESET);

  gpio.Pin = GPIO_PIN_5;
  gpio.Mode = GPIO_MODE_OUTPUT_PP;
  gpio.Pull = GPIO_NOPULL;
  gpio.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &gpio);

  gpio.Pin = GPIO_PIN_8;
  gpio.Mode = GPIO_MODE_OUTPUT_PP;
  gpio.Pull = GPIO_NOPULL;
  gpio.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOB, &gpio);

  gpio.Pin = GPIO_PIN_9;
  gpio.Mode = GPIO_MODE_AF_PP;
  gpio.Pull = GPIO_NOPULL;
  gpio.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOB, &gpio);

  gpio.Pin = GPIO_PIN_0 | GPIO_PIN_1;
  gpio.Mode = GPIO_MODE_AF_PP;
  gpio.Pull = GPIO_NOPULL;
  gpio.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOA, &gpio);

  gpio.Pin = GPIO_PIN_11;
  gpio.Mode = GPIO_MODE_OUTPUT_PP;
  gpio.Pull = GPIO_NOPULL;
  gpio.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOA, &gpio);

  gpio.Pin = GPIO_PIN_10;
  gpio.Mode = GPIO_MODE_INPUT;
  gpio.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &gpio);

  gpio.Pin = GPIO_PIN_4 | GPIO_PIN_5;
  gpio.Mode = GPIO_MODE_ANALOG;
  HAL_GPIO_Init(GPIOA, &gpio);

  gpio.Pin = GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_15;
  gpio.Mode = GPIO_MODE_INPUT;
  gpio.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOB, &gpio);

  gpio.Pin = GPIO_PIN_8 | GPIO_PIN_9;
  gpio.Mode = GPIO_MODE_INPUT;
  gpio.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOA, &gpio);
}

static void boardhw_init_motor_pwm(void) {
  __HAL_RCC_TIM2_CLK_ENABLE();

  TIM2->PSC = 71U;
  TIM2->ARR = 999U;
  TIM2->CCR1 = 0U;
  TIM2->CCR2 = 0U;
  TIM2->CCMR1 =
      (6U << TIM_CCMR1_OC1M_Pos) | TIM_CCMR1_OC1PE |
      (6U << TIM_CCMR1_OC2M_Pos) | TIM_CCMR1_OC2PE;
  TIM2->CCER = TIM_CCER_CC1E | TIM_CCER_CC2E;
  TIM2->CR1 = TIM_CR1_ARPE | TIM_CR1_CEN;
  TIM2->EGR = TIM_EGR_UG;
}

static void boardhw_init_buzzer_pwm(void) {
  __HAL_RCC_TIM4_CLK_ENABLE();

  TIM4->PSC = 71U;
  TIM4->ARR = 999U;
  TIM4->CCR4 = 0U;
  TIM4->CCMR2 = (6U << TIM_CCMR2_OC4M_Pos) | TIM_CCMR2_OC4PE;
  TIM4->CCER = 0U;
  TIM4->CR1 = TIM_CR1_ARPE | TIM_CR1_CEN;
  TIM4->EGR = TIM_EGR_UG;
}

static void boardhw_init_adc(void) {
  __HAL_RCC_ADC1_CLK_ENABLE();

  RCC->CFGR &= ~RCC_CFGR_ADCPRE;
  RCC->CFGR |= RCC_CFGR_ADCPRE_DIV6;

  ADC1->CR1 = 0U;
  ADC1->CR2 = ADC_CR2_EXTTRIG | ADC_CR2_EXTSEL;
  ADC1->SMPR2 |= ADC_SMPR2_SMP4 | ADC_SMPR2_SMP5;
  ADC1->SQR1 = 0U;
  ADC1->SQR2 = 0U;
  ADC1->SQR3 = 0U;

  ADC1->CR2 |= ADC_CR2_ADON;
  boardhw_delay_us(10U);
  ADC1->CR2 |= ADC_CR2_RSTCAL;
  while ((ADC1->CR2 & ADC_CR2_RSTCAL) != 0U) {
  }
  ADC1->CR2 |= ADC_CR2_CAL;
  while ((ADC1->CR2 & ADC_CR2_CAL) != 0U) {
  }
}

static uint8_t boardhw_read_encoder_ab(void) {
  uint8_t a = (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_8) == GPIO_PIN_RESET) ? 1U : 0U;
  uint8_t b = (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_9) == GPIO_PIN_RESET) ? 1U : 0U;

  return (uint8_t)((a << 1) | b);
}

void BoardHw_Init(void) {
  memset(&s_board_hw, 0, sizeof(s_board_hw));
  s_board_hw.servo_pulse_us = 1500U;

  boardhw_enable_dwt();
  boardhw_init_gpio();
  boardhw_init_motor_pwm();
  boardhw_init_buzzer_pwm();
  boardhw_init_adc();

  s_board_hw.last_encoder_ab = boardhw_read_encoder_ab();
}

void BoardHw_Task(void) {
  uint32_t now_us = boardhw_micros();
  uint8_t current_ab = boardhw_read_encoder_ab();

  if ((now_us - s_board_hw.servo_frame_start_us) >= 20000U) {
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, GPIO_PIN_SET);
    s_board_hw.servo_frame_start_us = now_us;
    s_board_hw.servo_pulse_end_us = now_us + s_board_hw.servo_pulse_us;
    s_board_hw.servo_pin_high = 1U;
  }

  if (s_board_hw.servo_pin_high && (now_us >= s_board_hw.servo_pulse_end_us)) {
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, GPIO_PIN_RESET);
    s_board_hw.servo_pin_high = 0U;
  }

  if (s_board_hw.buzzer_active
      && ((int32_t)(HAL_GetTick() - s_board_hw.buzzer_stop_tick) >= 0)) {
    BoardHw_BuzzerStop();
  }

  if (current_ab != s_board_hw.last_encoder_ab) {
    uint8_t previous = s_board_hw.last_encoder_ab;

    if (((previous == 0U) && (current_ab == 1U))
        || ((previous == 1U) && (current_ab == 3U))
        || ((previous == 3U) && (current_ab == 2U))
        || ((previous == 2U) && (current_ab == 0U))) {
      s_board_hw.encoder_position++;
    } else if (((previous == 0U) && (current_ab == 2U))
        || ((previous == 2U) && (current_ab == 3U))
        || ((previous == 3U) && (current_ab == 1U))
        || ((previous == 1U) && (current_ab == 0U))) {
      s_board_hw.encoder_position--;
    }

    s_board_hw.last_encoder_ab = current_ab;
  }
}

void BoardHw_SetRelay(uint8_t enabled) {
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, enabled ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

uint8_t BoardHw_GetRelay(void) {
  return (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_5) == GPIO_PIN_SET) ? 1U : 0U;
}

void BoardHw_BuzzerBeep(uint16_t freq_hz, uint16_t duration_ms) {
  uint32_t arr;

  if (freq_hz < 50U) {
    freq_hz = 50U;
  }

  arr = 1000000UL / freq_hz;
  if (arr == 0U) {
    arr = 1U;
  }

  TIM4->ARR = (uint16_t)(arr - 1U);
  TIM4->CCR4 = (uint16_t)((arr * 20U) / 100U);
  TIM4->EGR = TIM_EGR_UG;
  TIM4->CCER |= TIM_CCER_CC4E;

  s_board_hw.buzzer_active = 1U;
  s_board_hw.buzzer_stop_tick = HAL_GetTick() + duration_ms;
}

void BoardHw_BuzzerStop(void) {
  TIM4->CCER &= ~TIM_CCER_CC4E;
  TIM4->CCR4 = 0U;
  s_board_hw.buzzer_active = 0U;
}

uint8_t BoardHw_IsBuzzerActive(void) {
  return s_board_hw.buzzer_active;
}

void BoardHw_SetServoAngle(uint8_t angle) {
  if (angle > 180U) {
    angle = 180U;
  }

  s_board_hw.servo_pulse_us = (uint16_t)(500U + ((uint32_t)angle * 2000U) / 180U);
}

void BoardHw_SetMotorSpeed(uint8_t speed_percent) {
  uint32_t duty;

  if (speed_percent > 100U) {
    speed_percent = 100U;
  }

  duty = ((uint32_t)(TIM2->ARR + 1U) * speed_percent) / 100U;
  TIM2->CCR1 = (uint16_t)duty;
  TIM2->CCR2 = 0U;
}

void BoardHw_StopMotor(void) {
  TIM2->CCR1 = 0U;
  TIM2->CCR2 = 0U;
}

bool BoardHw_ReadAht20(float *temperature_c, float *humidity_pct) {
  uint8_t trigger_cmd[3] = {0xACU, 0x33U, 0x00U};
  uint8_t read_buffer[7];
  uint32_t raw_humidity;
  uint32_t raw_temperature;

  if ((temperature_c == NULL) || (humidity_pct == NULL)) {
    return false;
  }

  if (HAL_I2C_Master_Transmit(&hi2c1, AHT20_ADDRESS, trigger_cmd,
      sizeof(trigger_cmd), 100U) != HAL_OK) {
    return false;
  }

  HAL_Delay(90U);

  if (HAL_I2C_Master_Receive(&hi2c1, AHT20_ADDRESS, read_buffer,
      sizeof(read_buffer), 100U) != HAL_OK) {
    return false;
  }

  if ((read_buffer[0] & 0x80U) != 0U) {
    return false;
  }

  raw_humidity = ((uint32_t)read_buffer[1] << 12)
      | ((uint32_t)read_buffer[2] << 4)
      | ((uint32_t)read_buffer[3] >> 4);
  raw_temperature = (((uint32_t)read_buffer[3] & 0x0FU) << 16)
      | ((uint32_t)read_buffer[4] << 8)
      | read_buffer[5];

  *humidity_pct = ((float)raw_humidity * 100.0f) / 1048576.0f;
  *temperature_c = ((float)raw_temperature * 200.0f) / 1048576.0f - 50.0f;
  return true;
}

bool BoardHw_ReadUltrasonic(float *distance_cm) {
  uint32_t timeout_start;
  uint32_t pulse_start;
  uint32_t pulse_width;

  if (distance_cm == NULL) {
    return false;
  }

  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_11, GPIO_PIN_RESET);
  boardhw_delay_us(2U);
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_11, GPIO_PIN_SET);
  boardhw_delay_us(12U);
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_11, GPIO_PIN_RESET);

  timeout_start = boardhw_micros();
  while (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_10) == GPIO_PIN_RESET) {
    if ((boardhw_micros() - timeout_start) > 30000U) {
      return false;
    }
  }

  pulse_start = boardhw_micros();
  while (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_10) == GPIO_PIN_SET) {
    if ((boardhw_micros() - pulse_start) > 30000U) {
      return false;
    }
  }

  pulse_width = boardhw_micros() - pulse_start;
  *distance_cm = (float)pulse_width / 58.0f;
  return true;
}

uint16_t BoardHw_ReadAdcChannel(uint8_t channel) {
  ADC1->SQR3 = channel;
  ADC1->SR = 0U;
  ADC1->CR2 |= ADC_CR2_ADON;
  ADC1->CR2 |= ADC_CR2_SWSTART;
  while ((ADC1->SR & ADC_SR_EOC) == 0U) {
  }
  return (uint16_t)ADC1->DR;
}

void BoardHw_GetInputState(BoardInputState *state) {
  if (state == NULL) {
    return;
  }

  state->key1_pressed =
      (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_12) == GPIO_PIN_RESET) ? 1U : 0U;
  state->key2_pressed =
      (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_13) == GPIO_PIN_RESET) ? 1U : 0U;
  state->encoder_button_pressed =
      (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_15) == GPIO_PIN_RESET) ? 1U : 0U;
  state->encoder_position = s_board_hw.encoder_position;
}
