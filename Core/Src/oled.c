/**
 * @file oled.c
 * @brief 波特律动OLED驱动(CH1116) — 已修正 OLED_DisPlay_On 命令错误
 */
#include "oled.h"
#include "i2c.h"
#include <math.h>
#include <stdlib.h>

/* OLED I2C 地址（8-bit 写地址） */
#define OLED_ADDRESS 0x7A

#define OLED_PAGE   8
#define OLED_ROW    (8 * OLED_PAGE)
#define OLED_COLUMN 128

uint8_t OLED_GRAM[OLED_PAGE][OLED_COLUMN];

/* ========================== 底层通信 ========================== */

void OLED_Send(uint8_t *data, uint8_t len) {
  HAL_I2C_Master_Transmit(&hi2c1, OLED_ADDRESS, data, len, HAL_MAX_DELAY);
}

void OLED_SendCmd(uint8_t cmd) {
  static uint8_t sendBuffer[2] = {0x00, 0x00};
  sendBuffer[1] = cmd;
  OLED_Send(sendBuffer, 2);
}

/* ========================== OLED 驱动 ========================= */

void OLED_Init() {
  OLED_SendCmd(0xAE); /* 关闭显示 */

  OLED_SendCmd(0x02); /* 列起始地址低4位 = 2（CH1116 偏移） */
  OLED_SendCmd(0x10); /* 列起始地址高4位 = 0 */

  OLED_SendCmd(0x40); /* 起始行 = 0 */
  OLED_SendCmd(0xB0); /* 页地址 = 0 */

  OLED_SendCmd(0x81); /* 对比度设置 */
  OLED_SendCmd(0xCF);

  OLED_SendCmd(0xA1); /* 段重映射（右→左） */
  OLED_SendCmd(0xA6); /* 正向显示 */

  OLED_SendCmd(0xA8); /* 多路复用率 */
  OLED_SendCmd(0x3F); /* 1/64 duty */

  OLED_SendCmd(0xAD); /* 启动 DC-DC 电荷泵 */
  OLED_SendCmd(0x8B); /* 使用内部 VCC */

  OLED_SendCmd(0x33); /* 泵电压 10V */

  OLED_SendCmd(0xC8); /* COM 扫描方向（反向） */

  OLED_SendCmd(0xD3); /* 显示偏移 */
  OLED_SendCmd(0x00);

  OLED_SendCmd(0xD5); /* 内部时钟频率 */
  OLED_SendCmd(0xC0);

  OLED_SendCmd(0xD9); /* 预充电时间 */
  OLED_SendCmd(0x1F);

  OLED_SendCmd(0xDA); /* COM 引脚布局 */
  OLED_SendCmd(0x12);

  OLED_SendCmd(0xDB); /* VCOMH 电平 */
  OLED_SendCmd(0x40);

  OLED_NewFrame();
  OLED_ShowFrame();

  OLED_SendCmd(0xAF); /* 开启显示 */
}

/* ── BUG FIX: OLED_DisPlay_On / Off 修正 ───────────────────────────
 * 原代码使用的 0x8D/0x14/0x10 是 SSD1306 的电荷泵命令，
 * CH1116 没有 0x8D 命令，这些字节会被 CH1116 忽略或误解。
 * CH1116 正确的命令：0xAD (set charge pump) + 0x8B/0x8A。
 * ────────────────────────────────────────────────────────────────── */
void OLED_DisPlay_On() {
  OLED_SendCmd(0xAD); /* DC-DC 电荷泵设置 */
  OLED_SendCmd(0x8B); /* 启用内部 VCC */
  OLED_SendCmd(0xAF); /* 开启显示 */
}

void OLED_DisPlay_Off() {
  OLED_SendCmd(0xAD); /* DC-DC 电荷泵设置 */
  OLED_SendCmd(0x8A); /* 关闭内部 VCC */
  OLED_SendCmd(0xAE); /* 关闭显示 */
}

