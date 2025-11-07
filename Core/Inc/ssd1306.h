#ifndef __SSD1306_H__
#define __SSD1306_H__
#ifdef __cplusplus
 extern "C" {
#endif
#include "main.h"
#include "fonts.h"
#include "FreeRTOS.h"
#include "semphr.h"

// HAL I2C handle
#include "i2c.h"

// Адреса дисплея
#define SSD1306_I2C_ADDR        0x3C
// Розміри
#define SSD1306_WIDTH           128
#define SSD1306_HEIGHT          32 // Змінено на 32

// Кольори
#define Black                   0x00
#define White                   0x01

// Ініціалізація
uint8_t ssd1306_Init(void);

// Функції малювання
void ssd1306_Fill(uint8_t color);
void ssd1306_DrawPixel(uint8_t x, uint8_t y, uint8_t color);
void ssd1306_SetCursor(uint8_t x, uint8_t y);
char ssd1306_WriteString(const char* str, FontDef_8bit_t* Font, uint8_t color);

// Функції оновлення екрану
void ssd1306_UpdateScreen(void); // Стара, блокуюча
void ssd1306_UpdateScreenDMA(SemaphoreHandle_t sem); // Наша нова, неблокуюча
#ifdef __cplusplus
}
#endif
#endif // __SSD1306_H__
