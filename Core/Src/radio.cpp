#include <radio.hpp>
#include "cmsis_os.h"
#include "main.h" // Для доступу до hspi1 та пінів
#include <stdio.h>
#include <string.h>
#include <ui_feedback.hpp> // Для UI_Blink_Once()

// --- Глобальні об'єкти (визначені тут) ---
MyRadio g_radio;
// !!! ВИДАЛЕНО: osSemaphoreId_t g_radio_irq_sem; (Тепер створюється в freertos.c)

// --- Глобальні об'єкти (визначені в інших файлах) ---
extern SPI_HandleTypeDef hspi1; // Хендл SPI (з main.c)

// --- Адреси ---
uint8_t TX_ADDRESS[5] = {0xEE, 0xDD, 0xCC, 0xBB, 0xAA};

// --- C-Wrappers (Вхідні точки) ---
extern "C" {

// Оголошуємо хендли, які створені у freertos.c
extern osMessageQueueId_t radioTxQueueHandleHandle;
extern osSemaphoreId_t radioIrqSemHandleHandle; // <<< ПРАВИЛЬНЕ ІМ'Я З MX

/**
 * @brief C-функція для ініціалізації радіо.
 * (Тепер порожня, оскільки семафор створюється в MX)
 */
void radio_init(void)
{
    // !!! ВИДАЛЕНО: osSemaphoreNew(...)
    // CubeMX тепер робить це у MX_FREERTOS_Init()
}

/**
 * @brief Вхідна точка задачі RTOS.
 */
void radio_run_task(void)
{
    g_radio.task();
}

} // extern "C"

// --- C++ Class Implementation ---

MyRadio::MyRadio()
    // Ініціалізуємо наш C++ драйвер
    : radio(&hspi1,
    		NRF24_CSN_GPIO_Port, NRF24_CSN_Pin,
			NRF24_CE_GPIO_Port,  NRF24_CE_Pin,
            32)
{
    // Конструктор
}

/**
 * @brief Внутрішня ініціалізація (з вашого старого коду, але C++ методами)
 */
bool MyRadio::init(void)
{
    radio.init_tx(106, _1Mbps);
    radio.set_tx_address(TX_ADDRESS);
    radio.set_rx_address_p0(TX_ADDRESS); // Для Auto-ACK
    return true;
}


/**
 * @brief ГОЛОВНИЙ ЦИКЛ ЗАДАЧІ РАДІО
 * (Це вже ПОВНІСТЮ пересаджена логіка)
 */
void MyRadio::task(void)
{
    if (!this->init()) {
        vTaskDelete(NULL); // Помилка ініціалізації
    }

    uint8_t local_tx_buffer[32]; // Локальний буфер

    while(1)
    {
        // --- 1. Чекаємо на ДАНІ від LogicTask ---
    	if (osMessageQueueGet(radioTxQueueHandleHandle, local_tx_buffer, NULL, osWaitForever) == osOK)
        {
            // Є робота! Дані в local_tx_buffer.

            // 1. Завантажуємо дані в FIFO
            radio.transmit(local_tx_buffer);

            // 2. Починаємо передачу імпульсом CE
            radio.ce_high();
            osDelay(1); // 1мс (більше ніж 10us, безпечно)
            radio.ce_low();

            // 3. Чекаємо на IRQ (використовуємо ПРАВИЛЬНЕ ім'я семафора)
            if (osSemaphoreAcquire(radioIrqSemHandleHandle, 100) == osOK)
            {
                // IRQ спрацював!
                UI_Blink_Once();

                // 4. Обробимо IRQ (скинемо прапори)
                radio.handle_tx_irq();
            }
            else
            {
                // IRQ не спрацював (Timeout).
                radio.flush_tx_fifo();
            }
        }
    }
}
