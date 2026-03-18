#include "module_sensor.h"

#include "board_hw.h"
#include "device_state.h"

#include <stdio.h>
#include <string.h>

static BoardInputState s_last_inputs;
static uint32_t s_next_auto_tick;

static void sensor_format_fixed1(float value, char *out, size_t out_size) {
  long scaled;
  unsigned long fraction;

  if ((out == NULL) || (out_size == 0U)) {
    return;
  }

  scaled = (long)((value * 10.0f) + ((value >= 0.0f) ? 0.5f : -0.5f));
  fraction = (unsigned long)((scaled < 0) ? -scaled : scaled) % 10UL;

  if (scaled < 0) {
    snprintf(out, out_size, "-%ld.%lu", -((scaled / 10L)), fraction);
  } else {
    snprintf(out, out_size, "%ld.%lu", scaled / 10L, fraction);
  }
}

static void sensor_report_inputs_if_changed(void) {
  BoardInputState current;
  char line[64];

  BoardHw_GetInputState(&current);

  if ((current.key1_pressed == s_last_inputs.key1_pressed)
      && (current.key2_pressed == s_last_inputs.key2_pressed)
      && (current.encoder_button_pressed == s_last_inputs.encoder_button_pressed)
      && (current.encoder_position == s_last_inputs.encoder_position)) {
    return;
  }

  s_last_inputs = current;
  DeviceState_GetMutable()->key1_pressed = current.key1_pressed;
  DeviceState_GetMutable()->key2_pressed = current.key2_pressed;
  DeviceState_GetMutable()->encoder_button_pressed =
      current.encoder_button_pressed;
  DeviceState_GetMutable()->encoder_position = current.encoder_position;

  snprintf(line, sizeof(line), "key1=%u;key2=%u;button=%u;encoder=%d",
      current.key1_pressed, current.key2_pressed, current.encoder_button_pressed,
      current.encoder_position);
  Proto_SendEvt("SENSOR", "INPUT", line);
}

static void sensor_report_aht20(void) {
  DeviceState *state = DeviceState_GetMutable();
  char temp_text[16];
  char humidity_text[16];
  char line[64];

  if (BoardHw_ReadAht20(&state->temperature_c, &state->humidity_pct)) {
    sensor_format_fixed1(state->temperature_c, temp_text, sizeof(temp_text));
    sensor_format_fixed1(state->humidity_pct, humidity_text, sizeof(humidity_text));
    snprintf(line, sizeof(line), "temp=%s;humidity=%s", temp_text, humidity_text);
    Proto_SendStat("SENSOR", line);
  } else {
    Proto_SendLog("warn", "aht20 read failed");
  }
}

void Sensor_Handle(const ProtoFrame *frame) {
  DeviceState *state = DeviceState_GetMutable();
  char target[24];
  char temp_text[16];
  char humidity_text[16];
  char distance_text[16];
  char line[64];

  if (strcmp(frame->action, "READ") == 0) {
    if (!Proto_PayloadGetDecodedString(frame->payload, "target", target,
        sizeof(target))) {
      strncpy(target, "AHT20", sizeof(target) - 1U);
      target[sizeof(target) - 1U] = '\0';
    }

    if (strcmp(target, "AHT20") == 0) {
      if (BoardHw_ReadAht20(&state->temperature_c, &state->humidity_pct)) {
        Proto_SendAckOk(frame->request_id);
        sensor_format_fixed1(state->temperature_c, temp_text, sizeof(temp_text));
        sensor_format_fixed1(state->humidity_pct, humidity_text, sizeof(humidity_text));
        snprintf(line, sizeof(line), "temp=%s;humidity=%s", temp_text,
            humidity_text);
        Proto_SendStat("SENSOR", line);
      } else {
        Proto_SendAckErr(frame->request_id, "IO_ERROR", "aht20 read failed");
      }
      return;
    }

    if (strcmp(target, "ULTRASONIC") == 0) {
      if (BoardHw_ReadUltrasonic(&state->distance_cm)) {
        Proto_SendAckOk(frame->request_id);
        sensor_format_fixed1(state->distance_cm, distance_text,
            sizeof(distance_text));
        snprintf(line, sizeof(line), "distance=%s", distance_text);
        Proto_SendStat("SENSOR", line);
      } else {
        Proto_SendAckErr(frame->request_id, "IO_ERROR", "ultrasonic read failed");
      }
      return;
    }

    Proto_SendAckErr(frame->request_id, "BAD_PAYLOAD", "unknown sensor target");
    return;
  }

  if (strcmp(frame->action, "AUTO") == 0) {
    state->sensor_auto_enabled =
        (uint8_t)Proto_PayloadGetInt(frame->payload, "enabled", 0);
    state->sensor_interval_ms =
        (uint16_t)Proto_PayloadGetInt(frame->payload, "interval", 1000);
    if (state->sensor_interval_ms < 500U) {
      state->sensor_interval_ms = 500U;
    }
    s_next_auto_tick = HAL_GetTick() + state->sensor_interval_ms;
    Proto_SendAckOk(frame->request_id);
    snprintf(line, sizeof(line), "auto=%u;interval=%u", state->sensor_auto_enabled,
        state->sensor_interval_ms);
    Proto_SendStat("SENSOR", line);
    return;
  }

  Proto_SendAckErr(frame->request_id, "BAD_ACTION", "unknown sensor action");
}

void Sensor_Task(void) {
  DeviceState *state = DeviceState_GetMutable();

  sensor_report_inputs_if_changed();

  if (!state->sensor_auto_enabled) {
    return;
  }

  if ((int32_t)(HAL_GetTick() - s_next_auto_tick) < 0) {
    return;
  }

  s_next_auto_tick = HAL_GetTick() + state->sensor_interval_ms;
  sensor_report_aht20();
}
