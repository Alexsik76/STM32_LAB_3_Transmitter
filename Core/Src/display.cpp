#include "display.h"
#include "ssd1306.h"
#include <stdio.h> // For snprintf
#include <cstring>
// --- Global Objects ---

/**
 * @brief Global semaphore, signaled by I2C/DMA interrupts (defined in display.cpp).
 * It is declared 'extern' in display.h to be visible by main.c
 */
SemaphoreHandle_t g_i2c_tx_done_sem;

/**
 * @brief Global instance of our C++ display class.
 * @note This relies on hi2c1 being globally defined in i2c.h (which is included via ssd1306.h)
 */
MyDisplay g_display(&hi2c1);

// --- C-Wrappers (Entry Point) ---
extern "C" {

/**
 * @brief C-callable function to initialize the display subsystem.
 * @note Called once from main.c before the RTOS scheduler starts.
 */
void display_init(void)
{
    // Create the binary semaphore (it starts "empty")
    g_i2c_tx_done_sem = xSemaphoreCreateBinary();
}

/**
 * @brief RTOS task entry function.
 * @note Called by freertos.c to start the display task.
 */
void display_task_entry(void *argument)
{
    // Pass control to our C++ object's main task method
    g_display.task();
}

} // extern "C"

// --- C++ Class Implementation ---

/**
 * @brief Constructor for MyDisplay class.
 */
MyDisplay::MyDisplay(I2C_HandleTypeDef *hi2c)
{
    // Initialize private member variables
    this->hi2c = hi2c;
    this->current_key = 0;      // '\0' means no key is active
    this->needs_update = true;  // Force a screen update on the first run
    // Initialize the status text buffer
    strncpy(this->status_text, "Press a key", sizeof(this->status_text) - 1);
}

/**
 * @brief Private method to initialize the SSD1306 controller.
 */
bool MyDisplay::init(void)
{
    // Wait for the display to power on and stabilize
    vTaskDelay(pdMS_TO_TICKS(100));
    return ssd1306_Init();
}

/**
 * @brief Public API called by the keypad task to report a key press.
 */
void MyDisplay::on_key_press(char key)
{
    // '*' key acts as a 'clear' command
    char new_key_state = (key == '*') ? 0 : key;

    if (this->current_key != new_key_state) {
        // Only update if the state has actually changed
        this->current_key = new_key_state;
        this->needs_update = true; // Set the flag to trigger a redraw
    }
}
/**
 * @brief Public API to set the top-left status bar text.
 */
void MyDisplay::set_status_text(const char* text)
{
    // Copy the new text into our buffer
    strncpy(this->status_text, text, sizeof(this->status_text) - 1);
    this->status_text[sizeof(this->status_text) - 1] = '\0'; // Ensure null termination

    this->needs_update = true; // Trigger a screen redraw
}

/**
 * @brief Private method to render the buffer and send it to the display via DMA.
 */
/**
 * @brief Renders the 2-zone UI to the buffer and sends it via DMA.
 */
void MyDisplay::update_screen(void)
{
    // 1. Clear the entire buffer
    ssd1306_Fill(Black);

    // --- Zone 1: Status Bar (Top 16 pixels) ---

    // Set cursor to the top-left for the status text
    ssd1306_SetCursor(0, 0);

    ssd1306_WriteString(this->status_text, &Font_6x8, White);

    // (We will reserve the top-right corner for radio icons later)
    // ssd1306_SetCursor(112, 0);
    // ssd1306_WriteString("[SND]", &Font_6x8, White);


    // --- Zone 2: Main Area (Bottom 48 pixels) ---

    if (this->current_key != 0) {
        // A key is pressed, draw it big!

        // Center the large character
        // 128 (screen width) - 11 (font width) = 117. 117 / 2 = 58
        // 16 (status bar height) + ( (64-16) / 2 ) - (18 / 2) = 16 + 24 - 9 = 31
        ssd1306_SetCursor(58, 31);

        // Create a 2-char string to print the single key
        char str[2] = {this->current_key, '\0'};
        ssd1306_WriteString_Large(str, &Font_11x18, White);
    }

    // 4. Start the non-blocking DMA transfer
    ssd1306_UpdateScreenDMA();

    // 5. Wait for the transfer to complete.
    xSemaphoreTake(g_i2c_tx_done_sem, pdMS_TO_TICKS(100));

    // 6. Clear the flag
    this->needs_update = false;
}

/**
 * @brief The main, infinite loop for the display RTOS task.
 */
void MyDisplay::task(void)
{
    // 1. Turn off the onboard LED (PC13)
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);

    // Initialize the display
    if (!this->init()) {
        vTaskDelete(NULL); // Initialization failed, kill the task
    }

    // Main task loop
    while (1)
    {
        // We no longer read the keypad here.
        // We only check if the keypad task has "told" us to redraw.
        if (this->needs_update)
        {
            this->update_screen();
        }

        // Sleep to yield CPU time.
        // The display will only update as fast as the keypad sends events.
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}
