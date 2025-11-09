#include "ssd1306.h"
#include <string.h>
// Глобальний буфер екрану (128 * 32 / 8 = 512 байт)
static uint8_t SSD1306_Buffer[SSD1306_WIDTH * SSD1306_HEIGHT / 8];

// Вказівник на дані для DMA-передачі
// [0] = Control Byte (0x40 - "data")
// [1..512] = наш буфер
static uint8_t SSD1306_DMATransmitBuffer[sizeof(SSD1306_Buffer) + 1];

// Приватна функція для відправки команд (блокуюча)
static HAL_StatusTypeDef ssd1306_WriteCommand(uint8_t cmd)
{
    return HAL_I2C_Mem_Write(&hi2c1, (SSD1306_I2C_ADDR << 1), 0x00, 1, &cmd, 1, HAL_MAX_DELAY);
}

// Ініціалізація
uint8_t ssd1306_Init(void)
{
    // Затримка на ввімкненняЯк мінімум, спочатку успішний коміт
    HAL_Delay(100);
    // Послідовність ініціалізації для 128x32
    if (ssd1306_WriteCommand(0xAE) != HAL_OK) return 0; // Display OFF
    if (ssd1306_WriteCommand(0x20) != HAL_OK) return 0; // Set Memory Addressing Mode
    if (ssd1306_WriteCommand(0x00) != HAL_OK) return 0; // 00 = Horizontal
    if (ssd1306_WriteCommand(0xB0) != HAL_OK) return 0; // Set Page Start Address (0xB0-B7)
    if (ssd1306_WriteCommand(0xC8) != HAL_OK) return 0; // Set COM Output Scan Direction
    if (ssd1306_WriteCommand(0x00) != HAL_OK) return 0; // ---set low column address
    if (ssd1306_WriteCommand(0x10) != HAL_OK) return 0; // ---set high column address
    if (ssd1306_WriteCommand(0x40) != HAL_OK) return 0; // --set start line address
    if (ssd1306_WriteCommand(0x81) != HAL_OK) return 0; // Set contrast
    if (ssd1306_WriteCommand(0xFF) != HAL_OK) return 0;
    if (ssd1306_WriteCommand(0xA1) != HAL_OK) return 0; // Set segment re-map 0 to 127
    if (ssd1306_WriteCommand(0xA6) != HAL_OK) return 0; // Set normal display
    if (ssd1306_WriteCommand(0xA8) != HAL_OK) return 0; // Set multiplex ratio
    if (ssd1306_WriteCommand(0x1F) != HAL_OK) return 0; // 1/32 duty (for 128x32)
    if (ssd1306_WriteCommand(0xD3) != HAL_OK) return 0; // Set display offset
    if (ssd1306_WriteCommand(0x00) != HAL_OK) return 0; // no offset
    if (ssd1306_WriteCommand(0xD5) != HAL_OK) return 0; // Set display clock divide ratio
    if (ssd1306_WriteCommand(0x80) != HAL_OK) return 0; //
    if (ssd1306_WriteCommand(0xD9) != HAL_OK) return 0; // Set pre-charge period
    if (ssd1306_WriteCommand(0xF1) != HAL_OK) return 0;
    if (ssd1306_WriteCommand(0xDA) != HAL_OK) return 0; // Set com pins hardware config
    if (ssd1306_WriteCommand(0x02) != HAL_OK) return 0; // (for 128x32)
    if (ssd1306_WriteCommand(0xDB) != HAL_OK) return 0; // Set vcomh
    if (ssd1306_WriteCommand(0x40) != HAL_OK) return 0;
    if (ssd1306_WriteCommand(0x8D) != HAL_OK) return 0; // Set Charge Pump
    if (ssd1306_WriteCommand(0x14) != HAL_OK) return 0; // Enabled
    if (ssd1306_WriteCommand(0xAF) != HAL_OK) return 0; // Display ON

    // Очищуємо буфер
    ssd1306_Fill(Black);
    // Оновлюємо екран (блокуючим методом, 1 раз)
    ssd1306_UpdateScreen();

    // Перший байт DMA-буфера - це завжди "Control Byte"
    // 0x40 = "я зараз буду надсилати дані"
    SSD1306_DMATransmitBuffer[0] = 0x40;

    return 1; // Успіх
}

