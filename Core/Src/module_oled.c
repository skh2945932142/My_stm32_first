#include "module_oled.h"

#include "device_state.h"
#include "font.h"
#include "oled.h"

#include <stdio.h>
#include <string.h>

static void oled_render_status(const char *title, const char *text) {
  const DeviceState *state = DeviceState_Get();

  OLED_SetColorMode(
      state->oled_inverted ? OLED_COLOR_REVERSED : OLED_COLOR_NORMAL);
  OLED_NewFrame();
  OLED_PrintASCIIString(0, 0, title, &afont8x6, OLED_COLOR_NORMAL);
  OLED_PrintASCIIString(0, 16, text, &afont8x6, OLED_COLOR_NORMAL);
  OLED_ShowFrame();
}

void Oled_ModuleInit(void) {
  DeviceState *state = DeviceState_GetMutable();

  state->oled_inverted = 0U;
  state->oled_text[0] = '\0';
  OLED_SetColorMode(OLED_COLOR_NORMAL);
  OLED_NewFrame();
  OLED_ShowFrame();
}

void Oled_ClearScreen(void) {
  DeviceState *state = DeviceState_GetMutable();

  state->oled_text[0] = '\0';
  OLED_SetColorMode(
      state->oled_inverted ? OLED_COLOR_REVERSED : OLED_COLOR_NORMAL);
  OLED_NewFrame();
  OLED_ShowFrame();
}

void Oled_Handle(const ProtoFrame *frame) {
  DeviceState *state = DeviceState_GetMutable();
  char text_buffer[sizeof(state->oled_text)];
  uint8_t enabled;
  char line[48];

  if (strcmp(frame->action, "TEXT") == 0) {
    if (!Proto_PayloadGetDecodedString(frame->payload, "text", text_buffer,
        sizeof(text_buffer))) {
      Proto_URLDecode(frame->payload, text_buffer, sizeof(text_buffer));
    }

    if (text_buffer[0] == '\0') {
      Oled_ClearScreen();
      Proto_SendAckOk(frame->request_id);
      Proto_SendStat("OLED", "mode=clear");
      return;
    }

    strncpy(state->oled_text, text_buffer, sizeof(state->oled_text) - 1U);
    state->oled_text[sizeof(state->oled_text) - 1U] = '\0';

    oled_render_status("STM32 WEB", state->oled_text);
    Proto_SendAckOk(frame->request_id);
    snprintf(line, sizeof(line), "mode=text;chars=%u",
        (unsigned int)strlen(state->oled_text));
    Proto_SendStat("OLED", line);
    return;
  }

  if (strcmp(frame->action, "CLEAR") == 0) {
    Oled_ClearScreen();
    Proto_SendAckOk(frame->request_id);
    Proto_SendStat("OLED", "mode=clear");
    return;
  }

  if (strcmp(frame->action, "INVERT") == 0) {
    enabled = (uint8_t)Proto_PayloadGetInt(frame->payload, "enabled",
        state->oled_inverted ? 0 : 1);
    state->oled_inverted = enabled ? 1U : 0U;

    if (state->oled_text[0] != '\0') {
      oled_render_status("STM32 WEB", state->oled_text);
    } else {
      Oled_ClearScreen();
    }

    Proto_SendAckOk(frame->request_id);
    snprintf(line, sizeof(line), "mode=invert;enabled=%u",
        (unsigned int)state->oled_inverted);
    Proto_SendStat("OLED", line);
    return;
  }

  if (strcmp(frame->action, "DEMO") == 0) {
    strncpy(state->oled_text, "OLED DEMO", sizeof(state->oled_text) - 1U);
    state->oled_text[sizeof(state->oled_text) - 1U] = '\0';

    oled_render_status("STM32 READY", state->oled_text);
    Proto_SendAckOk(frame->request_id);
    Proto_SendStat("OLED", "mode=demo");
    return;
  }

  Proto_SendAckErr(frame->request_id, "BAD_ACTION", "unknown oled action");
}
