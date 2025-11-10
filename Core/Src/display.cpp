#include "display.h"
#include "ssd1306.h"
#include <stdio.h> // For snprintf

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
 * @brief Private method to render the buffer and send it to the display via DMA.
 */
void MyDisplay::update_screen(void)
{
    // 1. Draw all content to the internal screen buffer
    ssd1306_Fill(Black);
    ssd1306_SetCursor(0, 0);

    if (this->current_key != 0) {
        // A key is active
        char str[10];
        snprintf(str, 10, "Key: %c", this->current_key);
        ssd1306_WriteString(str, &Font_6x8, White);
    } else {
        // No key is active
        ssd1306_WriteString("Press a key", &Font_6x8, White);
    }

    // 2. Start the non-blocking DMA transfer
    ssd1306_UpdateScreenDMA();

    // 3. Wait for the transfer to complete.
    // The semaphore is given by the I2C/DMA interrupt callbacks.
    // This also acts as our (now functional) I2C peripheral lock protection.
    xSemaphoreTake(g_i2c_tx_done_sem, pdMS_TO_TICKS(100));

    // 4. Clear the flag
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
