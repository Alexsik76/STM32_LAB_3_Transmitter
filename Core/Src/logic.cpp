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
    char key;

    // Початкове оновлення екрану передавача
    update_local_display();

    while (1)
    {
        // Чекаємо натискання клавіші
        if (osMessageQueueGet(keyEventQueueHandleHandle, &key, NULL, osWaitForever) == osOK)
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
				memset(sync_packet.payload, 0, 31); // Даних немає
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

                case MODE_SERVO:
                    // Шлемо тільки команди керування
                    if (key == '2' || key == '4' || key == '6' || key == '8') {
                        packet.payload[0] = key;
                        send_it = true;

                        // Локальна візуалізація
                        if (key == '2') send_to_display(DISP_CMD_SET_MAIN_TEXT, "CMD: UP");
                        if (key == '8') send_to_display(DISP_CMD_SET_MAIN_TEXT, "CMD: DOWN");
                        if (key == '4') send_to_display(DISP_CMD_SET_MAIN_TEXT, "CMD: LEFT");
                        if (key == '6') send_to_display(DISP_CMD_SET_MAIN_TEXT, "CMD: RIGHT");
                    }
                    break;

                case MODE_AUTO:
                    // Будь-яка клавіша відправляє послідовність
                	strncpy(packet.payload, "Abc sequence", 30);
					send_it = true;
					send_to_display(DISP_CMD_SET_MAIN_TEXT, "Sent!");
                    break;
            }

            // --- 3. ВІДПРАВКА В ЕФІР ---
            if (send_it) {
                osMessageQueuePut(radioTxQueueHandleHandle, &packet, 0, 0);
            }
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
			send_to_display(DISP_CMD_SET_MAIN_TEXT, "Keys: 2,4,6,8");
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