// Заповнення буфера
void ssd1306_Fill(uint8_t color)
{
    uint8_t fill_val = (color == Black) ? 0x00 : 0xFF;
    for (uint32_t i = 0; i < sizeof(SSD1306_Buffer); i++)
    {
        SSD1306_Buffer[i] = fill_val;
    }
}

// Малювання пікселя у буфер
void ssd1306_DrawPixel(uint8_t x, uint8_t y, uint8_t color)
{
    if (x >= SSD1306_WIDTH || y >= SSD1306_HEIGHT) {
        return;
    }

    if (color == White) {
        SSD1306_Buffer[x + (y / 8) * SSD1306_WIDTH] |= (1 << (y % 8));
    } else {
        SSD1306_Buffer[x + (y / 8) * SSD1306_WIDTH] &= ~(1 << (y % 8));
    }
}

// Встановлення курсора (потрібно для виводу тексту)
static uint8_t current_x = 0;
static uint8_t current_y = 0;

void ssd1306_SetCursor(uint8_t x, uint8_t y)
{
    current_x = x;
    current_y = y;
}

// Вивід символу у буфер
static char ssd1306_WriteChar(char ch, FontDef_8bit_t* Font, uint8_t color)
{
    uint32_t i, j;

    if (SSD1306_WIDTH <= (current_x + Font->FontWidth) ||
        SSD1306_HEIGHT <= (current_y + Font->FontHeight))
    {
        return 0;
    }

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

// Вивід рядка у буфер (оновлено для 8-бітного шрифту)
char ssd1306_WriteString(const char* str, FontDef_8bit_t* Font, uint8_t color)
{
    while (*str) {
        if (ssd1306_WriteChar(*str, Font, color) != *str) {
            return *str;
        }
        str++;
    }
    return *str;
}

static void ssd1306_SetFullAddressWindow(void){
	ssd1306_WriteCommand(0x21); // Set column address
	ssd1306_WriteCommand(0);    // Start
	ssd1306_WriteCommand(SSD1306_WIDTH - 1); // End
	ssd1306_WriteCommand(0x22); // Set page address
	ssd1306_WriteCommand(0);    // Start
	ssd1306_WriteCommand(SSD1306_HEIGHT/8 - 1); // End (4 сторінки для 32px)

}
// Блокуюча функція оновлення екрану (використовується в Init)
void ssd1306_UpdateScreen(void)
{
	ssd1306_SetFullAddressWindow();
    // Відправляємо буфер (блокуючим методом)
    HAL_I2C_Mem_Write(&hi2c1, (SSD1306_I2C_ADDR << 1), 0x40, 1,
                      SSD1306_Buffer, sizeof(SSD1306_Buffer), HAL_MAX_DELAY);
}

// *** НАША ГОЛОВНА ФУНКЦІЯ ***
// Неблокуюча функція оновлення через DMA
// *** НАША ГОЛОВНА ФУНКЦІЯ (ПРАВИЛЬНА) ***
HAL_StatusTypeDef ssd1306_UpdateScreenDMA(void)
{
    // 1. Встановлюємо адресацію
	ssd1306_SetFullAddressWindow();
    // 2. Запускаємо DMA передачу
    return HAL_I2C_Mem_Write_DMA(&hi2c1, (SSD1306_I2C_ADDR << 1),
                                    0x40,
                                    I2C_MEMADD_SIZE_8BIT,
                                    SSD1306_Buffer,
                                    sizeof(SSD1306_Buffer));
}




