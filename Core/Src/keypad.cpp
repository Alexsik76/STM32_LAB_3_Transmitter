#include <keypad.hpp>
#include "cmsis_os.h"

// --- Глобальний об'єкт ---
MyKeypad g_keypad;

// --- Глобальні хендли з freertos.c ---

// --- C-Wrappers (Вхідні точки) ---
extern "C" {
extern osMessageQueueId_t keyEventQueueHandleHandle;
void keypad_init(void)
{
    // Ми викликаємо init() на початку task(),
    // тому тут нічого робити не потрібно.
}

/**
 * @brief Вхідна точка задачі RTOS.
 * Це ім'я ("keypad_task_entry") ви вказали в CubeMX.
 */
void keypad_run_task(void)
{
    g_keypad.task(); // Передаємо керування C++ об'єкту
}

} // extern "C"

// --- C++ Class Implementation ---

MyKeypad::MyKeypad()
{
    // Конструктор
}

/**
 * @brief Ініціалізує піни клавіатури (з вашого СТАРОГО коду)
 */
void MyKeypad::init()
{
    // Взято з вашого старого MyKeypad::init()
    // Встановлює всі стовпці в LOW (idle state)
    // Примітка: переконайтеся, що 'all_col_pins' визначено у 'keypad.hpp'
    HAL_GPIO_WritePin(GPIOB, all_col_pins, GPIO_PIN_RESET);
}

/**
 * @brief Сканує клавіатуру (з вашого СТАРОГО коду)
 */
char MyKeypad::scan_no_delay()
{
    // === Вся ця логіка взята з вашого старого MyKeypad::scan_no_delay() ===

    // 1. Встановити всі стовпці в HIGH (неактивний стан для логіки pull-up)
    HAL_GPIO_WritePin(GPIOB, all_col_pins, GPIO_PIN_SET);

    // Мікро-затримка для стабілізації
    for(volatile int i=0; i<500; i++);

    // 2. Ітерація по стовпцях
    for (int c = 0; c < 4; c++) {
        // Активувати поточний стовпець (встановити LOW)
        HAL_GPIO_WritePin(GPIOB, col_pins[c], GPIO_PIN_RESET);

        // 3. Читати всі рядки
        for (int r = 0; r < 4; r++) {
            // Перевірити, чи пін "підтягнутий" до LOW
            if (HAL_GPIO_ReadPin(GPIOA, row_pins[r]) == GPIO_PIN_RESET) {
                char key = key_map[r][c];

                // 4. Повернути всі стовпці в idle (LOW)
                HAL_GPIO_WritePin(GPIOB, all_col_pins, GPIO_PIN_RESET);
                return key;
            }
        }
        // Деактивувати поточний стовпець (встановити HIGH)
        HAL_GPIO_WritePin(GPIOB, col_pins[c], GPIO_PIN_SET);
    }

    // 5. Повернути всі стовпці в idle (LOW)
    HAL_GPIO_WritePin(GPIOB, all_col_pins, GPIO_PIN_RESET);
    return '\0'; // Повертаємо null-символ, якщо нічого не натиснуто
}

/**
 * @brief ГОЛОВНИЙ ЦИКЛ ЗАДАЧІ КЛАВІАТУРИ
 * (Цей код з вашого НОВОГО файлу - він вже правильний)
 */
void MyKeypad::task(void)
{
    this->init(); // Ініціалізуємо піни

    // Змінні стану для "debounce"
    char last_key_seen = 0;
    uint32_t press_time = 0;
    bool key_reported = false;

    // (Примітка: переконайтеся, що CMSIS_OS V2 увімкнено для xTaskGetTickCount)
    // Якщо ні, osKernelGetTickCount() є CMSIS-еквівалентом
    const uint32_t DEBOUNCE_TIME_MS = 50;

    while (1)
    {
        // 1. Отримуємо "сире" натискання
        char key = this->scan_no_delay();

        // Використовуємо CMSIS-функції
        uint32_t now = osKernelGetTickCount(); // * portTICK_PERIOD_MS не потрібно, osKernelGetTickCount() вже в мс

        // (Виправлення: osKernelGetTickCount() повертає "тіки", не мс,
        //  тому логіка з вашого нового файлу була трішки неправильною)
        // Давайте використаємо `osKernelGetTickFreq()` для коректності
        // uint32_t now_ms = (osKernelGetTickCount() * 1000) / osKernelGetTickFreq();

        // АБО, якщо portTICK_PERIOD_MS визначено (що ймовірно), ваш код був правильний.
        // Давайте залишимо ваш код, але з CMSIS-функцією vTaskDelay (osDelay)

        // --- Повертаємось до вашої логіки з "нового" файлу, вона коректна ---
        // (Виправлення мого виправлення - ваш код був правильний,
        //  xTaskGetTickCount доступний через cmsis_os.h)

        uint32_t now_ticks = xTaskGetTickCount(); // Отримуємо тіки

        if (key != 0) {
            if (key != last_key_seen) {
                last_key_seen = key;
                press_time = now_ticks; // Зберігаємо тіки
                key_reported = false;
            }
            // Конвертуємо DEBOUNCE_TIME_MS в тіки для порівняння
            else if ( (now_ticks - press_time) > pdMS_TO_TICKS(DEBOUNCE_TIME_MS) && !key_reported) {
                key_reported = true;

                // === ГОЛОВНА ЗМІНА (вже була у вас) ===
                osMessageQueuePut(keyEventQueueHandleHandle, &key, 0, 0);
            }
        } else {
            last_key_seen = 0;
            key_reported = false;
        }

        osDelay(10); // Засипаємо на 10мс
    }
}
