#include "nrf24l01.h"
#include "spi.h" // We need this for the HAL_SPI functions
#include <string.h>

// --- Defines for NRF24L01 Registers ---
#define NRF24_REG_CONFIG      0x00
#define NRF24_REG_EN_AA       0x01
#define NRF24_REG_EN_RXADDR   0x02
#define NRF24_REG_SETUP_AW    0x03
#define NRF24_REG_SETUP_RETR  0x04
#define NRF24_REG_RF_CH       0x05
#define NRF24_REG_RF_SETUP    0x06
#define NRF24_REG_STATUS      0x07
#define NRF24_REG_RX_ADDR_P0  0x0A
#define NRF24_REG_TX_ADDR     0x10
#define NRF24_REG_RX_PW_P0    0x11
#define NRF24_REG_FIFO_STATUS 0x17
#define NRF24_REG_DYNPD       0x1C
#define NRF24_REG_FEATURE     0x1D

// --- Defines for NRF24L01 Commands ---
#define NRF24_CMD_R_REGISTER    0x00
#define NRF24_CMD_W_REGISTER    0x20
#define NRF24_CMD_R_RX_PAYLOAD  0x61
#define NRF24_CMD_W_TX_PAYLOAD  0xA0
#define NRF24_CMD_FLUSH_TX      0xE1
#define NRF24_CMD_FLUSH_RX      0xE2
#define NRF24_CMD_NOP           0xFF

// --- Private SPI Functions ---

/**
 * @brief  Sends one byte to the nRF24L01.
 * @param  data: The byte to send.
 * @return The status byte returned from the nRF24L01.
 */
static uint8_t NRF24_SPI_Transmit(uint8_t data)
{
    uint8_t rx_data;
    // We must use HAL_SPI_TransmitReceive to get the STATUS byte back
    HAL_SPI_TransmitReceive(&hspi1, &data, &rx_data, 1, HAL_MAX_DELAY);
    return rx_data;
}

/**
 * @brief  Writes a single byte to a specific register.
 * @param  reg: The register address.
 * @param  data: The byte to write.
 */
static void NRF24_WriteRegister(uint8_t reg, uint8_t data)
{
    uint8_t buf[2];
    buf[0] = NRF24_CMD_W_REGISTER | reg; // Command + Register Address
    buf[1] = data;                      // Data

    NRF24_CSN_Enable(); // CSN Low (Select chip)
    HAL_SPI_Transmit(&hspi1, buf, 2, HAL_MAX_DELAY);
    NRF24_CSN_Disable(); // CSN High (Deselect chip)
}

/**
 * @brief  Writes multiple bytes to a register.
 * @param  reg: The register address.
 * @param  data: Pointer to the data buffer.
 * @param  size: Number of bytes to write.
 */
static void NRF24_WriteRegisterMulti(uint8_t reg, uint8_t* data, uint8_t size)
{
    uint8_t buf[1];
    buf[0] = NRF24_CMD_W_REGISTER | reg; // Command + Register Address

    NRF24_CSN_Enable();
    HAL_SPI_Transmit(&hspi1, buf, 1, HAL_MAX_DELAY);      // Send write command
    HAL_SPI_Transmit(&hspi1, data, size, HAL_MAX_DELAY); // Send data
    NRF24_CSN_Disable();
}

/**
 * @brief  Reads a single byte from a specific register.
 * @param  reg: The register address.
 * @return The byte read from the register.
 */
static uint8_t NRF24_ReadRegister(uint8_t reg)
{
    uint8_t buf[2];
    buf[0] = NRF24_CMD_R_REGISTER | reg; // Command + Register Address
    buf[1] = NRF24_CMD_NOP;          // Dummy byte to clock data out

    NRF24_CSN_Enable();
    // We must use TransmitReceive to get the data back
    HAL_SPI_TransmitReceive(&hspi1, buf, buf, 2, HAL_MAX_DELAY);
    NRF24_CSN_Disable();

    return buf[1]; // The second byte is the data
}

/**
 * @brief  Reads multiple bytes from a register.
 * @param  reg: The register address.
 * @param  data: Pointer to the buffer to store the data.
 * @param  size: Number of bytes to read.
 */
static void NRF24_ReadRegisterMulti(uint8_t reg, uint8_t* data, uint8_t size)
{
    uint8_t buf[1];
    buf[0] = NRF24_CMD_R_REGISTER | reg; // Command + Register Address

    NRF24_CSN_Enable();
    HAL_SPI_Transmit(&hspi1, buf, 1, HAL_MAX_DELAY);    // Send read command
    HAL_SPI_Receive(&hspi1, data, size, HAL_MAX_DELAY); // Receive data
    NRF24_CSN_Disable();
}

