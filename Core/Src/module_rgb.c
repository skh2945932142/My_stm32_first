#include "module_rgb.h"

#include "device_state.h"
#include "main.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static uint8_t rgb_clamp(int value) {
  if (value < 0) {
    return 0U;
  }
  if (value > 255) {
    return 255U;
  }
  return (uint8_t)value;
}

/* ── BUG FIX: RGB LED 共阳接法逻辑修正 ────────────────────────────────
 * 电路图：MHP5050RGBDT 共阳极接 3.3V，阴极通过限流电阻接 GPIO。
 *   GPIO 低电平 (RESET) → 电流流通 → LED 亮
 *   GPIO 高电平 (SET)   → 无电流   → LED 灭
 *
 * 原代码：r_on ? GPIO_PIN_SET : GPIO_PIN_RESET   ← 高电平=亮，逻辑反了！
 * 修正后：r_on ? GPIO_PIN_RESET : GPIO_PIN_SET   ← 低电平=亮，正确。
 * ─────────────────────────────────────────────────────────────────── */
static void rgb_apply_outputs(uint8_t r_on, uint8_t g_on, uint8_t b_on) {
  /* 红色：PB0  绿色：PA7  蓝色：PA6  —— 共阳，低有效 */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, r_on ? GPIO_PIN_RESET : GPIO_PIN_SET);
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7, g_on ? GPIO_PIN_RESET : GPIO_PIN_SET);
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, b_on ? GPIO_PIN_RESET : GPIO_PIN_SET);
}

static void rgb_report_static_state(void) {
  char line[48];
  const DeviceState *state = DeviceState_Get();

  snprintf(line, sizeof(line), "r=%u;g=%u;b=%u", state->rgb_r, state->rgb_g,
      state->rgb_b);
  Proto_SendStat("RGB", line);
}

void Rgb_Set(uint8_t r, uint8_t g, uint8_t b) {
  DeviceState *state = DeviceState_GetMutable();

  state->rgb_r = r;
  state->rgb_g = g;
  state->rgb_b = b;
  state->rgb_mode = ((r == 0U) && (g == 0U) && (b == 0U)) ? RGB_MODE_OFF
                                                           : RGB_MODE_STATIC;

  rgb_apply_outputs(r > 0U, g > 0U, b > 0U);
}

void Rgb_Off(void) {
  Rgb_Set(0U, 0U, 0U);
}

void Rgb_Handle(const ProtoFrame *frame) {
  DeviceState *state = DeviceState_GetMutable();
  char value_buffer[16];
  bool has_r;
  bool has_g;
  bool has_b;
  uint8_t r;
  uint8_t g;
  uint8_t b;
  int enabled;
  int period_ms;
  char line[72];

  if (strcmp(frame->action, "SET") == 0) {
    r = rgb_clamp(Proto_PayloadGetInt(frame->payload, "r", 0));
    g = rgb_clamp(Proto_PayloadGetInt(frame->payload, "g", 0));
    b = rgb_clamp(Proto_PayloadGetInt(frame->payload, "b", 0));

    Rgb_Set(r, g, b);
    Proto_SendAckOk(frame->request_id);
    rgb_report_static_state();
    return;
  }

  if (strcmp(frame->action, "OFF") == 0) {
    Rgb_Off();
    Proto_SendAckOk(frame->request_id);
    Proto_SendStat("RGB", "r=0;g=0;b=0");
    return;
  }

  if (strcmp(frame->action, "BREATH") == 0) {
    enabled = Proto_PayloadGetInt(frame->payload, "enabled", 1);
    if (enabled == 0) {
      Rgb_Off();
      Proto_SendAckOk(frame->request_id);
      Proto_SendStat("RGB", "r=0;g=0;b=0");
      return;
    }

    has_r = Proto_PayloadGetString(frame->payload, "r", value_buffer,
        sizeof(value_buffer));
    r = has_r ? rgb_clamp((int)strtol(value_buffer, NULL, 10)) : state->rgb_r;

    has_g = Proto_PayloadGetString(frame->payload, "g", value_buffer,
        sizeof(value_buffer));
    g = has_g ? rgb_clamp((int)strtol(value_buffer, NULL, 10)) : state->rgb_g;

    has_b = Proto_PayloadGetString(frame->payload, "b", value_buffer,
        sizeof(value_buffer));
    b = has_b ? rgb_clamp((int)strtol(value_buffer, NULL, 10)) : state->rgb_b;

    if ((!has_r) && (!has_g) && (!has_b) && (r == 0U) && (g == 0U)
        && (b == 0U)) {
      r = 255U;
      g = 255U;
      b = 255U;
    }

    period_ms = Proto_PayloadGetInt(frame->payload, "period",
        state->rgb_breath_period_ms);

    state->rgb_r = r;
    state->rgb_g = g;
    state->rgb_b = b;
    state->rgb_mode = RGB_MODE_BREATH;
    state->rgb_breath_period_ms = (uint16_t)((period_ms < 400) ? 400 : period_ms);

    Proto_SendAckOk(frame->request_id);
    snprintf(line, sizeof(line), "mode=breath;r=%u;g=%u;b=%u", r, g, b);
    Proto_SendStat("RGB", line);
    return;
  }

  Proto_SendAckErr(frame->request_id, "BAD_ACTION", "unknown rgb action");
}

void Rgb_Task(void) {
  static uint32_t last_tick = 0U;
  DeviceState *state = DeviceState_GetMutable();
  uint32_t now;
  uint32_t period;
  uint32_t phase;
  uint32_t half_period;
  uint8_t envelope;
  uint8_t pwm_slot;
  uint8_t r_on;
  uint8_t g_on;
  uint8_t b_on;

  if (state->rgb_mode != RGB_MODE_BREATH) {
    return;
  }

  now = HAL_GetTick();
  if (now == last_tick) {
    return;
  }
  last_tick = now;

  period = state->rgb_breath_period_ms;
  if (period < 400U) {
    period = 400U;
  }

  phase = now % period;
  half_period = period / 2U;
  if (half_period == 0U) {
    half_period = 1U;
  }

  if (phase < half_period) {
    envelope = (uint8_t)((phase * 255U) / half_period);
  } else {
    envelope = (uint8_t)(((period - phase) * 255U) / (period - half_period));
  }

  pwm_slot = (uint8_t)(now & 0x0FU);
  r_on = (((uint16_t)state->rgb_r * envelope) / 255U) > (uint16_t)(pwm_slot * 16U);
  g_on = (((uint16_t)state->rgb_g * envelope) / 255U) > (uint16_t)(pwm_slot * 16U);
  b_on = (((uint16_t)state->rgb_b * envelope) / 255U) > (uint16_t)(pwm_slot * 16U);

  rgb_apply_outputs(r_on, g_on, b_on);
}
