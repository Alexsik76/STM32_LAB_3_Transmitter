#include "logic.hpp"
#include "rtos_tasks.h" // Хендли черг
#include "string.h"
#include "stdio.h"
#include "cmsis_os.h"
// Глобальний об'єкт
LogicTask g_logic_task;

extern osMessageQueueId_t keyEventQueueHandleHandle;
extern osMessageQueueId_t radioTxQueueHandleHandle;
extern osMessageQueueId_t displayQueueHandleHandle;
extern ADC_HandleTypeDef hadc1;
uint32_t adc_buffer[2];
extern "C" {
    void logic_run_task(void) {
    	g_logic_task.task();
    }
}

LogicTask::LogicTask() {
    this->current_mode = MODE_KEYPAD;
}

void LogicTask::task(void)
{
	HAL_ADC_Start_DMA(&hadc1, adc_buffer, 2);
    char key;

    // Початкове оновлення екрану передавача
    update_local_display();

    while (1)
    {
        // Чекаємо натискання клавіші
        if (osMessageQueueGet(keyEventQueueHandleHandle, &key, NULL, 100) == osOK)
        {
            // --- 1. ЛОГІКА ПЕРЕМИКАННЯ РЕЖИМІВ (#) ---
            if (key == '#')
            {
            	if (this->current_mode == MODE_AUTO) this->current_mode = MODE_KEYPAD;
				else this->current_mode = (SystemMode_t)((int)this->current_mode + 1);

				update_local_display();

				// НОВЕ: Відправляємо "пустий" пакет, щоб Приймач одразу дізнався про зміну режиму
				RadioPacket sync_packet;
				sync_packet.mode = (uint8_t)this->current_mode;
				if (this->current_mode == MODE_SERVO) {
						// Для серво заповнюємо ЦЕНТРОМ (127), щоб не сіпались
						memset(sync_packet.payload, 127, 31);
					} else {
						// Для інших режимів - нулями
						memset(sync_packet.payload, 0, 31);
					}
				osMessageQueuePut(radioTxQueueHandleHandle, &sync_packet, 0, 0);

				continue;
            }

            // --- 2. ФОРМУВАННЯ ПАКЕТУ ---
            RadioPacket packet;
            memset(&packet, 0, sizeof(packet)); // Чистимо пам'ять
            packet.mode = (uint8_t)this->current_mode;

            bool send_it = false;

            switch (this->current_mode)
            {
                case MODE_KEYPAD:
                    // Шлемо будь-яку клавішу
                    packet.payload[0] = key;
                    send_it = true;

                    // Локально показуємо, що натиснули
                    send_to_display(DISP_CMD_SHOW_KEY, "", key);
                    break;
                case MODE_AUTO:
                    // Будь-яка клавіша відправляє послідовність
                	strncpy(packet.payload, "Abc sequence", 30);
					send_it = true;
					send_to_display(DISP_CMD_SET_MAIN_TEXT, "Sent!");
                    break;
                case MODE_SERVO:
				break;
            }

            // --- 3. ВІДПРАВКА В ЕФІР ---
            if (send_it) {
                osMessageQueuePut(radioTxQueueHandleHandle, &packet, 0, 0);
            }
        }
        if (this->current_mode == MODE_SERVO)
                {
                    // "Пам'ять" про минулий стан (ініціалізується лише раз)
                    static uint8_t last_x = 0;
                    static uint8_t last_y = 0;

                    // 1. Беремо свіжі дані
                    uint32_t raw_x = adc_buffer[0];
                    uint32_t raw_y = adc_buffer[1];

                    // 2. Обробка (з твоєю розширеною мертвою зоною)
                    auto process_axis = [](uint32_t raw_val) -> uint8_t {
                        if (raw_val > 1700 && raw_val < 2400) return 127;
                        uint32_t scaled = raw_val / 16;
                        if (scaled > 255) scaled = 255;
                        return (uint8_t)scaled;
                    };

                    uint8_t x_byte = process_axis(raw_x);
                    uint8_t y_byte = process_axis(raw_y);

                    // 3. ПЕРЕВІРКА: Чи змінилися дані?
                    if (x_byte != last_x || y_byte != last_y)
                    {
                        // Тільки якщо змінилися - формуємо і шлемо пакет
                        RadioPacket joy_packet;
                        joy_packet.mode = MODE_SERVO;
                        joy_packet.payload[0] = x_byte;
                        joy_packet.payload[1] = y_byte;

                        osMessageQueuePut(radioTxQueueHandleHandle, &joy_packet, 0, 0);

                        // Оновлюємо екран передавача
                        char buf[32];
                        snprintf(buf, sizeof(buf), "TX: %d %d", x_byte, y_byte);
                        send_to_display(DISP_CMD_SET_MAIN_TEXT, buf);

                        // Запам'ятовуємо нові значення як "старі"
                        last_x = x_byte;
                        last_y = y_byte;
                    }

                    // 4. Затримка
                    // Навіть якщо ми нічого не слали, треба почекати,
                    // щоб не опитувати АЦП мільйон разів на секунду.
                    osDelay(50);
                }
    }
}

// Оновлення статусу на екрані передавача
void LogicTask::update_local_display()
{
	switch (this->current_mode) {
		case MODE_KEYPAD:
			send_to_display(DISP_CMD_SET_STATUS, "Mode: Keypad");
			send_to_display(DISP_CMD_SET_MAIN_TEXT, "Ready");
			break;
		case MODE_SERVO:
			send_to_display(DISP_CMD_SET_STATUS, "Mode: Servo");
			send_to_display(DISP_CMD_SET_MAIN_TEXT, "2,4,6,8");
			break;
		case MODE_AUTO:
			send_to_display(DISP_CMD_SET_STATUS, "Mode: Auto");
			send_to_display(DISP_CMD_SET_MAIN_TEXT, "Press Key");
			break;
	    }
}

// Допоміжна функція (така ж, як на приймачі)
void LogicTask::send_to_display(DisplayCommand_t cmd, const char* text, char key) {
    DisplayMessage_t msg;
    msg.command = cmd;
    if (text) strncpy(msg.text, text, 31);
    else msg.text[0] = 0;
    msg.key = key;
    osMessageQueuePut(displayQueueHandleHandle, &msg, 0, 0);
}