/**
 * @brief  Sends a command to the nRF24L01.
 * @param  cmd: The command to send.
 */
static void NRF24_SendCommand(uint8_t cmd)
{
    NRF24_CSN_Enable();
    NRF24_SPI_Transmit(cmd);
    NRF24_CSN_Disable();
}

// --- Public GPIO Functions ---

/**
 * @brief  Pulls the Chip Enable (CE) pin LOW.
 */
void NRF24_CE_Disable(void)
{
    HAL_GPIO_WritePin(NRF24_CE_GPIO_Port, NRF24_CE_Pin, GPIO_PIN_RESET);
}

/**
 * @brief  Pulls the Chip Enable (CE) pin HIGH.
 */
void NRF24_CE_Enable(void)
{
    HAL_GPIO_WritePin(NRF24_CE_GPIO_Port, NRF24_CE_Pin, GPIO_PIN_SET);
}

/**
 * @brief  Pulls the Chip Select Not (CSN) pin LOW (activates SPI).
 */
void NRF24_CSN_Enable(void)
{
    HAL_GPIO_WritePin(NRF24_CSN_GPIO_Port, NRF24_CSN_Pin, GPIO_PIN_RESET);
}

/**
 * @brief  Pulls the Chip Select Not (CSN) pin HIGH (deactivates SPI).
 */
void NRF24_CSN_Disable(void)
{
    HAL_GPIO_WritePin(NRF24_CSN_GPIO_Port, NRF24_CSN_Pin, GPIO_PIN_SET);
}

// --- Public Configuration Functions ---

/**
 * @brief  Initializes the nRF24L01 with default settings.
 * @return 1 if init is OK, 0 if init failed (SPI check failed).
 */
uint8_t NRF24_Init(void)
{
    // Set default pin states
    NRF24_CE_Disable();  // CE Low (Standby-I mode)
    NRF24_CSN_Disable(); // CSN High (SPI inactive)

    // Wait for the radio to power up
    HAL_Delay(100);

    // --- Check SPI connection by writing to a known register ---
    uint8_t test_addr[5] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE};
    uint8_t read_addr[5];
    NRF24_WriteRegisterMulti(NRF24_REG_TX_ADDR, test_addr, 5);
    NRF24_ReadRegisterMulti(NRF24_REG_TX_ADDR, read_addr, 5);
    if (memcmp(test_addr, read_addr, 5) != 0) {
        return 0; // SPI connection failed
    }

    // --- Apply Default Settings ---
    NRF24_WriteRegister(NRF24_REG_CONFIG, 0x0C);      // Enable CRC (2 bytes), Power UP, PRX
    NRF24_WriteRegister(NRF24_REG_EN_AA, 0x01);       // Enable Auto-Ack on Pipe 0
    NRF24_WriteRegister(NRF24_REG_EN_RXADDR, 0x01);   // Enable RX Pipe 0
    NRF24_WriteRegister(NRF24_REG_SETUP_AW, 0x03);    // 5-byte address width
    NRF24_WriteRegister(NRF24_REG_SETUP_RETR, 0x03);  // 15 retries, 250us delay
    NRF24_WriteRegister(NRF24_REG_RF_CH, 0x6A);       // Channel 106 (2.400 + 0.106 = 2.506 GHz)
    NRF24_WriteRegister(NRF24_REG_RF_SETUP, 0x06);    // 1Mbps, 0dBm power
    NRF24_WriteRegister(NRF24_REG_RX_PW_P0, 32);      // 32-byte payload for Pipe 0

    // Enable Dynamic Payloads and ACK Payloads (for non-blocking)
    NRF24_WriteRegister(NRF24_REG_FEATURE, 0x06);
    NRF24_WriteRegister(NRF24_REG_DYNPD, 0x01);

    // Clear any pending interrupts
    NRF24_ClearInterrupts();

    // Flush FIFOs
    NRF24_SendCommand(NRF24_CMD_FLUSH_TX);
    NRF24_SendCommand(NRF24_CMD_FLUSH_RX);

    return 1; // Success
}

/**
 * @brief  Sets the RF Channel (frequency).
 * @param  channel: 0-125 (2.400GHz to 2.525GHz).
 */
void NRF24_SetRFChannel(uint8_t channel)
{
    if (channel > 125) channel = 125;
    NRF24_WriteRegister(NRF24_REG_RF_CH, channel);
}

/**
 * @brief  Sets the Transmit (TX) address.
 * @param  addr: 5-byte address buffer.
 */
void NRF24_SetTXAddress(uint8_t *addr)
{
    NRF24_WriteRegisterMulti(NRF24_REG_TX_ADDR, addr, 5);
}

