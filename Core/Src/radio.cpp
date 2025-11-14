#include "radio.h"
#include "nrf24l01.h" // Include the low-level driver
#include "FreeRTOS.h"
#include "task.h"
#include <string.h> // For memcpy

// --- Global Objects ---

/**
 * @brief Global semaphore, signaled by the nRF24L01 IRQ pin.
 */
SemaphoreHandle_t g_radio_irq_sem;

/**
 * @brief Global instance of our C++ radio class.
 */
MyRadio g_radio;

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
    if (!NRF24_Init()) {
        // SPI error or module not found
        return false;
    }

    NRF24_SetTXAddress(TX_ADDRESS);
    NRF24_SetRXAddress(RX_ADDRESS);
    NRF24_SetRFChannel(106); // Channel 106 (2.506 GHz)

    // Default to RX mode
    NRF24_RXMode();

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

    while(1)
    {
        // --- 1. Перевіряємо, чи є нова робота (з черги) ---
        if (xQueueReceive(this->tx_queue, NULL, 0) == pdTRUE)
        {
            // Є робота! Переходимо в TX режим
            NRF24_TXMode();

            // Відправляємо дані (це блокуюча функція з нашого .c)
            NRF24_Transmit(radio_tx_buffer);

            // (NRF24_Transmit ВЖЕ чекає на IRQ,
            // тому нам не потрібно чекати на семафор тут)

            // Повертаємося в RX режим (слухати)
            NRF24_RXMode();
        }

        // --- 2. Перевіряємо, чи нас не розбудив IRQ (отримання даних) ---
        if (xSemaphoreTake(g_radio_irq_sem, 0) == pdTRUE)
        {
            uint8_t status = NRF24_GetStatus();
            if (status & (1 << 6)) // RX_DR (Data Ready)
            {
                uint8_t rx_buf[32];
                NRF24_GetData(rx_buf);
                NRF24_ClearInterrupts();
                // TODO: Обробити отримані дані rx_buf
            }
            // (Ми ігноруємо TX_DS та MAX_RT, бо NRF24_Transmit обробляє їх сам)
        }

        // "Спимо" 10мс перед наступною перевіркою
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
