#ifndef RADIO_PROTOCOL_H
#define RADIO_PROTOCOL_H

#include <stdint.h>

#define RADIO_PAYLOAD_SIZE 32

// Режими системи (спільні для TX та RX)
typedef enum {
    MODE_KEYPAD = 0,    // Режим 1: Трансляція клавіш
    MODE_SERVO,         // Режим 2: Керування сервоприводами (2,4,6,8)
    MODE_AUTO           // Режим 3: Передача "Abc"
} SystemMode_t;

// Структура пакету
typedef struct {
    uint8_t mode;                 // SystemMode_t (1 байт)
    char    payload[31];          // Дані (символ клавіші або рядок)
} RadioPacket;

#endif