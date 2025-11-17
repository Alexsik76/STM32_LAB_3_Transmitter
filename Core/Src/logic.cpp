/*
 * logic.cpp
 *
 * Created on: Nov 17, 2025
 * Author: osiko
 */

#include "logic.hpp"

#include <messaging.h> // <<< Наш файл з командами для дисплея
#include "cmsis_os.h"
#include <string.h>
#include <stdio.h> // для snprintf

// --- Глобальний об'єкт ---
LogicTask g_logic_task;

// --- Глобальні хендли черг з freertos.c ---


// --- C-Wrappers (Вхідні точки) ---
extern "C" {
// Оголошуємо хендли, які створені у freertos.c
extern osMessageQueueId_t keyEventQueueHandleHandle;
extern osMessageQueueId_t displayQueueHandleHandle;
extern osMessageQueueId_t radioTxQueueHandleHandle;

/**
 * @brief Вхідна точка задачі RTOS.
 * Це ім'я ("logic_task_entry") ви вказали в CubeMX.
 */
void logic_run_task(void)
{
    g_logic_task.task(); // Передаємо керування C++ об'єкту
}

} // extern "C"

// --- C++ Class Implementation ---

LogicTask::LogicTask()
{
    // Встановлюємо початковий режим
    this->current_mode = MODE_NORMAL_KEYS;
}

/**
 * @brief ГОЛОВНИЙ ЦИКЛ ЗАДАЧІ ЛОГІКИ
 */
void LogicTask::task(void)
{
    // Буфер для отримання символу з черги клавіатури
    char received_key;

    // Початкове повідомлення на дисплей
    send_display_status("Mode: Normal");
    send_display_main("Ready.");

    while (1)
    {
        // "Заснути" і чекати на натискання клавіші
    	if (osMessageQueueGet(keyEventQueueHandleHandle, &received_key, NULL, osWaitForever) == osOK)
        {
            // Ми прокинулись! Отримали клавішу.
            // Тепер передаємо її обробнику, що відповідає за поточний режим.

            switch (this->current_mode)
            {
                case MODE_NORMAL_KEYS:
                    handle_mode_normal_keys(received_key);
                    break;

                case MODE_SERVO_ARROWS:
                    handle_mode_servo_arrows(received_key);
                    break;

                case MODE_SEND_PRESET_ABC:
                    handle_mode_send_preset(received_key);
                    break;
            }
        }
    }
}

// --- Реалізація обробників режимів ---
// (Ця логіка реалізує 3 режими, які ми обговорювали)

void LogicTask::handle_mode_normal_keys(char key)
{
    // У цьому режимі ми передаємо всі клавіші
	send_display_main("");
    // 1. Команда для дисплея
    send_display_key(key);

    // 2. Команда для радіо (наприклад, пакет "K:[key]")
    uint8_t radio_packet[32];
    snprintf((char*)radio_packet, 32, "K:%c", key);
    send_radio_packet(radio_packet, 32);

    // 3. Перевірка на зміну режиму
    if (key == '#') {
        this->current_mode = MODE_SERVO_ARROWS;
        send_display_status("Mode: Servo");
        send_display_main("Arrows (2,4,6,8)");
    } else if (key == '*') {
        this->current_mode = MODE_SEND_PRESET_ABC;
        send_display_status("Mode: Send ABC");
        send_display_main("Press ANY key...");
    }
}

void LogicTask::handle_mode_servo_arrows(char key)
{
    uint8_t radio_packet[32] = {0};
    bool should_send = false;

    // 1. Перевіряємо, чи це "наша" клавіша
    switch (key)
    {
        case '2': // Вгору
            snprintf((char*)radio_packet, 32, "SRV:UP");
            should_send = true;
            break;
        case '8': // Вниз
            snprintf((char*)radio_packet, 32, "SRV:DOWN");
            should_send = true;
            break;
        case '4': // Вліво
            snprintf((char*)radio_packet, 32, "SRV:LEFT");
            should_send = true;
            break;
        case '6': // Вправо
            snprintf((char*)radio_packet, 32, "SRV:RIGHT");
            should_send = true;
            break;
    }

    // 2. Якщо так - відправляємо команду на радіо
    if (should_send) {
        send_display_main((char*)radio_packet); // Покажемо на дисплеї "SRV:UP"
        send_radio_packet(radio_packet, 32);
    }

    // 3. Перевірка на вихід з режиму (наприклад, '#')
    if (key == '#') {
        this->current_mode = MODE_NORMAL_KEYS;
        send_display_status("Mode: Normal");
        send_display_main("Ready.");
    }
}

void LogicTask::handle_mode_send_preset(char key)
{
    // У цьому режимі ми ігноруємо, *яку* клавішу натиснули
    // (окрім клавіші виходу)

    // 1. Команда для дисплея
    send_display_main("Sending 'Abc'...");

    // 2. Команда для радіо
    uint8_t radio_packet[32];
    snprintf((char*)radio_packet, 32, "Abc");
    send_radio_packet(radio_packet, 32);

    // 3. Автоматично повертаємось у головний режим
    this->current_mode = MODE_NORMAL_KEYS;
    send_display_status("Mode: Normal");
    // (send_display_main не кличемо, хай "Sending..." повисить)
}


// --- Реалізація "відправників" ---

void LogicTask::send_display_status(const char* text)
{
    DisplayMessage_t msg;
    msg.command = DISPLAY_CMD_SET_STATUS;
    strncpy(msg.text, text, 32);
    osMessageQueuePut(displayQueueHandleHandle, &msg, 0, 0);
}

void LogicTask::send_display_main(const char* text)
{
    DisplayMessage_t msg;
    msg.command = DISPLAY_CMD_SET_MAIN;
    strncpy(msg.text, text, 32);
    osMessageQueuePut(displayQueueHandleHandle, &msg, 0, 0);
}

void LogicTask::send_display_key(char key)
{
    DisplayMessage_t msg;
    msg.command = DISPLAY_CMD_SHOW_KEY;
    msg.key = key;
    osMessageQueuePut(displayQueueHandleHandle, &msg, 0, 0);
}

void LogicTask::send_radio_packet(uint8_t* data, uint8_t len)
{

	osMessageQueuePut(radioTxQueueHandleHandle, data, 0, 0);
}
