#include <display.hpp>    // Наш новий .hpp
#include "rtos_tasks.h"   // Наш C-міст
#include <messaging.h>    // Наші структури команд
#include "cmsis_os.h"     // Новий API для RTOS
#include <string.h>       // для strncpy
#include <stdio.h>        // для snprintf

// Підключаємо драйвер дисплея та шрифти
#include "ssd1306.h"
#include "fonts.h"

// --- Глобальні об'єкти ---

// Хендл I2C (визначений у main.c / i2c.c)
extern I2C_HandleTypeDef hi2c1;

// Наш глобальний C++ об'єкт (тепер з аргументом)
MyDisplay g_display(&hi2c1);

// --- C-Wrappers (Вхідні точки) ---
extern "C" {

// Хендли, згенеровані CubeMX (з freertos.c)
extern osMessageQueueId_t displayQueueHandleHandle;
extern osSemaphoreId_t i2cTxDoneSemHandleHandle; // Ви казали, що MX згенерував це

/**
 * @brief Ініціалізація (зараз порожня, оскільки семафор створює MX)
 */
void display_init(void)
{
    // Cемафор 'i2cTxDoneSemHandleHandle' створюється у freertos.c
    // Тому ця функція залишається порожньою.
}

/**
 * @brief Вхідна точка задачі RTOS.
 */
void display_run_task(void)
{
    g_display.task(); // Передаємо керування C++ об'єкту
}

} // extern "C"

// --- C++ Class Implementation ---

/**
 * @brief Конструктор (з вашого старого коду)
 */
MyDisplay::MyDisplay(I2C_HandleTypeDef *hi2c)
{
    // Ініціалізація приватних членів
    this->hi2c = hi2c; // Зберігаємо вказівник
    this->current_key = 0;
    this->main_text[0] = '\0';
    this->status_text[0] = '\0';
    strncpy(this->status_text, "Booting...", sizeof(this->status_text) - 1);
}

/**
 * @brief Ініціалізація (з вашого старого коду)
 */
bool MyDisplay::init()
{
    // Чекаємо на стабілізацію (використовуємо CMSIS API)
    osDelay(100);
    return ssd1306_Init();
}

/**
 * @brief ГОЛОВНИЙ ЦИКЛ ЗАДАЧІ ДИСПЛЕЯ (з вашого НОВОГО коду)
 */
void MyDisplay::task(void)
{
    if (!this->init()) {
        vTaskDelete(NULL); // Не вдалося запустити дисплей
    }

    // Локальний буфер для отримання повідомлень з черги
    DisplayMessage_t msg;

    // Перше оновлення екрану при запуску
    this->update_screen_internal();

    while (1)
    {
        // "Заснути" і чекати на повідомлення (команду) з черги
    	if (osMessageQueueGet(displayQueueHandleHandle, &msg, NULL, osWaitForever) == osOK)
        {
            // Нас "розбудили"! Ми отримали команду.
            bool needs_redraw = false;

            switch (msg.command)
            {
                case DISPLAY_CMD_SET_STATUS:
                    this->set_status_text(msg.text);
                    needs_redraw = true;
                    break;

                case DISPLAY_CMD_SET_MAIN:
                    this->set_main_text(msg.text);
                    needs_redraw = true;
                    break;

                case DISPLAY_CMD_SHOW_KEY:
                    this->on_key_press(msg.key);
                    needs_redraw = true;
                    break;

                case DISPLAY_CMD_CLEAR_ALL:
                    this->clear_all();
                    needs_redraw = true;
                    break;
            }

            // Оновлюємо екран, ТІЛЬКИ якщо команда щось змінила
            if (needs_redraw)
            {
                this->update_screen_internal();
            }
        }
    }
}


// --- Реалізація методів малювання (з вашого СТАРОГО коду) ---

/**
 * @brief Оновлює внутрішній буфер status_text
 */
void MyDisplay::set_status_text(const char* text)
{
    strncpy(this->status_text, text, sizeof(this->status_text) - 1);
    this->status_text[sizeof(this->status_text) - 1] = '\0'; // Гарантуємо нуль-термінатор
}

/**
 * @brief Оновлює внутрішній буфер main_text
 */
void MyDisplay::set_main_text(const char* text)
{
    strncpy(this->main_text, text, sizeof(this->main_text) - 1);
    this->main_text[sizeof(this->main_text) - 1] = '\0';
}

/**
 * @brief Оновлює внутрішню змінну current_key
 */
void MyDisplay::on_key_press(char key)
{
    // '*' (або 0) діє як "очистити клавішу"
    this->current_key = (key == '*' || key == 0) ? 0 : key;
}

/**
 * @brief Очищує внутрішні буфери
 */
void MyDisplay::clear_all()
{
    this->status_text[0] = '\0';
    this->main_text[0] = '\0';
    this->current_key = 0;
}

/**
 * @brief Фактичне малювання (з вашого старого MyDisplay::update_screen)
 */
void MyDisplay::update_screen_internal()
{
    // 1. Очистити буфер драйвера
    ssd1306_Fill(Black);

    // --- Зона 1: Status Bar (Верх) ---
    if (this->status_text[0] != '\0')
    {
        ssd1306_SetCursor(0, 0);
        // Примітка: ваш старий код використовував Font_6x8, переконайтеся, що він є
        ssd1306_WriteString(this->status_text, &Font_6x8, White);
    }

    // --- Зона 2: Головна Область (Центр) ---
    if (this->main_text[0] != '\0')
    {
        // Є головний текст, малюємо його
        ssd1306_SetCursor(2, 31);
        ssd1306_WriteString_Large(this->main_text, &Font_11x18, White);
    }
    // Якщо головний текст порожній, малюємо натиснуту клавішу
    else if (this->current_key != 0)
    {
        // Немає головного тексту, але є клавіша
        ssd1306_SetCursor(58, 31); // Центруємо

        char str[2] = {this->current_key, '\0'};
        ssd1306_WriteString_Large(str, &Font_11x18, White);
    }

    // 4. Запустити неблокуючу передачу DMA
    ssd1306_UpdateScreenDMA();

    // 5. Чекати, поки DMA/I2C завершить (використовуємо CMSIS та нове ім'я семафора)
    // 100мс таймаут
    osSemaphoreAcquire(i2cTxDoneSemHandleHandle, 100);
//    osDelay(10);
}
