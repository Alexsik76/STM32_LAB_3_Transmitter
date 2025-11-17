#pragma once

#include "main.h" // Для HAL_SPI... та GPIO
#include "nrf24l01p.h" // Ваш оригінальний .h файл з усіма #define NRF24L01P_... (регістрами) та typedefs

class Nrf24l01p
{
public:
    // ## Публічний API ##

    /**
     * @brief Конструктор.
     * @param spi: Вказівник на HAL SPI handle (напр. &hspi1)
     * @param cs_port: Порт для CSN (Chip Select)
     * @param cs_pin: Пін для CSN
     * @param ce_port: Порт для CE (Chip Enable)
     * @param ce_pin: Пін для CE
     * @param payload_len: Довжина пакету (замість NRF24L01P_PAYLOAD_LENGTH)
     */
    Nrf24l01p(SPI_HandleTypeDef* spi,
              GPIO_TypeDef* cs_port, uint16_t cs_pin,
              GPIO_TypeDef* ce_port, uint16_t ce_pin,
              uint8_t payload_len);

    // Головні функції
    void init_rx(channel MHz, air_data_rate bps);
    void init_tx(channel MHz, air_data_rate bps);

    void receive(uint8_t* rx_payload);
    void transmit(uint8_t* tx_payload);
    void handle_tx_irq();

    // Налаштування
    void set_tx_address(uint8_t* address);
    void set_rx_address_p0(uint8_t* address);

    // Керування
    void power_up();
    void power_down();
    void flush_rx_fifo();
    void flush_tx_fifo();
    void ce_high();
    void ce_low();

    // Статус
    uint8_t get_status();
    uint8_t get_fifo_status();

    // Очистка прапорців
    void clear_rx_dr();
    void clear_tx_ds();
    void clear_max_rt();

private:
    // ## Приватні члени ##

    // Збережені налаштування
    SPI_HandleTypeDef* spi_handle;
    GPIO_TypeDef* csn_port;
    uint16_t           csn_pin;
    GPIO_TypeDef* ce_port;
    uint16_t           ce_pin;
    uint8_t            payload_length;

    // ## Приватні методи (колишні static C-функції) ##

    // Керування пінами
    void cs_high();
    void cs_low();


    // SPI комунікація
    uint8_t read_register(uint8_t reg);
    uint8_t write_register(uint8_t reg, uint8_t value);
    void write_register_multi(uint8_t reg, uint8_t* value, uint8_t len);

    // Внутрішні функції
    void reset();
    void set_prx_mode();
    void set_ptx_mode();
    uint8_t read_rx_fifo(uint8_t* rx_payload);
    uint8_t write_tx_fifo(uint8_t* tx_payload);
    void set_rx_payload_widths(widths bytes);
    void set_crc_length(length bytes);
    void set_address_widths(widths bytes);
    void set_auto_retransmit_count(count cnt);
    void set_auto_retransmit_delay(delay us);
    void set_rf_channel(channel MHz);
    void set_rf_tx_output_power(output_power dBm);
    void set_rf_air_data_rate(air_data_rate bps);
};
