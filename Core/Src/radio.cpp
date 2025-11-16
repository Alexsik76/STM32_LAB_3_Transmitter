#include "radio.h"
#include "nrf24l01p.h" // Include the low-level driver
#include "FreeRTOS.h"
#include "task.h"
#include <string.h> // For memcpy
#include "display.h"
#include <stdio.h>
#include "ui_feedback.h"

// --- Global Objects ---

/**
 * @brief Global semaphore, signaled by the nRF24L01 IRQ pin.
 */
SemaphoreHandle_t g_radio_irq_sem;

/**
 * @brief Global instance of our C++ radio class.
 */
MyRadio g_radio;
extern MyDisplay g_display;
// --- Pipe Addresses ---
// (Must be the same on transmitter and receiver)
uint8_t TX_ADDRESS[5] = {0xEE, 0xDD, 0xCC, 0xBB, 0xAA};
uint8_t RX_ADDRESS[5] = {0xEE, 0xDD, 0xCC, 0xBB, 0xAA};
uint8_t radio_tx_buffer[32];
// --- C-Wrappers (Entry Point) ---
extern "C" {

/**
 * @brief C-callable function to initialize the radio subsystem.
 */
void radio_init(void)
{
    // Create the binary semaphore for IRQ
    g_radio_irq_sem = xSemaphoreCreateBinary();
}

/**
 * @brief RTOS task entry function.
 */
void radio_task_entry(void *argument)
{
    g_radio.task();
}

} // extern "C"

// --- C++ Class Implementation ---

MyRadio::MyRadio()
{
	this->tx_queue = xQueueCreate(1, 0);
}

/**
 * @brief Private method to initialize the nRF24.
 */
bool MyRadio::init(void)
{
	nrf24l01p_tx_init(106, _1Mbps);
	return true;
}

/**
 * @brief Public API for other tasks to send data.
 */
bool MyRadio::send_data(uint8_t* data)
{
	memcpy(radio_tx_buffer, data, 32);
	if (xQueueSend(this->tx_queue, NULL, 0) == pdTRUE) {
	        return true;
	    }
	return false;
}

/**
 * @brief MAIN RADIO TASK LOOP
 */
/**
 * @brief MAIN RADIO TASK LOOP
 */
/**
 * @brief MAIN RADIO TASK LOOP (ТІЛЬКИ ПЕРЕДАВАЧ)
 */
void MyRadio::task(void)
{
    if (!this->init()) { // Це тепер коректно викликає nrf24l01p_tx_init
        vTaskDelete(NULL);
    }

    // Згідно з драйвером, nrf24l01p_tx_init вже встановив CE HIGH.
    // Модуль знаходиться в режимі Standby-II, готовий до передачі.

    while(1)
    {
        // --- 1. Чекаємо, поки keypad_task дасть нам роботу ---
        // Блокуємо задачу, поки в черзі не з'явиться повідомлення
        if (xQueueReceive(this->tx_queue, NULL, portMAX_DELAY) == pdTRUE)
        {
            // Є робота!

            // 1. Завантажуємо дані в FIFO
            nrf24l01p_write_tx_fifo(radio_tx_buffer);

            // 2. Оскільки CE вже HIGH, передача почнеться автоматично.

            // 3. Чекаємо на семафор від IRQ (до 100мс), що передача завершена
            if (xSemaphoreTake(g_radio_irq_sem, pdMS_TO_TICKS(100)) == pdTRUE)
            {
                // IRQ спрацював!
                // 4. Обробимо IRQ (скинемо прапори TX_DS/MAX_RT і блимнемо діодом)
                nrf24l01p_tx_irq();
            }
            else
            {
                // IRQ не спрацював (Timeout).
                // Це означає, що ми не отримали переривання.
                // Скинемо TX FIFO, щоб підготуватися до наступної спроби.
                nrf24l01p_flush_tx_fifo();
            }

            // Ми НЕ переходимо в RX-режим і НЕ чіпаємо CE.
            // Модуль залишається в режимі Standby-II (CE=HIGH, PRIM_RX=0),
            // готовий до наступної передачі.
        }
    }
}
