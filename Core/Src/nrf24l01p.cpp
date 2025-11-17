#include "nrf24l01p.hpp"
#include <string.h> // для memset

// #############################################################################
// ## Конструктор
// #############################################################################

Nrf24l01p::Nrf24l01p(SPI_HandleTypeDef* spi,
                     GPIO_TypeDef* cs_port, uint16_t cs_pin,
                     GPIO_TypeDef* ce_port, uint16_t ce_pin,
                     uint8_t payload_len)
{
    // Зберігаємо всі вказівники та налаштування
    this->spi_handle = spi;
    this->csn_port = cs_port;
    this->csn_pin = cs_pin;
    this->ce_port = ce_port;
    this->ce_pin = ce_pin;
    this->payload_length = payload_len;
}

// #############################################################################
// ## Публічні методи (API)
// #############################################################################

void Nrf24l01p::init_rx(channel MHz, air_data_rate bps)
{
    this->reset();
    this->set_prx_mode();
    this->power_up();
    this->set_rx_payload_widths(this->payload_length);
    this->set_rf_channel(MHz);
    this->set_rf_air_data_rate(bps);
    this->set_rf_tx_output_power(_0dBm);
    this->set_crc_length(1);
    this->set_address_widths(5);
    this->set_auto_retransmit_count(3);
    this->set_auto_retransmit_delay(250);
    this->ce_high(); // Вмикаємо прийом
}

void Nrf24l01p::init_tx(channel MHz, air_data_rate bps)
{
    this->reset();
    this->set_ptx_mode();
    this->power_up();
    this->set_rf_channel(MHz);
    this->set_rf_air_data_rate(bps);
    this->set_rf_tx_output_power(_0dBm);
    this->set_crc_length(1);
    this->set_address_widths(5);
    this->set_auto_retransmit_count(3);
    this->set_auto_retransmit_delay(250);
    this->ce_low();
}

void Nrf24l01p::receive(uint8_t* rx_payload)
{
    this->read_rx_fifo(rx_payload);
    this->clear_rx_dr();
}

void Nrf24l01p::transmit(uint8_t* tx_payload)
{
    this->write_tx_fifo(tx_payload);

}

void Nrf24l01p::handle_tx_irq()
{
    uint8_t status = this->get_status();

    if (status & (1 << 5)) // TX_DS (Data Sent)
    {
        this->clear_tx_ds();
    }

    if (status & (1 << 4)) // MAX_RT (Max Retries)
    {
        this->clear_max_rt();
        this->flush_tx_fifo(); // Очистити FIFO, якщо не вдалося
    }
}

void Nrf24l01p::set_tx_address(uint8_t* address)
{
    write_register_multi(NRF24L01P_REG_TX_ADDR, address, 5);
}

void Nrf24l01p::set_rx_address_p0(uint8_t* address)
{
    write_register_multi(NRF24L01P_REG_RX_ADDR_P0, address, 5);
}

void Nrf24l01p::power_up()
{
    uint8_t new_config = read_register(NRF24L01P_REG_CONFIG);
    new_config |= (1 << 1); // Set PWR_UP bit
    write_register(NRF24L01P_REG_CONFIG, new_config);
    HAL_Delay(2); // Затримка на стабілізацію
}

void Nrf24l01p::power_down()
{
    uint8_t new_config = read_register(NRF24L01P_REG_CONFIG);
    new_config &= ~(1 << 1); // Clear PWR_UP bit
    write_register(NRF24L01P_REG_CONFIG, new_config);
}

void Nrf24l01p::flush_rx_fifo()
{
    uint8_t command = NRF24L01P_CMD_FLUSH_RX;
    uint8_t status;

    this->cs_low();
    HAL_SPI_TransmitReceive(this->spi_handle, &command, &status, 1, 2000);
    this->cs_high();
}

void Nrf24l01p::flush_tx_fifo()
{
    uint8_t command = NRF24L01P_CMD_FLUSH_TX;
    uint8_t status;

    this->cs_low();
    HAL_SPI_TransmitReceive(this->spi_handle, &command, &status, 1, 2000);
    this->cs_high();
}

uint8_t Nrf24l01p::get_status()
{
    uint8_t command = NRF24L01P_CMD_NOP;
    uint8_t status;

    this->cs_low();
    HAL_SPI_TransmitReceive(this->spi_handle, &command, &status, 1, 2000);
    this->cs_high();

    return status;
}

uint8_t Nrf24l01p::get_fifo_status()
{
    return read_register(NRF24L01P_REG_FIFO_STATUS);
}

void Nrf24l01p::clear_rx_dr()
{
    // Очищуємо біт RX_DR (біт 6)
    write_register(NRF24L01P_REG_STATUS, (1 << 6));
}

