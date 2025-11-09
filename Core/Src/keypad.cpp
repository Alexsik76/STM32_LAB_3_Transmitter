#include "keypad.h"
#include "FreeRTOS.h"
#include "task.h"
#include "ui_feedback.h"
#include "display.h" // Щоб "бачити" клас MyDisplay

// Оголошуємо, що g_display існує в іншому файлі
extern MyDisplay g_display;

// --- Глобальні C++ об'єкти ---
static MyKeypad g_keypad;
// (g_last_key and keypad_get_key видалені)

// --- Реалізація C++ класу MyKeypad ---

MyKeypad::MyKeypad() {}

void MyKeypad::init(void) {
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15, GPIO_PIN_RESET);
}

/**
 * @brief Це ВАШ 100% РОБОЧИЙ КОД сканування, перенесений у клас
 * @return Символ клавіші або '\0'
 */
char MyKeypad::scan_no_delay(void) {
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15, GPIO_PIN_SET);

    // Мікро-затримка для стабілізації, яку ми додали раніше
    for(volatile int i=0; i<500; i++);

    for (int c = 0; c < 4; c++) {
        HAL_GPIO_WritePin(GPIOB, col_pins[c], GPIO_PIN_RESET);
        for (int r = 0; r < 4; r++) {
            if (HAL_GPIO_ReadPin(GPIOA, row_pins[r]) == GPIO_PIN_RESET) {
                char key = key_map[r][c];
                // Повертаємо стовпці в стан очікування
                HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15, GPIO_PIN_RESET);
                return key;
            }
        }
        // Деактивуємо поточний стовпець
        HAL_GPIO_WritePin(GPIOB, col_pins[c], GPIO_PIN_SET);
    }

    // Повертаємо всі стовпці в стан очікування
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15, GPIO_PIN_RESET);
    return '\0';
}

// --- C-обгортки (міст до RTOS) ---
extern "C" {

/**
 * @brief Функція RTOS-задачі з ПРАВИЛЬНИM debounce
 */
void keypad_task(void* argument)
{
    g_keypad.init();

    char last_key_seen = 0;
    uint32_t press_time = 0;
    bool key_reported = false; // Прапорець, що ми вже відправили клавішу

    const uint32_t DEBOUNCE_TIME_MS = 50; // 50 мс на debounce

    while (1)
    {
        char key = g_keypad.scan_no_delay(); // Скануємо
        uint32_t now = xTaskGetTickCount() * portTICK_PERIOD_MS;

        if (key != 0) {
            // Кнопка натиснута
            if (key != last_key_seen) {
                // Це нова кнопка
                last_key_seen = key;
                press_time = now;
                key_reported = false; // Скидаємо прапорець
            } else if (now - press_time > DEBOUNCE_TIME_MS && !key_reported) {
                // Кнопка утримується > 50 мс і ми її ще не відправляли

                // === ГОЛОВНА ЛОГІКА ===
                // Передаємо натискання об'єкту дисплея
                g_display.on_key_press(key);
                key_reported = true; // Встановлюємо прапорець
                UI_Blink_Once();
            }
        } else {
            // Кнопку відпустили
            last_key_seen = 0;
            key_reported = false;
        }

        vTaskDelay(pdMS_TO_TICKS(10)); // Опитуємо кожні 10 мс
    }
}


} // extern "C"
