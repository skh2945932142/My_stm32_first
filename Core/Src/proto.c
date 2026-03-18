#include "proto.h"

#include "module_adc.h"
#include "module_buzzer.h"
#include "module_motor.h"
#include "module_oled.h"
#include "module_relay.h"
#include "module_rgb.h"
#include "module_sensor.h"
#include "module_servo.h"
#include "module_sys.h"
#include "transport_uart.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef void (*ProtoModuleHandler)(const ProtoFrame *frame);

typedef struct {
  const char *name;
  bool enabled;
  const char *cap_payload;
  ProtoModuleHandler handler;
} ProtoModuleDef;

static const ProtoModuleDef s_modules[] = {
    {"SYS", true, NULL, Sys_Handle},
    {"OLED", true, "enabled=1;maxChars=31", Oled_Handle},
    {"RGB", true, "enabled=1;channels=3;pwm=0", Rgb_Handle},
    {"BUZZER", true, "enabled=1;music=0", Buzzer_Handle},
    {"RELAY", true, "enabled=1;channels=1", Relay_Handle},
    {"SERVO", true, "enabled=1;range=0-180", Servo_Handle},
    {"MOTOR", true, "enabled=1;driver=drv8833", Motor_Handle},
    {"SENSOR", true, "enabled=1;aht20=1;ultrasonic=1;keys=2;encoder=1",
        Sensor_Handle},
    {"ADC", true, "enabled=1;ntc=1;pot=1", Adc_Handle},
    {"GPIO", false, "enabled=0", NULL},
    {"WS2812", false, "enabled=0", NULL},
    {"RTC", false, "enabled=0", NULL},
};

static bool proto_copy_token(const char *start, const char *end, char *out,
    size_t out_size) {
  size_t len;

  if ((start == NULL) || (end == NULL) || (out == NULL) || (out_size == 0U)
      || (end < start)) {
    return false;
  }

  len = (size_t)(end - start);
  if (len >= out_size) {
    return false;
  }

  memcpy(out, start, len);
  out[len] = '\0';
  return true;
}

static bool proto_find_payload_value(const char *payload, const char *key,
    char *out, size_t out_size) {
  const char *segment;
  size_t key_len;

  if ((payload == NULL) || (key == NULL) || (out == NULL) || (out_size == 0U)) {
    return false;
  }

  key_len = strlen(key);
  segment = payload;

  while (*segment != '\0') {
    const char *segment_end = strchr(segment, ';');
    const char *equals;
    size_t value_len;

    if (segment_end == NULL) {
      segment_end = segment + strlen(segment);
    }

    equals = memchr(segment, '=', (size_t)(segment_end - segment));
    if ((equals != NULL) && ((size_t)(equals - segment) == key_len)
        && (strncmp(segment, key, key_len) == 0)) {
      value_len = (size_t)(segment_end - equals - 1);
      if (value_len >= out_size) {
        value_len = out_size - 1U;
      }
      memcpy(out, equals + 1, value_len);
      out[value_len] = '\0';
      return true;
    }

    if (*segment_end == '\0') {
      break;
    }
    segment = segment_end + 1;
  }

  return false;
}

static uint8_t proto_hex_value(char ch) {
  if ((ch >= '0') && (ch <= '9')) {
    return (uint8_t)(ch - '0');
  }
  if ((ch >= 'A') && (ch <= 'F')) {
    return (uint8_t)(10 + (ch - 'A'));
  }
  if ((ch >= 'a') && (ch <= 'f')) {
    return (uint8_t)(10 + (ch - 'a'));
  }
  return 0xFFU;
}

static const ProtoModuleDef* proto_find_module(const char *name) {
  size_t i;

  for (i = 0U; i < (sizeof(s_modules) / sizeof(s_modules[0])); i++) {
    if (strcmp(name, s_modules[i].name) == 0) {
      return &s_modules[i];
    }
  }

  return NULL;
}

bool Proto_ParseLine(char *line, ProtoFrame *out) {
  char id_buffer[16];
  char *sep1;
  char *sep2;
  char *sep3;
  char *sep4;
  long request_id;

  if ((line == NULL) || (out == NULL)) {
    return false;
  }

  sep1 = strchr(line, '|');
  if (sep1 == NULL) {
    return false;
  }

  sep2 = strchr(sep1 + 1, '|');
  if (sep2 == NULL) {
    return false;
  }

  sep3 = strchr(sep2 + 1, '|');
  if (sep3 == NULL) {
    return false;
  }

  sep4 = strchr(sep3 + 1, '|');
  if (sep4 == NULL) {
    return false;
  }

  if (!proto_copy_token(line, sep1, out->type, sizeof(out->type))) {
    return false;
  }
  if (!proto_copy_token(sep1 + 1, sep2, id_buffer, sizeof(id_buffer))) {
    return false;
  }
  if (!proto_copy_token(sep2 + 1, sep3, out->module, sizeof(out->module))) {
    return false;
  }
  if (!proto_copy_token(sep3 + 1, sep4, out->action, sizeof(out->action))) {
    return false;
  }

  strncpy(out->payload, sep4 + 1, sizeof(out->payload) - 1U);
  out->payload[sizeof(out->payload) - 1U] = '\0';

  request_id = strtol(id_buffer, NULL, 10);
  out->request_id = (int)request_id;
  return true;
}