void Nrf24l01p::clear_tx_ds()
{
    // Очищуємо біт TX_DS (біт 5)
    write_register(NRF24L01P_REG_STATUS, (1 << 5));
}

void Nrf24l01p::clear_max_rt()
{
    // Очищуємо біт MAX_RT (біт 4)
    write_register(NRF24L01P_REG_STATUS, (1 << 4));
}


// #############################################################################
// ## Приватні методи (Керування пінами та SPI)
// #############################################################################

void Nrf24l01p::cs_high()
{
    HAL_GPIO_WritePin(this->csn_port, this->csn_pin, GPIO_PIN_SET);
}

void Nrf24l01p::cs_low()
{
    HAL_GPIO_WritePin(this->csn_port, this->csn_pin, GPIO_PIN_RESET);
}

void Nrf24l01p::ce_high()
{
    HAL_GPIO_WritePin(this->ce_port, this->ce_pin, GPIO_PIN_SET);
}

void Nrf24l01p::ce_low()
{
    HAL_GPIO_WritePin(this->ce_port, this->ce_pin, GPIO_PIN_RESET);
}

uint8_t Nrf24l01p::read_register(uint8_t reg)
{
    uint8_t command = NRF24L01P_CMD_R_REGISTER | reg;
    uint8_t status;
    uint8_t read_val;

    this->cs_low();
    HAL_SPI_TransmitReceive(this->spi_handle, &command, &status, 1, 2000);
    HAL_SPI_Receive(this->spi_handle, &read_val, 1, 2000);
    this->cs_high();

    return read_val;
}

uint8_t Nrf24l01p::write_register(uint8_t reg, uint8_t value)
{
    uint8_t command = NRF24L01P_CMD_W_REGISTER | reg;
    uint8_t status;
    uint8_t write_val = value;

    this->cs_low();
    HAL_SPI_TransmitReceive(this->spi_handle, &command, &status, 1, 2000);
    HAL_SPI_Transmit(this->spi_handle, &write_val, 1, 2000);
    this->cs_high();

    return write_val; // У вашому C-коді поверталося 'write_val', хоча логічніше 'status'
}

void Nrf24l01p::write_register_multi(uint8_t reg, uint8_t* value, uint8_t len)
{
    uint8_t command = NRF24L01P_CMD_W_REGISTER | reg;
    uint8_t status;

    this->cs_low();
    HAL_SPI_TransmitReceive(this->spi_handle, &command, &status, 1, 2000);
    HAL_SPI_Transmit(this->spi_handle, value, len, 2000);
    this->cs_high();
}


// #############################################################################
// ## Приватні методи (Внутрішня логіка, перенесена з .c)
// #############################################################################

void Nrf24l01p::reset()
{
    // Reset pins
    this->cs_high();
    this->ce_low();
    HAL_Delay(5); // Дамо чіпу час на запуск

    // Reset registers
    write_register(NRF24L01P_REG_CONFIG, 0x08);
    write_register(NRF24L01P_REG_EN_AA, 0x3F);
    write_register(NRF24L01P_REG_EN_RXADDR, 0x03);
    write_register(NRF24L01P_REG_SETUP_AW, 0x03);
    write_register(NRF24L01P_REG_SETUP_RETR, 0x03);
    write_register(NRF24L01P_REG_RF_CH, 0x02);
    write_register(NRF24L01P_REG_RF_SETUP, 0x07); // 0dBm, 1Mbps
    write_register(NRF24L01P_REG_STATUS, 0x7E);  // Очистити всі IRQ
    write_register(NRF24L01P_REG_RX_PW_P0, 0x00);
    write_register(NRF24L01P_REG_RX_PW_P1, 0x00);
    write_register(NRF24L01P_REG_RX_PW_P2, 0x00);
    write_register(NRF24L01P_REG_RX_PW_P3, 0x00);
    write_register(NRF24L01P_REG_RX_PW_P4, 0x00);
    write_register(NRF24L01P_REG_RX_PW_P5, 0x00);
    write_register(NRF24L01P_REG_FIFO_STATUS, 0x11);
    write_register(NRF24L01P_REG_DYNPD, 0x00);
    write_register(NRF24L01P_REG_FEATURE, 0x00);

    // Reset FIFO
    this->flush_rx_fifo();
    this->flush_tx_fifo();

    // Очистити прапорці IRQ
    clear_rx_dr();
    clear_tx_ds();
    clear_max_rt();
}

void Nrf24l01p::set_prx_mode()
{
    uint8_t new_config = read_register(NRF24L01P_REG_CONFIG);
    new_config |= (1 << 0); // Set PRIM_RX bit
    write_register(NRF24L01P_REG_CONFIG, new_config);
}

