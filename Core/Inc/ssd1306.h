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

// --- Configuration ---

// Display I2C Address
#define SSD1306_I2C_ADDR        0x3C
// Display Dimensions
#define SSD1306_WIDTH           128
#define SSD1306_HEIGHT          64 // Using 128x64 display

// Colors
#define Black                   0x00
#define White                   0x01

// --- Public Functions ---

/**
 * @brief Initializes the SSD1306 controller.
 * @return 1 on success, 0 on failure.
 */
uint8_t ssd1306_Init(void);

/**
 * @brief Fills the entire screen buffer with a color.
 * @param color Black or White.
 */
void ssd1306_Fill(uint8_t color);

/**
 * @brief Draws a single pixel in the screen buffer.
 * @param x X coordinate.
 * @param y Y coordinate.
 * @param color Black or White.
 */
void ssd1306_DrawPixel(uint8_t x, uint8_t y, uint8_t color);

/**
 * @brief Sets the text cursor position in the screen buffer.
 * @param x X coordinate.
 * @param y Y coordinate.
 */
void ssd1306_SetCursor(uint8_t x, uint8_t y);

/**
 * @brief Draws a string in the screen buffer.
 * @param str Null-terminated string.
 * @param Font Font definition struct.
 * @param color Black or White.
 * @return Last character drawn.
 */
char ssd1306_WriteString(const char* str, FontDef_8bit_t* Font, uint8_t color);

/**
 * @brief Updates the screen using a blocking I2C write.
 * @note Used only for initial setup before RTOS starts.
 */
void ssd1306_UpdateScreen(void);

/**
 * @brief Updates the screen using a non-blocking DMA transfer.
 * @return HAL status of the DMA initiation.
 */
HAL_StatusTypeDef ssd1306_UpdateScreenDMA(void);

#ifdef __cplusplus
}
#endif

#endif // __SSD1306_H__
