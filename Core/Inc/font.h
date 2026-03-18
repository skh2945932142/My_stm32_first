#ifndef __FONT_H__
#define __FONT_H__

#include <stdint.h>

typedef struct {
    uint8_t  w;
    uint8_t  h;
    const uint8_t *chars;
} ASCIIFont;

typedef struct {
    uint8_t  w;
    uint8_t  h;
    uint16_t len;            // 新增：包含的汉字数量
    const ASCIIFont *ascii;
    const uint8_t *chars;
} Font;

typedef struct {
    uint8_t  w;
    uint8_t  h;
    const uint8_t *data;
} Image;

/* 定义在 font.c，此处仅声明 */
extern const ASCIIFont afont16x8;   /* 16px高 × 8px宽，大字体 */
extern const ASCIIFont afont8x6;    /* 8px高  × 6px宽，小字体 */

#endif /* __FONT_H__ */
