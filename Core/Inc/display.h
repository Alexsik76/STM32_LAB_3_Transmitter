#pragma once

#include "FreeRTOS.h"
#include "semphr.h"
#include "main.h"

// --- C-Обгортки ---
// (Вони потрібні лише для запуску C++ з C-файлу freertos.c)
#ifdef __cplusplus
extern "C" {
#endif

void display_task_entry(void *argument); // C-функція запуску
extern SemaphoreHandle_t g_i2c_tx_done_sem; // Наш глобальний семафор

#ifdef __cplusplus
}
#endif

// --- C++ Світ ---
#ifdef __cplusplus

// Оголошуємо наш C++ клас
class MyDisplay
{
public:
    MyDisplay(I2C_HandleTypeDef *hi2c); // Конструктор

    // Наш головний потік RTOS (тепер це метод класу)
    void task(void);

    // Публічний метод, який "приймає" натискання клавіш
    void on_key_press(char key);

private:
    bool init(void);            // Приватна ініціалізація
    void update_screen(void);   // Приватне оновлення екрану

    // --- Стан (State) класу ---
    I2C_HandleTypeDef *hi2c;
    char current_key;           // Поточна клавіша ('0' = нічого)
    bool needs_update;          // Прапорець "треба перемалювати"
};

#endif // __cplusplus
