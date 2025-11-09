#include "display.h"
#include "ssd1306.h"
#include <stdio.h> // Для snprintf

// --- Глобальні об'єкти ---

// Створюємо глобальний семафор (доступний для main.c)
SemaphoreHandle_t g_i2c_tx_done_sem;

// Створюємо глобальний екземпляр нашого C++ класу
// (Ми не можемо створити його в display_init, бо C++ ще не готовий)
MyDisplay g_display(&hi2c1);

// --- C-Обгортки (Точка входу) ---
extern "C" {

// Ця функція викликається ОДИН РАЗ перед запуском RTOS
void display_init(void)
{
    // (Більше не створюємо g_display тут, він вже створений)
    g_i2c_tx_done_sem = xSemaphoreCreateBinary();
}

// Цю функцію викликає freertos.c для запуску нашого потоку
void display_task_entry(void *argument)
{
    // Просто передаємо керування нашому C++ об'єкту
    g_display.task();
}

} // extern "C"

// --- Реалізація C++ класу MyDisplay ---

MyDisplay::MyDisplay(I2C_HandleTypeDef *hi2c)
{
    // Ініціалізуємо приватні змінні
    this->hi2c = hi2c;
    this->current_key = 0;
    this->needs_update = true; // Оновити екран при першому запуску
}

/**
 * @brief Приватний метод ініціалізації I2C
 */
bool MyDisplay::init(void)
{
    vTaskDelay(pdMS_TO_TICKS(100)); // Затримка на "прокидання"
    return ssd1306_Init();
}

/**
 * @brief Публічний метод, який викликає keypad_task
 */
void MyDisplay::on_key_press(char key)
{
    char new_key_state = (key == '*') ? 0 : key;
    if (this->current_key != new_key_state) {
        // Стан змінився!
        this->current_key = new_key_state;
        this->needs_update = true; // Встановлюємо прапорець
    }
}

/**
 * @brief Приватний метод оновлення екрану (викликається з task())
 */
void MyDisplay::update_screen(void)
{
    ssd1306_Fill(Black);
    ssd1306_SetCursor(0, 0);

    if (this->current_key != 0) {
        char str[10];
        snprintf(str, 10, "Key: %c", this->current_key);
        ssd1306_WriteString(str, &Font_6x8, White);
    } else {
        ssd1306_WriteString("Hello RTOS!", &Font_6x8, White);
    }

    // Запускаємо DMA
    ssd1306_UpdateScreenDMA(g_i2c_tx_done_sem);

    // Чекаємо на семафор АБО ТАЙМАУТ
    if (xSemaphoreTake(g_i2c_tx_done_sem, pdMS_TO_TICKS(100)) == pdFALSE)
    {
        // ТАЙМАУТ! (Колбеки не спрацювали)
        // Це наш "аварійний" фікс, який ми довели
        HAL_I2C_DeInit(this->hi2c);
        MX_I2C1_Init();
    }

    this->needs_update = false; // Скидаємо прапорець
}

/**
 * @brief ГОЛОВНИЙ ЦИКЛ ПОТОКУ ДИСПЛЕЯ
 */
void MyDisplay::task(void)
{
    // 1. Гасимо діод
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);

    if (!this->init()) { vTaskDelete(NULL); } // Ініціалізуємо себе

    while (1)
    {
        // Ми більше не читаємо клавіатуру тут

        // Оновлюємо, ТІЛЬКИ якщо keypad_task попросив
        if (this->needs_update)
        {
            this->update_screen();
        }

        // Просто спимо і чекаємо
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}
