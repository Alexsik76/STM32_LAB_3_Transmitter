#pragma once

#include "FreeRTOS.h"
#include "semphr.h"
#include "main.h"

// --- C-Wrappers ---
// C-callable functions for starting the task and initialization.
#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief RTOS task entry function (called by freertos.c).
 */
void display_task_entry(void *argument);

/**
 * @brief C-callable function to initialize the display subsystem.
 */
void display_init(void);

/**
 * @brief Global semaphore, signaled by I2C/DMA interrupts (defined in display.cpp).
 */
extern SemaphoreHandle_t g_i2c_tx_done_sem;

#ifdef __cplusplus
}
#endif

// --- C++ World ---
#ifdef __cplusplus

/**
 * @brief Main class for managing the OLED Display.
 * This class encapsulates all logic for the display task,
 * including state and screen updates.
 */
class MyDisplay
{
public:
    /**
     * @brief Constructor.
     * @param hi2c Pointer to the HAL I2C handle.
     */
    MyDisplay(I2C_HandleTypeDef *hi2c);

    /**
     * @brief The main RTOS task loop for the display.
     * This function runs forever after initialization.
     */
    void task(void);

    /**
     * @brief Public API for the keypad task to report a key press.
     * @param key The character that was pressed.
     */
    void on_key_press(char key);
    /**
         * @brief Public API to set the top-left status bar text.
         * @param text The new string to display.
         */
    void set_status_text(const char* text);
    void set_main_text(const char* text);
private:
    /**
     * @brief Initializes the SSD1306 controller.
     * @return true if successful, false otherwise.
     */
    bool init(void);

    /**
     * @brief Renders the current state to the buffer and sends it via DMA.
     */
    void update_screen(void);

    // --- Class State ---
    I2C_HandleTypeDef *hi2c;    // I2C handle
    char current_key;           // The last key pressed ('\0' = none)
    bool needs_update;          // Flag to trigger a screen redraw
    char status_text[24];
    char main_text[33];
};

#endif // __cplusplus
