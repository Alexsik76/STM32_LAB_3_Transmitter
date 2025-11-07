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
void display_task(void* argument)
{
    // Ініціалізуємо дисплей (використовуємо версію без "пінгу")
    if (!g_display->init())
    {
        // Якщо дисплей не знайдено, задача просто вбиває себе
        vTaskDelete(NULL);
    }

    // Якщо все добре, починаємо нескінченний цикл оновлення
    while (1)
    {
        // --- Тут буде логіка малювання ---
        ssd1306_Fill(Black);
        ssd1306_SetCursor(0, 0);
        ssd1306_WriteString("Hello RTOS!", &Font_6x8, White);

        // --- Логіка оновлення ---
        g_display->update_screen_DMA(); // Запускаємо DMA

        // Чекаємо на семафор (який "віддасть" переривання DMA)
        // з таймаутом 100 мс
        xSemaphoreTake(g_i2c_tx_done_sem, pdMS_TO_TICKS(100));

        // Чекаємо 200 мс перед наступним кадром
        vTaskDelay(pdMS_TO_TICKS(200));
    }
}
}


