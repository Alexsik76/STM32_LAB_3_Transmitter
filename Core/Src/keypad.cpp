#include "keypad.h"
#include "FreeRTOS.h"
#include "task.h"

// --- Глобальні C++ об'єкти та змінні ---
static MyKeypad g_keypad;
static volatile char g_last_key = 0;

// Масиви пінів з .h (ми не можемо отримати до них доступ
// з C-функцій, тому дублюємо тут)
static const uint16_t row_pins[4] = {GPIO_PIN_8, GPIO_PIN_9, GPIO_PIN_10, GPIO_PIN_11};
static const uint16_t col_pins[4] = {GPIO_PIN_12, GPIO_PIN_13, GPIO_PIN_14, GPIO_PIN_15};

// --- Реалізація C++ класу MyKeypad ---

MyKeypad::MyKeypad() {}

void MyKeypad::init(void) {
    // Встановлюємо всі стовпці в "0" (LOW) - стан очікування
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15, GPIO_PIN_RESET);
}

/**
 * @brief Сканує клавіатуру, оновлює g_last_key і блокується для debounce.
 * @note Ця функція ПОВІННА викликатися з RTOS-задачі, оскільки містить vTaskDelay.
 */
void MyKeypad::scan_and_update(void) {
    // Встановлюємо ВСІ стовпці у HIGH (неактивний стан)
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15, GPIO_PIN_SET);

    for (int c = 0; c < 4; c++) {
        // 1. Активуємо поточний стовпець (LOW)
        HAL_GPIO_WritePin(GPIOB, col_pins[c], GPIO_PIN_RESET);

        for (int r = 0; r < 4; r++) {
            if (HAL_GPIO_ReadPin(GPIOA, row_pins[r]) == GPIO_PIN_RESET) {
                // КНОПКА ЗНАЙДЕНА!

                // 1. МИТТЄВО оновлюємо глобальну змінну
                g_last_key = key_map[r][c];

                // 2. Тепер блокуємо ПОТІК, поки кнопку не відпустять
                while(HAL_GPIO_ReadPin(GPIOA, row_pins[r]) == GPIO_PIN_RESET) {
                    vTaskDelay(pdMS_TO_TICKS(20));
                }

                // 3. Повертаємо стовпці в стан очікування (LOW), як у вашому коді
                HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15, GPIO_PIN_RESET);
                return; // Виходимо з функції scan
            }
        }
        // 4. Деактивуємо поточний стовпець (HIGH)
        HAL_GPIO_WritePin(GPIOB, col_pins[c], GPIO_PIN_SET);
    }

    // 5. Повертаємо всі стовпці в стан очікування (LOW)
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15, GPIO_PIN_RESET);
}

// --- C-обгортки (міст до RTOS) ---
extern "C" {

void keypad_task(void* argument)
{
    g_keypad.init();

    while (1)
    {
        // Просто викликаємо scan, який сам все зробить
        g_keypad.scan_and_update();

        // Чекаємо 20 мс перед *наступною* спробою сканування
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}

char keypad_get_key(void) {
    char key = g_last_key;
    g_last_key = 0; // Скидаємо
    return key;
}

} // extern "C"
