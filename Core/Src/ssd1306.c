#include "ssd1306.h"
#include <string.h> // For memset (used in ssd1306_Fill)

/**
 * @brief Screen buffer (framebuffer).
 * @note 128 * 64 / 8 = 1024 bytes.
 */
static uint8_t SSD1306_Buffer[SSD1306_WIDTH * SSD1306_HEIGHT / 8];

/**
 * @brief Private function to send a single command byte.
 * @note This is a blocking function.
 * @param cmd The command byte to send.
 * @return HAL status.
 */
static HAL_StatusTypeDef ssd1306_WriteCommand(uint8_t cmd)
{
    // Uses 0x00 as the "Memory Address" to signify a command
    return HAL_I2C_Mem_Write(&hi2c1, (SSD1306_I2C_ADDR << 1), 0x00, 1, &cmd, 1, HAL_MAX_DELAY);
}

/**
 * @brief Initializes the SSD1306 controller (for 128x64).
 * @return 1 on success, 0 on failure.
 */
uint8_t ssd1306_Init(void)
{
    // Power-on delay
    HAL_Delay(100);

    // --- Initialization Sequence for 128x64 ---
    if (ssd1306_WriteCommand(0xAE) != HAL_OK) return 0; // Display OFF
    if (ssd1306_WriteCommand(0x20) != HAL_OK) return 0; // Set Memory Addressing Mode
    if (ssd1306_WriteCommand(0x00) != HAL_OK) return 0; // 00 = Horizontal
    if (ssd1306_WriteCommand(0xB0) != HAL_OK) return 0; // Set Page Start Address
    if (ssd1306_WriteCommand(0xC8) != HAL_OK) return 0; // Set COM Output Scan Direction
    if (ssd1306_WriteCommand(0x00) != HAL_OK) return 0; // ---set low column address
    if (ssd1306_WriteCommand(0x10) != HAL_OK) return 0; // ---set high column address
    if (ssd1306_WriteCommand(0x40) != HAL_OK) return 0; // --set start line address
    if (ssd1306_WriteCommand(0x81) != HAL_OK) return 0; // Set contrast
    if (ssd1306_WriteCommand(0xFF) != HAL_OK) return 0; // Max contrast
    if (ssd1306_WriteCommand(0xA1) != HAL_OK) return 0; // Set segment re-map 0 to 127
    if (ssd1306_WriteCommand(0xA6) != HAL_OK) return 0; // Set normal display
    if (ssd1306_WriteCommand(0xA8) != HAL_OK) return 0; // Set multiplex ratio
    if (ssd1306_WriteCommand(0x3F) != HAL_OK) return 0; // 1/64 duty (for 128x64)
    if (ssd1306_WriteCommand(0xD3) != HAL_OK) return 0; // Set display offset
    if (ssd1306_WriteCommand(0x00) != HAL_OK) return 0; // no offset
    if (ssd1306_WriteCommand(0xD5) != HAL_OK) return 0; // Set display clock divide ratio
    if (ssd1306_WriteCommand(0x80) != HAL_OK) return 0; //
    if (ssd1306_WriteCommand(0xD9) != HAL_OK) return 0; // Set pre-charge period
    if (ssd1306_WriteCommand(0xF1) != HAL_OK) return 0;
    if (ssd1306_WriteCommand(0xDA) != HAL_OK) return 0; // Set com pins hardware config
    if (ssd1306_WriteCommand(0x12) != HAL_OK) return 0; // (for 128x64)
    if (ssd1306_WriteCommand(0xDB) != HAL_OK) return 0; // Set vcomh
    if (ssd1306_WriteCommand(0x40) != HAL_OK) return 0;
    if (ssd1306_WriteCommand(0x8D) != HAL_OK) return 0; // Set Charge Pump
    if (ssd1306_WriteCommand(0x14) != HAL_OK) return 0; // Enabled
    if (ssd1306_WriteCommand(0xAF) != HAL_OK) return 0; // Display ON

    // Clear buffer
    ssd1306_Fill(Black);

    // Update screen (blocking method) for the first time
    ssd1306_UpdateScreen();

    return 1; // Success
}

/**
 * @brief Fills the entire screen buffer with a color.
 */