void OLED_SetColorMode(OLED_ColorMode mode) {
  if (mode == OLED_COLOR_NORMAL) {
    OLED_SendCmd(0xA6);
  }
  if (mode == OLED_COLOR_REVERSED) {
    OLED_SendCmd(0xA7);
  }
}

/* ========================== 显存操作 ========================== */

void OLED_NewFrame() {
  memset(OLED_GRAM, 0, sizeof(OLED_GRAM));
}

void OLED_ShowFrame() {
  static uint8_t sendBuffer[OLED_COLUMN + 1];
  sendBuffer[0] = 0x40;
  for (uint8_t i = 0; i < OLED_PAGE; i++) {
    OLED_SendCmd(0xB0 + i);
    OLED_SendCmd(0x02);
    OLED_SendCmd(0x10);
    memcpy(sendBuffer + 1, OLED_GRAM[i], OLED_COLUMN);
    OLED_Send(sendBuffer, OLED_COLUMN + 1);
  }
}

void OLED_SetPixel(uint8_t x, uint8_t y, OLED_ColorMode color) {
  if (x >= OLED_COLUMN || y >= OLED_ROW) return;
  if (!color) {
    OLED_GRAM[y / 8][x] |= 1 << (y % 8);
  } else {
    OLED_GRAM[y / 8][x] &= ~(1 << (y % 8));
  }
}

void OLED_SetByte_Fine(uint8_t page, uint8_t column, uint8_t data, uint8_t start, uint8_t end, OLED_ColorMode color) {
  static uint8_t temp;
  if (page >= OLED_PAGE || column >= OLED_COLUMN) return;
  if (color) data = ~data;
  temp = data | (0xff << (end + 1)) | (0xff >> (8 - start));
  OLED_GRAM[page][column] &= temp;
  temp = data & ~(0xff << (end + 1)) & ~(0xff >> (8 - start));
  OLED_GRAM[page][column] |= temp;
}

void OLED_SetByte(uint8_t page, uint8_t column, uint8_t data, OLED_ColorMode color) {
  if (page >= OLED_PAGE || column >= OLED_COLUMN) return;
  if (color) data = ~data;
  OLED_GRAM[page][column] = data;
}

void OLED_SetBits_Fine(uint8_t x, uint8_t y, uint8_t data, uint8_t len, OLED_ColorMode color) {
  uint8_t page = y / 8;
  uint8_t bit = y % 8;
  if (bit + len > 8) {
    OLED_SetByte_Fine(page, x, data << bit, bit, 7, color);
    OLED_SetByte_Fine(page + 1, x, data >> (8 - bit), 0, len + bit - 1 - 8, color);
  } else {
    OLED_SetByte_Fine(page, x, data << bit, bit, bit + len - 1, color);
  }
}

void OLED_SetBits(uint8_t x, uint8_t y, uint8_t data, OLED_ColorMode color) {
  uint8_t page = y / 8;
  uint8_t bit = y % 8;
  OLED_SetByte_Fine(page, x, data << bit, bit, 7, color);
  if (bit) {
    OLED_SetByte_Fine(page + 1, x, data >> (8 - bit), 0, bit - 1, color);
  }
}

void OLED_SetBlock(uint8_t x, uint8_t y, const uint8_t *data, uint8_t w, uint8_t h, OLED_ColorMode color) {
  uint8_t fullRow = h / 8;
  uint8_t partBit = h % 8;
  for (uint8_t i = 0; i < w; i++) {
    for (uint8_t j = 0; j < fullRow; j++) {
      OLED_SetBits(x + i, y + j * 8, data[i + j * w], color);
    }
  }
  if (partBit) {
    uint16_t fullNum = w * fullRow;
    for (uint8_t i = 0; i < w; i++) {
      OLED_SetBits_Fine(x + i, y + (fullRow * 8), data[fullNum + i], partBit, color);
    }
  }
}

/* ========================== 图形绘制 ========================== */