/**
 * @brief  Sets the Receive (RX) address for Pipe 0.
 * @param  addr: 5-byte address buffer.
 */
void NRF24_SetRXAddress(uint8_t *addr)
{
    NRF24_WriteRegisterMulti(NRF24_REG_RX_ADDR_P0, addr, 5);
}

/**
 * @brief  Puts the nRF24L01 into Transmit (TX) mode.
 */
void NRF24_TXMode(void)
{
    NRF24_CE_Disable(); // Go to Standby-I

    // Set CONFIG register: Power UP, CRC 2-byte, PTX (Transmit mode)
    uint8_t config = NRF24_ReadRegister(NRF24_REG_CONFIG);
    config = (config & 0xFE) | 0x08; // Clear PRIM_RX bit, Set PWR_UP
    NRF24_WriteRegister(NRF24_REG_CONFIG, config);

    // Note: CE is NOT enabled here. Transmit() will pulse it.
}

/**
 * @brief  Puts the nRF24L01 into Receive (RX) mode.
 */
void NRF24_RXMode(void)
{
    NRF24_CE_Disable(); // Go to Standby-I

    // Set CONFIG register: Power UP, CRC 2-byte, PRX (Receive mode)
    uint8_t config = NRF24_ReadRegister(NRF24_REG_CONFIG);
    config = config | 0x09; // Set PRIM_RX bit, Set PWR_UP
    NRF24_WriteRegister(NRF24_REG_CONFIG, config);

    NRF24_CE_Enable(); // Go to RX mode (listen)
}

/**
 * @brief  Transmits a 32-byte data packet (blocking).
 * @param  data: 32-byte buffer to send.
 * @return The STATUS register value.
 */
uint8_t NRF24_Transmit(uint8_t *data)
{
    uint8_t status;

    // 1. Write payload to TX FIFO
    uint8_t cmd = NRF24_CMD_W_TX_PAYLOAD;
    NRF24_CSN_Enable();
    HAL_SPI_Transmit(&hspi1, &cmd, 1, HAL_MAX_DELAY);
    HAL_SPI_Transmit(&hspi1, data, 32, HAL_MAX_DELAY);
    NRF24_CSN_Disable();

    // 2. Pulse CE to start transmission
    NRF24_CE_Enable();
    HAL_Delay(1); // Minimum 10us pulse
    NRF24_CE_Disable();

    // 3. Wait for transmission to complete (IRQ pin will go LOW)
    //    (This is the blocking part we will fix with RTOS)
    //    For now, we just wait until the IRQ pin goes low.
    while(HAL_GPIO_ReadPin(NRF24_IRQ_GPIO_Port, NRF24_IRQ_Pin) == GPIO_PIN_SET) {
        // Wait
    }

    // 4. Get and clear the status
    status = NRF24_GetStatus();
    NRF24_ClearInterrupts();

    return status;
}

/**
 * @brief  Gets the STATUS register.
 * @return STATUS register value.
 */
uint8_t NRF24_GetStatus(void)
{
    // NOP command always returns STATUS register
    return NRF24_SPI_Transmit(NRF24_CMD_NOP);
}

/**
 * @brief  Checks if data is ready in RX FIFO.
 * @return 1 if data is ready, 0 otherwise.
 */
uint8_t NRF24_DataReady(void)
{
    uint8_t status = NRF24_GetStatus();
    // Check the RX_DR (Data Ready) bit in the STATUS register
    if (status & (1 << 6)) {
        return 1;
    }
    return 0;
}

/**
 * @brief  Reads the received 32-byte payload from RX FIFO.
 * @param  data: 32-byte buffer to store the received data.
 */
void NRF24_GetData(uint8_t *data)
{
    uint8_t cmd = NRF24_CMD_R_RX_PAYLOAD;

    // 1. Send "Read RX Payload" command
    NRF24_CSN_Enable();
    HAL_SPI_Transmit(&hspi1, &cmd, 1, HAL_MAX_DELAY);

    // 2. Read the 32-byte payload
    HAL_SPI_Receive(&hspi1, data, 32, HAL_MAX_DELAY);
    NRF24_CSN_Disable();

    // 3. Clear the RX_DR interrupt bit
    NRF24_WriteRegister(NRF24_REG_STATUS, (1 << 6));
}

/**
 * @brief  Clears all interrupt flags in the STATUS register.
 */
void NRF24_ClearInterrupts(void)
{
    // Write '1' to clear RX_DR, TX_DS, and MAX_RT interrupt flags
    NRF24_WriteRegister(NRF24_REG_STATUS, 0x70);
}
