#ifndef NRF24L01_H_
#define NRF24L01_H_

#include "main.h"

// --- C-Wrappers ---
#ifdef __cplusplus
extern "C" {
#endif

// Initialization
uint8_t NRF24_Init(void);

// Configuration
void NRF24_SetRFChannel(uint8_t channel);
void NRF24_SetTXAddress(uint8_t *addr);
void NRF24_SetRXAddress(uint8_t *addr);

// Modes
void NRF24_TXMode(void);
void NRF24_RXMode(void);

// Data Transmission/Reception
uint8_t NRF24_Transmit(uint8_t *data);
uint8_t NRF24_DataReady(void);
void NRF24_GetData(uint8_t *data);

// Low-level
void NRF24_CE_Enable(void);
void NRF24_CE_Disable(void);
void NRF24_CSN_Enable(void);
void NRF24_CSN_Disable(void);
uint8_t NRF24_GetStatus(void);
void NRF24_ClearInterrupts(void);


#ifdef __cplusplus
}
#endif

#endif /* NRF24L01_H_ */
