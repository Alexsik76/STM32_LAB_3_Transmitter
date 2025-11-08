/*
 * display.cpp
 *
 *  Created on: Nov 7, 2025
 *      Author: osiko
 */
#include "display.h"      // Підключаємо наш .h файл
#include "ssd1306.h"      // Пізніше ми додамо цю бібліотеку
#include "FreeRTOS.h"     // Підключаємо RTOS
#include "task.h"         // Для vTaskDelay
#include "semphr.h"       // Для семафорів
#include "i2c.h"
#include "keypad.h"
#include <stdio.h>

// --- Глобальні C++ об'єкти ---
static MyDisplay* g_display; // Глобальний вказівник на наш C++ об'єкт
static SemaphoreHandle_t g_i2c_tx_done_sem; // Наш семафор

// --- Реалізація C++ класу MyDisplay ---

// Конструктор: просто зберігаємо вказівник на I2C
MyDisplay::MyDisplay(I2C_HandleTypeDef* hi2c)
{
    this->hi2c = hi2c;
}

// Метод ініціалізації
bool MyDisplay::init(void)
{
    // 1. "Пінг" пристрою з коротким таймаутом
    if (HAL_I2C_IsDeviceReady(this->hi2c, (SSD1306_I2C_ADDR << 1), 1, 100) != HAL_OK)
    {
        return false; // Дисплея немає на шині
    }

    // 2. Ініціалізація (поки що блокуюча)
    // ssd1306_Init() поверне 1 (true) при успіху
    return ssd1306_Init();
}

// Метод неблокуючого оновлення
void MyDisplay::update_screen_DMA(void)
{
    // Запускаємо передачу буфера через DMA
    // Ми будемо використовувати готову функцію з бібліотеки ssd1306
    ssd1306_UpdateScreenDMA(g_i2c_tx_done_sem);
}

// --- C-обгортки (міст до main.c) ---
extern "C" {
// Цю функцію викличе main.c ОДИН РАЗ при старті
void display_init(void)
{
    // 1. Створюємо семафор
    g_i2c_tx_done_sem = xSemaphoreCreateBinary();
    // 2. Створюємо C++ об'єкт
    g_display = new MyDisplay(&hi2c1);
}

// Це тіло нашої RTOS-задачі
// Це тіло нашої RTOS-задачі
// Це тіло нашої RTOS-задачі
// Це тіло нашої RTOS-задачі
// Це тіло нашої RTOS-задачі
void display_task(void* argument)
{
    // ... ініціалізація ...

    char current_key = 0; // Локальна змінна для відображення, що зберігає стан
    while (1)
    {
        char key = keypad_get_key(); // key буде != 0 лише в момент, коли було натиснуто

        // --- ЛОГІКА ЗБЕРЕЖЕННЯ СТАНУ ---
        if (key != 0) { // Якщо ми отримали подію
            if (key == '*') {
                current_key = 0; // СКИНУТИ стан
            } else {
                current_key = key; // ЗБЕРЕГТИ останнє натискання
            }
        }
        // --- КІНЕЦЬ ЛОГІКИ ЗБЕРЕЖЕННЯ СТАНУ ---

        ssd1306_Fill(Black);
        ssd1306_SetCursor(0, 0);

        if (current_key != 0) {
            char str[10];
            snprintf(str, 10, "Key: %c", current_key);
            ssd1306_WriteString(str, &Font_6x8, White);
        } else {
            // Виводимо "Hello RTOS!" тільки коли current_key = 0

            ssd1306_WriteString("Hello RTOS!", &Font_6x8, White);
            char str[10];
            if (current_key >= 32 && current_key <= 126) {
                snprintf(str, 20, "Key: %c", current_key);
            } else {
                // Якщо це 0 (або інший недрукований символ), виводимо його код
                snprintf(str, 20, "Code: %d", current_key);
            }
            ssd1306_SetCursor(0, 10);
            ssd1306_WriteString(str, &Font_6x8, White);


        }

        g_display->update_screen_DMA();
        xSemaphoreTake(g_i2c_tx_done_sem, pdMS_TO_TICKS(100));
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

}



