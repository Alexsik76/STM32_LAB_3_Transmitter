#ifndef __FONTS_H__
#define __FONTS_H__

#include <stdint.h>

// Структура опису шрифту
typedef struct {
    const uint8_t FontWidth;    // Ширина символу
    uint8_t FontHeight;   // Висота символу
    const uint16_t *data; // Вказівник на дані
} FontDef_t;

// Декларуємо шрифти, які додамо у .c файл
extern FontDef_t Font_7x10;
extern FontDef_t Font_11x18;

#endif // __FONTS_H__