void OLED_DrawLine(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, OLED_ColorMode color) {
  static uint8_t temp = 0;
  if (x1 == x2) {
    if (y1 > y2) { temp = y1; y1 = y2; y2 = temp; }
    for (uint8_t y = y1; y <= y2; y++) OLED_SetPixel(x1, y, color);
  } else if (y1 == y2) {
    if (x1 > x2) { temp = x1; x1 = x2; x2 = temp; }
    for (uint8_t x = x1; x <= x2; x++) OLED_SetPixel(x, y1, color);
  } else {
    int16_t dx = x2 - x1, dy = y2 - y1;
    int16_t ux = ((dx > 0) << 1) - 1, uy = ((dy > 0) << 1) - 1;
    int16_t x = x1, y = y1, eps = 0;
    dx = abs(dx); dy = abs(dy);
    if (dx > dy) {
      for (x = x1; x != x2; x += ux) {
        OLED_SetPixel(x, y, color);
        eps += dy;
        if ((eps << 1) >= dx) { y += uy; eps -= dx; }
      }
    } else {
      for (y = y1; y != y2; y += uy) {
        OLED_SetPixel(x, y, color);
        eps += dx;
        if ((eps << 1) >= dy) { x += ux; eps -= dy; }
      }
    }
  }
}

void OLED_DrawRectangle(uint8_t x, uint8_t y, uint8_t w, uint8_t h, OLED_ColorMode color) {
  OLED_DrawLine(x, y, x + w, y, color);
  OLED_DrawLine(x, y + h, x + w, y + h, color);
  OLED_DrawLine(x, y, x, y + h, color);
  OLED_DrawLine(x + w, y, x + w, y + h, color);
}

void OLED_DrawFilledRectangle(uint8_t x, uint8_t y, uint8_t w, uint8_t h, OLED_ColorMode color) {
  for (uint8_t i = 0; i < h; i++) OLED_DrawLine(x, y + i, x + w, y + i, color);
}

void OLED_DrawTriangle(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t x3, uint8_t y3, OLED_ColorMode color) {
  OLED_DrawLine(x1, y1, x2, y2, color);
  OLED_DrawLine(x2, y2, x3, y3, color);
  OLED_DrawLine(x3, y3, x1, y1, color);
}

void OLED_DrawFilledTriangle(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t x3, uint8_t y3, OLED_ColorMode color) {
  uint8_t a = 0, b = 0, y = 0, last = 0;
  if (y1 > y2) { a = y2; b = y1; } else { a = y1; b = y2; }
  y = a;
  for (; y <= b; y++) {
    if (y <= y3) {
      OLED_DrawLine(x1 + (y - y1) * (x2 - x1) / (y2 - y1), y,
                    x1 + (y - y1) * (x3 - x1) / (y3 - y1), y, color);
    } else { last = y - 1; break; }
  }
  for (; y <= b; y++) {
    OLED_DrawLine(x2 + (y - y2) * (x3 - x2) / (y3 - y2), y,
                  x1 + (y - last) * (x3 - x1) / (y3 - last), y, color);
  }
}

void OLED_DrawCircle(uint8_t x, uint8_t y, uint8_t r, OLED_ColorMode color) {
  int16_t a = 0, b = r, di = 3 - (r << 1);
  while (a <= b) {
    OLED_SetPixel(x - b, y - a, color); OLED_SetPixel(x + b, y - a, color);
    OLED_SetPixel(x - a, y + b, color); OLED_SetPixel(x - b, y - a, color);
    OLED_SetPixel(x - a, y - b, color); OLED_SetPixel(x + b, y + a, color);
    OLED_SetPixel(x + a, y - b, color); OLED_SetPixel(x + a, y + b, color);
    OLED_SetPixel(x - b, y + a, color);
    a++;
    if (di < 0) { di += 4 * a + 6; }
    else { di += 10 + 4 * (a - b); b--; }
    OLED_SetPixel(x + a, y + b, color);
  }
}

void OLED_DrawFilledCircle(uint8_t x, uint8_t y, uint8_t r, OLED_ColorMode color) {
  int16_t a = 0, b = r, di = 3 - (r << 1);
  while (a <= b) {
    for (int16_t i = x - b; i <= x + b; i++) { OLED_SetPixel(i, y + a, color); OLED_SetPixel(i, y - a, color); }
    for (int16_t i = x - a; i <= x + a; i++) { OLED_SetPixel(i, y + b, color); OLED_SetPixel(i, y - b, color); }
    a++;
    if (di < 0) { di += 4 * a + 6; }
    else { di += 10 + 4 * (a - b); b--; }
  }
}

