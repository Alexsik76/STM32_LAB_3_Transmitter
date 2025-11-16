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
	nrf24l01p_rx_init(106, _1Mbps);
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
void MyRadio::task(void)
{
    if (!this->init()) {
        vTaskDelete(NULL);
    }

    uint8_t rx_buf[NRF24L01P_PAYLOAD_LENGTH];

    while(1)
    {

        // --- 1. Перевіряємо, чи є нова робота (з черги) ---
    	if (xQueueReceive(this->tx_queue, NULL, 0) == pdTRUE)
		{
			// Переходимо в TX режим
			nrf24l01p_ptx_mode();

			// Відправляємо дані (використовуємо нову функцію)
			nrf24l01p_tx_transmit(radio_tx_buffer);
			HAL_GPIO_WritePin(NRF24L01P_CE_PIN_PORT, NRF24L01P_CE_PIN_NUMBER, GPIO_PIN_SET);
			vTaskDelay(pdMS_TO_TICKS(1)); // (Короткий імпульс в 1мс)
			HAL_GPIO_WritePin(NRF24L01P_CE_PIN_PORT, NRF24L01P_CE_PIN_NUMBER, GPIO_PIN_RESET);
			// (Ми будемо чекати на IRQ, щоб підтвердити відправку)
			xSemaphoreTake(g_radio_irq_sem, pdMS_TO_TICKS(100));

			// Очищуємо IRQ (TX_DS або MAX_RT)
			nrf24l01p_tx_irq();

			// Повертаємося в RX режим
			nrf24l01p_prx_mode();
			HAL_GPIO_WritePin(NRF24L01P_CE_PIN_PORT, NRF24L01P_CE_PIN_NUMBER, GPIO_PIN_SET); // (Знову починаємо слухати)
		}

        // --- 2. Перевіряємо, чи нас не розбудив IRQ (отримання даних) ---
        if (xSemaphoreTake(g_radio_irq_sem, 0) == pdTRUE)
        {
            // Отримано дані
            nrf24l01p_rx_receive(rx_buf);

            char status_str[37];
            snprintf(status_str, sizeof(status_str), "RX: %s", (char*)rx_buf);
            g_display.set_status_text(status_str);
        }

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