void Proto_Poll(void) {
  char line[TRANSPORT_MAX_LINE_LEN];
  ProtoFrame frame;

  while (Transport_ReadLine(line, sizeof(line))) {
    if (!Proto_ParseLine(line, &frame)) {
      Proto_SendLog("warn", "bad frame");
      continue;
    }

    Proto_HandleFrame(&frame);
  }
}

void Proto_URLDecode(const char *input, char *output, size_t out_size) {
  size_t out_index = 0U;

  if ((input == NULL) || (output == NULL) || (out_size == 0U)) {
    return;
  }

  while ((*input != '\0') && (out_index < (out_size - 1U))) {
    if ((input[0] == '%') && (input[1] != '\0') && (input[2] != '\0')) {
      uint8_t high = proto_hex_value(input[1]);
      uint8_t low = proto_hex_value(input[2]);

      if ((high != 0xFFU) && (low != 0xFFU)) {
        output[out_index++] = (char)((high << 4) | low);
        input += 3;
        continue;
      }
    }

    output[out_index++] = (*input == '+') ? ' ' : *input;
    input++;
  }

  output[out_index] = '\0';
}

void Proto_HandleFrame(const ProtoFrame *frame) {
  const ProtoModuleDef *module;

  if (frame == NULL) {
    return;
  }

  if (strcmp(frame->type, "REQ") != 0) {
    return;
  }

  module = proto_find_module(frame->module);
  if (module == NULL) {
    Proto_SendAckErr(frame->request_id, "BAD_MODULE", "unknown module");
    return;
  }

  if ((!module->enabled) || (module->handler == NULL)) {
    Proto_SendAckErr(frame->request_id, "UNSUPPORTED", "module disabled");
    return;
  }

  module->handler(frame);
}

int Proto_PayloadGetInt(const char *payload, const char *key, int default_value) {
  char value[16];
  long parsed;

  if (!Proto_PayloadGetString(payload, key, value, sizeof(value))) {
    return default_value;
  }

  parsed = strtol(value, NULL, 10);
  return (int)parsed;
}

bool Proto_PayloadGetString(const char *payload, const char *key, char *out,
    size_t out_size) {
  return proto_find_payload_value(payload, key, out, out_size);
}

bool Proto_PayloadGetDecodedString(const char *payload, const char *key,
    char *out, size_t out_size) {
  char raw_value[128];

  if (!Proto_PayloadGetString(payload, key, raw_value, sizeof(raw_value))) {
    return false;
  }

  Proto_URLDecode(raw_value, out, out_size);
  return true;
}

void Proto_SendAllCapabilities(void) {
  size_t i;

  for (i = 0U; i < (sizeof(s_modules) / sizeof(s_modules[0])); i++) {
    if (s_modules[i].cap_payload != NULL) {
      Proto_SendCap(s_modules[i].name, s_modules[i].cap_payload);
    }
  }
}

void Proto_SendAckOk(int request_id) {
  char line[32];

  snprintf(line, sizeof(line), "ACK|%d|OK", request_id);
  Transport_SendLine(line);
}

void Proto_SendAckErr(int request_id, const char *code, const char *message) {
  char line[96];

  snprintf(line, sizeof(line), "ACK|%d|ERR|%s|%s", request_id, code, message);
  Transport_SendLine(line);
}

void Proto_SendCap(const char *module, const char *payload) {
  char line[96];

  snprintf(line, sizeof(line), "CAP|%s|%s", module, payload);
  Transport_SendLine(line);
}

void Proto_SendStat(const char *module, const char *payload) {
  char line[96];

  snprintf(line, sizeof(line), "STAT|%s|%s", module, payload);
  Transport_SendLine(line);
}

void Proto_SendEvt(const char *module, const char *action, const char *payload) {
  char line[96];

  snprintf(line, sizeof(line), "EVT|%s|%s|%s", module, action, payload);
  Transport_SendLine(line);
}

void Proto_SendLog(const char *level, const char *message) {
  char line[96];

  snprintf(line, sizeof(line), "LOG|%s|%s", level, message);
  Transport_SendLine(line);
}