void OLED_DrawEllipse(uint8_t x, uint8_t y, uint8_t a, uint8_t b, OLED_ColorMode color) {
  int xpos = 0, ypos = b;
  int a2 = a * a, b2 = b * b;
  int d = b2 + a2 * (0.25 - b);
  while (a2 * ypos > b2 * xpos) {
    OLED_SetPixel(x + xpos, y + ypos, color); OLED_SetPixel(x - xpos, y + ypos, color);
    OLED_SetPixel(x + xpos, y - ypos, color); OLED_SetPixel(x - xpos, y - ypos, color);
    if (d < 0) { d = d + b2 * ((xpos << 1) + 3); xpos += 1; }
    else { d = d + b2 * ((xpos << 1) + 3) + a2 * (-(ypos << 1) + 2); xpos += 1; ypos -= 1; }
  }
  d = b2 * (xpos + 0.5) * (xpos + 0.5) + a2 * (ypos - 1) * (ypos - 1) - a2 * b2;
  while (ypos > 0) {
    OLED_SetPixel(x + xpos, y + ypos, color); OLED_SetPixel(x - xpos, y + ypos, color);
    OLED_SetPixel(x + xpos, y - ypos, color); OLED_SetPixel(x - xpos, y - ypos, color);
    if (d < 0) { d = d + b2 * ((xpos << 1) + 2) + a2 * (-(ypos << 1) + 3); xpos += 1; ypos -= 1; }
    else { d = d + a2 * (-(ypos << 1) + 3); ypos -= 1; }
  }
}

void OLED_DrawImage(uint8_t x, uint8_t y, const Image *img, OLED_ColorMode color) {
  OLED_SetBlock(x, y, img->data, img->w, img->h, color);
}

/* ========================== 文字绘制 ========================== */

void OLED_PrintASCIIChar(uint8_t x, uint8_t y, char ch, const ASCIIFont *font, OLED_ColorMode color) {
  OLED_SetBlock(x, y, font->chars + (ch - ' ') * (((font->h + 7) / 8) * font->w), font->w, font->h, color);
}

void OLED_PrintASCIIString(uint8_t x, uint8_t y, const char *str, const ASCIIFont *font, OLED_ColorMode color) {
  uint8_t x0 = x;
  while (*str) {
    OLED_PrintASCIIChar(x0, y, *str, font, color);
    x0 += font->w;
    str++;
  }
}

uint8_t _OLED_GetUTF8Len(const char *string) {
  if ((string[0] & 0x80) == 0x00) return 1;
  else if ((string[0] & 0xE0) == 0xC0) return 2;
  else if ((string[0] & 0xF0) == 0xE0) return 3;
  else if ((string[0] & 0xF8) == 0xF0) return 4;
  return 0;
}

void OLED_PrintString(uint8_t x, uint8_t y, const char *str, const Font *font, OLED_ColorMode color) {
  uint16_t i = 0;
  uint8_t oneLen = (((font->h + 7) / 8) * font->w) + 4;
  uint8_t found, utf8Len;
  uint8_t *head;
  while (str[i]) {
    found = 0;
    utf8Len = _OLED_GetUTF8Len(str + i);
    if (utf8Len == 0) break;
    for (uint8_t j = 0; j < font->len; j++) {
      head = (uint8_t *)(font->chars) + (j * oneLen);
      if (memcmp(str + i, head, utf8Len) == 0) {
        OLED_SetBlock(x, y, head + 4, font->w, font->h, color);
        x += font->w; i += utf8Len; found = 1; break;
      }
    }
    if (found == 0) {
      OLED_PrintASCIIChar(x, y, (utf8Len == 1) ? str[i] : ' ', font->ascii, color);
      x += font->ascii->w; i += utf8Len;
    }
  }
}
