#pragma once
#include "main.h"
#include "i2c.h" // Потрібно для I2C_HandleTypeDef

// Підключаємо ваш драйвер дисплея
#include "ssd1306.h"

class MyDisplay {
public:
    /**
     * @brief Конструктор. Зберігає вказівник на I2C.
     */
    MyDisplay(I2C_HandleTypeDef *hi2c);

    /**
     * @brief Головний цикл задачі RTOS для дисплея.
     * Викликається з C-обгортки.
     */
    void task();

    /**
     * @brief Ініціалізує сам дисплей (SPI/I2C, команди).
     * @return true, якщо успішно.
     */
    bool init();

    // --- Методи-виконавці, які викликаються з task() ---
    void set_status_text(const char* text);
    void set_main_text(const char* text);
    void on_key_press(char key);
    void clear_all();

private:
    /**
     * @brief Приватний метод для фізичного оновлення екрану (з вашого старого коду).
     */
    void update_screen_internal();

    // --- Приватні члени (перенесено з вашого старого коду) ---
    I2C_HandleTypeDef* hi2c; // Вказівник на I2C

    // Внутрішні буфери для тексту
    char status_text[32];
    char main_text[32];
    char current_key;
};

// --- Глобальний C++ об'єкт ---
extern MyDisplay g_display;

// !!! ВСІ 'extern "C"' ПРОТОТИПИ ЗВІДСИ ВИДАЛЕНІ !!!
