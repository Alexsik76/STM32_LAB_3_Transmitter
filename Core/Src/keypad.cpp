#include "keypad.h"
#include "FreeRTOS.h"
#include "task.h"
#include "ui_feedback.h"
#include "display.h" // To see the MyDisplay class
#include "radio.h"
#include <cstring>
// Declare the global g_display object (defined in display.cpp)
extern MyDisplay g_display;
extern MyRadio g_radio;
// --- Global C++ Objects ---

/**
 * @brief Global instance of our C++ keypad class.
 */
static MyKeypad g_keypad;

// --- C++ Class Implementation ---

MyKeypad::MyKeypad() {}

/**
 * @brief Initializes the keypad GPIOs to a default (idle) state.
 */
void MyKeypad::init(void) {
    // Set all columns to LOW (idle state)
    // Using the 'all_col_pins' constant from keypad.h
    HAL_GPIO_WritePin(GPIOB, all_col_pins, GPIO_PIN_RESET);
}

/**
 * @brief Performs a single, non-blocking scan based on the proven C logic.
 * @return The character of the first key found, or '\0' if none.
 */
char MyKeypad::scan_no_delay(void) {
    // 1. Set all columns to HIGH (inactive state for pull-up logic)
    HAL_GPIO_WritePin(GPIOB, all_col_pins, GPIO_PIN_SET);

    // Micro-delay for signal stabilization
    for(volatile int i=0; i<500; i++);

    // 2. Iterate through columns
    for (int c = 0; c < 4; c++) {
        // Activate the current column (set LOW)
        HAL_GPIO_WritePin(GPIOB, col_pins[c], GPIO_PIN_RESET);

        // 3. Read all rows
        for (int r = 0; r < 4; r++) {
            // Check if the pull-up input pin is pulled LOW
            if (HAL_GPIO_ReadPin(GPIOA, row_pins[r]) == GPIO_PIN_RESET) {
                char key = key_map[r][c];

                // 4. Return all columns to idle state (LOW)
                HAL_GPIO_WritePin(GPIOB, all_col_pins, GPIO_PIN_RESET);
                return key;
            }
        }
        // Deactivate the current column (set HIGH)
        HAL_GPIO_WritePin(GPIOB, col_pins[c], GPIO_PIN_SET);
    }

    // 5. Return all columns to idle state (LOW)
    HAL_GPIO_WritePin(GPIOB, all_col_pins, GPIO_PIN_RESET);
    return '\0';
}

// --- C-Wrappers (RTOS Task) ---
extern "C" {

/**
 * @brief RTOS task for handling the keypad.
 * @note This task performs the scan and all debounce logic,
 * then communicates the result to the display task.
 */
/**
 * @brief RTOS task for handling the keypad.
 * @note This task performs the scan and all debounce logic,
 * then communicates the result to the display and radio tasks.
 */
void keypad_task(void* argument)
{
    g_keypad.init();

    // State variables for the debounce logic
    char last_key_seen = 0;
    uint32_t press_time = 0;
    bool key_reported = false;

    const uint32_t DEBOUNCE_TIME_MS = 50;

    while (1)
    {
        char key = g_keypad.scan_no_delay();
        uint32_t now = xTaskGetTickCount() * portTICK_PERIOD_MS;

        if (key != 0) {
            // A key is physically pressed
            if (key != last_key_seen) {
                // This is a new key press
                last_key_seen = key;
                press_time = now;
                key_reported = false;
            } else if (now - press_time > DEBOUNCE_TIME_MS && !key_reported) {
                // The key has been held for > 50ms and we haven't reported it yet
                key_reported = true;
//                UI_Blink_Once();             // Blink the LED

                // === НОВА ЛОГІКА КЕРУВАННЯ РЕЖИМАМИ ===

                if (key == '#') {
					// Режим 1: Передача "Abc"
					g_display.set_status_text("Sending...");    // Статус вгорі
					g_display.set_main_text("Abc");          // Дані в центрі
					g_display.on_key_press(0);               // Очищуємо натиснуту клавішу

					// Готуємо наш 32-байтний пакет
					uint8_t tx_buf[32] = {0}; // Обнуляємо буфер
					strncpy((char*)tx_buf, "Abc", sizeof(tx_buf) - 1);

					// Відправляємо дані в чергу radio_task
					g_radio.send_data(tx_buf);

				} else if (key == '*') {
					// '*' - це "Стерти" / Повернути в стан очікування
					g_display.set_status_text("Idle");
					g_display.set_main_text("");       // Очищуємо головний екран
					g_display.on_key_press(0);         // Очищуємо натиснуту клавішу

				} else {
					// Всі інші клавіші: просто відобразити
					g_display.set_status_text("Key Press");
					g_display.set_main_text("");       // Очищуємо головний екран
					g_display.on_key_press(key);       // Показуємо клавішу
				}
            }
        } else {
            // No key is pressed
            last_key_seen = 0;
            key_reported = false;
        }

        vTaskDelay(pdMS_TO_TICKS(10)); // Poll every 10ms
    }
}

} // extern "C"
