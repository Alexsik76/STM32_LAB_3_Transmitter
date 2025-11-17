#pragma once
#include "main.h"

class MyKeypad {
public:
    MyKeypad();

    /**
     * @brief Ініціалізує піни клавіатури.
     */
    void init();

    /**
     * @brief Сканує клавіатуру БЕЗ затримки (non-blocking).
     * @return Повертає символ натиснутої клавіші (1-9, *, #) або 0.
     */
    char scan_no_delay();

    /**
     * @brief Головний цикл задачі RTOS для клавіатури.
     * Викликається з C-обгортки.
     */
    void task();

private:
    // <<< --- ВЕСЬ ЦЕЙ БЛОК ПЕРЕНЕСЕНО З ВАШОГО СТАРОГО .h --- >>>

    // Key mapping
    const char key_map[4][4] = {
        {'1', '2', '3', 'A'},
        {'4', '5', '6', 'B'},
        {'7', '8', '9', 'C'},
        {'*', '0', '#', 'D'}
    };

    // Row pins (Inputs, Read)
    const uint16_t row_pins[4] = {
        KEY_ROW_0_Pin, // PA8
        KEY_ROW_1_Pin, // PA9
        KEY_ROW_2_Pin, // PA10
        KEY_ROW_3_Pin  // PA15
    };

    // Column pins (Outputs, Write)
    const uint16_t col_pins[4] = {
        KEY_COL_0_Pin, // PB12
        KEY_COL_1_Pin, // PB13
        KEY_COL_2_Pin, // PB14
        KEY_COL_3_Pin  // PB15
    };

    // Bitmask combining all 4 column pins for fast writing
    const uint16_t all_col_pins = col_pins[0] | col_pins[1] | col_pins[2] | col_pins[3];
};

// --- Глобальний C++ об'єкт ---
extern MyKeypad g_keypad;

