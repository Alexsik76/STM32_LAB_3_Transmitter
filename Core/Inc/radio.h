#pragma once

#include "FreeRTOS.h"
#include "semphr.h"
#include "queue.h"
#include "main.h"

// --- C-Обгортки ---
#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief RTOS task entry function (called by freertos.c).
 */
void radio_task_entry(void *argument);

/**
 * @brief C-callable function to initialize the radio subsystem.
 */
void radio_init(void);

/**
 * @brief Global semaphore, signaled by the nRF24L01 IRQ pin.
 */
extern SemaphoreHandle_t g_radio_irq_sem;

#ifdef __cplusplus
}
#endif

// --- C++ Світ ---
#ifdef __cplusplus

/**
 * @brief Main class for managing the nRF24L01 Radio.
 * Encapsulates all RTOS logic for non-blocking communication.
 */
class MyRadio
{
public:
    /**
     * @brief Constructor.
     */
    MyRadio();

    /**
     * @brief The main RTOS task loop for the radio.
     */
    void task(void);

    /**
     * @brief Public API for other tasks to send data.
     * @param data 32-byte buffer to send.
     * @return true if successfully queued for sending.
     */
    bool send_data(uint8_t* data);

private:
    /**
     * @brief Initializes the nRF24L01 controller.
     * @return true if successful, false otherwise.
     */
    bool init(void);
    // --- Class State ---
    QueueHandle_t tx_queue;
};

#endif // __cplusplus
