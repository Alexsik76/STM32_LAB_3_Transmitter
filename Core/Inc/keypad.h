#pragma once

// --- C-Wrapper ---
// C-callable function for starting the task.
#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief RTOS task entry function (called by freertos.c).
 */
void keypad_task(void *argument);

#ifdef __cplusplus
}
#endif

// --- C++ World ---
#ifdef __cplusplus

#include "main.h"

/**
 * @brief Class for managing the 4x4 matrix keypad.
 * This class encapsulates the low-level scanning logic.
 */
class MyKeypad
{
public:
    /**
     * @brief Constructor.
     */
    MyKeypad();

    /**
     * @brief Initializes the keypad GPIOs to a default state.
     */
    void init(void);

    /**
     * @brief Performs a single, non-blocking scan of the keypad.
     * @return The character of the first key found, or '\0' if none.
     */
    char scan_no_delay(void);

private:
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
        KEY_ROW_3_Pin  // PA15 (Moved from PA11 to avoid USB conflict)
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

#endif // __cplusplus