void Nrf24l01p::set_ptx_mode()
{
    uint8_t new_config = read_register(NRF24L01P_REG_CONFIG);
    new_config &= ~(1 << 0); // Clear PRIM_RX bit
    write_register(NRF24L01P_REG_CONFIG, new_config);
}

uint8_t Nrf24l01p::read_rx_fifo(uint8_t* rx_payload)
{
    uint8_t command = NRF24L01P_CMD_R_RX_PAYLOAD;
    uint8_t status;

    this->cs_low();
    HAL_SPI_TransmitReceive(this->spi_handle, &command, &status, 1, 2000);
    // Використовуємо this->payload_length замість #define
    HAL_SPI_Receive(this->spi_handle, rx_payload, this->payload_length, 2000);
    this->cs_high();

    return status;
}

uint8_t Nrf24l01p::write_tx_fifo(uint8_t* tx_payload)
{
    uint8_t command = NRF24L01P_CMD_W_TX_PAYLOAD;
    uint8_t status;

    this->cs_low();
    HAL_SPI_TransmitReceive(this->spi_handle, &command, &status, 1, 2000);
    // Використовуємо this->payload_length замість #define
    HAL_SPI_Transmit(this->spi_handle, tx_payload, this->payload_length, 2000);
    this->cs_high();

    return status;
}

void Nrf24l01p::set_rx_payload_widths(widths bytes)
{
    // Ми використовуємо лише P0
    write_register(NRF24L01P_REG_RX_PW_P0, bytes);
}

void Nrf24l01p::set_crc_length(length bytes)
{
    uint8_t new_config = read_register(NRF24L01P_REG_CONFIG);

    switch(bytes)
    {
        case 1:
            new_config &= ~(1 << 3); // Clear EN_CRC
            new_config &= ~(1 << 2); // Clear CRCO
            break;
        case 2:
            new_config |= (1 << 3);  // Set EN_CRC
            new_config |= (1 << 2);  // Set CRCO
            break;
    }
    write_register(NRF24L01P_REG_CONFIG, new_config);
}

void Nrf24l01p::set_address_widths(widths bytes)
{
    // Кодування: 0b01 = 3 байти, 0b10 = 4 байти, 0b11 = 5 байт
    if(bytes >= 3 && bytes <= 5)
        write_register(NRF24L01P_REG_SETUP_AW, bytes - 2);
}

void Nrf24l01p::set_auto_retransmit_count(count cnt)
{
    uint8_t new_setup_retr = read_register(NRF24L01P_REG_SETUP_RETR);

    cnt &= 0x0F; // Максимум 15
    new_setup_retr &= 0xF0; // Очистити старі біти ARC
    new_setup_retr |= cnt;  // Встановити нові

    write_register(NRF24L01P_REG_SETUP_RETR, new_setup_retr);
}

void Nrf24l01p::set_auto_retransmit_delay(delay us)
{
    uint8_t new_setup_retr = read_register(NRF24L01P_REG_SETUP_RETR);

    // Кодування: 0000 = 250us, 0001 = 500us ... 1111 = 4000us
    uint8_t delay_code = (us / 250) - 1;
    if (us < 250) delay_code = 0;
    if (delay_code > 0x0F) delay_code = 0x0F; // Максимум 4000us

    new_setup_retr &= 0x0F; // Очистити старі біти ARD
    new_setup_retr |= (delay_code << 4); // Встановити нові

    write_register(NRF24L01P_REG_SETUP_RETR, new_setup_retr);
}

void Nrf24l01p::set_rf_channel(channel MHz)
{
    if (MHz > 125) MHz = 125;
    write_register(NRF24L01P_REG_RF_CH, MHz);
}

void Nrf24l01p::set_rf_tx_output_power(output_power dBm)
{
    uint8_t new_rf_setup = read_register(NRF24L01P_REG_RF_SETUP);

    new_rf_setup &= ~((1<<2) | (1<<1)); // Очистити біти RF_PWR
    new_rf_setup |= (dBm << 1);         // Встановити нові

    write_register(NRF24L01P_REG_RF_SETUP, new_rf_setup);
}

void Nrf24l01p::set_rf_air_data_rate(air_data_rate bps)
{
    uint8_t new_rf_setup = read_register(NRF24L01P_REG_RF_SETUP);

    // Очистити біти data rate
    new_rf_setup &= ~((1 << 5) | (1 << 3));

    switch(bps)
    {
        case _1Mbps:
            // 0 0
            break;
        case _2Mbps:
            new_rf_setup |= (1 << 3); // 0 1
            break;
        case _250kbps:
            new_rf_setup |= (1 << 5); // 1 0
            break;
    }
    write_register(NRF24L01P_REG_RF_SETUP, new_rf_setup);
}