void ssd1306_Fill(uint8_t color)
{
    // Set all bytes in the buffer to 0x00 (Black) or 0xFF (White)
    uint8_t fill_val = (color == Black) ? 0x00 : 0xFF;
    memset(SSD1306_Buffer, fill_val, sizeof(SSD1306_Buffer));
}

/**
 * @brief Draws a single pixel in the screen buffer.
 */
void ssd1306_DrawPixel(uint8_t x, uint8_t y, uint8_t color)
{
    // Check boundaries
    if (x >= SSD1306_WIDTH || y >= SSD1306_HEIGHT) {
        return;
    }

    // Set or clear the specific bit for the pixel
    if (color == White) {
        SSD1306_Buffer[x + (y / 8) * SSD1306_WIDTH] |= (1 << (y % 8));
    } else {
        SSD1306_Buffer[x + (y / 8) * SSD1306_WIDTH] &= ~(1 << (y % 8));
    }
}

// Static cursor position for text
static uint8_t current_x = 0;
static uint8_t current_y = 0;

/**
 * @brief Sets the text cursor position.
 */
void ssd1306_SetCursor(uint8_t x, uint8_t y)
{
    current_x = x;
    current_y = y;
}

/**
 * @brief Draws a single character (8-bit font).
 */
static char ssd1306_WriteChar(char ch, FontDef_8bit_t* Font, uint8_t color)
{
    uint32_t i, j;

    // Check boundaries
    if (SSD1306_WIDTH <= (current_x + Font->FontWidth) ||
        SSD1306_HEIGHT <= (current_y + Font->FontHeight))
    {
        return 0;
    }

    // Draw character column by column
    for (i = 0; i < Font->FontWidth; i++) {
        uint8_t b = Font->data[(ch - 32) * Font->FontWidth + i];
        for (j = 0; j < Font->FontHeight; j++) {
            if ((b >> j) & 0x01) {
                ssd1306_DrawPixel(current_x + i, current_y + j, (uint8_t)color);
            } else {
                ssd1306_DrawPixel(current_x + i, current_y + j, (uint8_t)!color);
            }
        }
    }

    current_x += Font->FontWidth;
    return ch;
}

/**
 * @brief Draws a string (8-bit font).
 */
char ssd1306_WriteString(const char* str, FontDef_8bit_t* Font, uint8_t color)
{
    while (*str) {
        if (ssd1306_WriteChar(*str, Font, color) != *str) {
            return *str; // Error
        }
        str++;
    }
    return *str; // Success
}

/**
 * @brief Private function to set the display's memory "window" to fullscreen.
 * @note This is your refactor to remove code duplication.
 */
static void ssd1306_SetFullAddressWindow(void)
{
	ssd1306_WriteCommand(0x21); // Set column address
	ssd1306_WriteCommand(0);    // Start
	ssd1306_WriteCommand(SSD1306_WIDTH - 1); // End
	ssd1306_WriteCommand(0x22); // Set page address
	ssd1306_WriteCommand(0);    // Start
	ssd1306_WriteCommand(SSD1306_HEIGHT/8 - 1); // End (now 7 for 64px)
}

/**
 * @brief Updates the screen using a blocking I2C write.
 */
void ssd1306_UpdateScreen(void)
{
	ssd1306_SetFullAddressWindow();

    // Send buffer (blocking method)
    HAL_I2C_Mem_Write(&hi2c1, (SSD1306_I2C_ADDR << 1), 0x40, 1,
                      SSD1306_Buffer, sizeof(SSD1306_Buffer), HAL_MAX_DELAY);
}

/**
 * @brief Updates the screen using a non-blocking DMA transfer.
 */
HAL_StatusTypeDef ssd1306_UpdateScreenDMA(void)
{
    // 1. Set memory window
	ssd1306_SetFullAddressWindow();

    // 2. Start DMA transfer and return its initiation status
    return HAL_I2C_Mem_Write_DMA(&hi2c1, (SSD1306_I2C_ADDR << 1),
                                    0x40,
                                    I2C_MEMADD_SIZE_8BIT,
                                    SSD1306_Buffer,
                                    sizeof(SSD1306_Buffer));
}
