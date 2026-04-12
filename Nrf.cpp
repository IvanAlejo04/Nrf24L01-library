/*
 * Copyright (c) 2026 Ivan Alejo
 * Licensed under the MIT License.
 * See LICENSE file in the project root for full license information.
 */
// april 9 2026

#include "Nrf.h"
#include "main.h"
extern SPI_HandleTypeDef hspi1;

/* USER CODE BEGIN ET */
// extern SPI_HandleTypeDef hspi1;  - type this block of code the exter..... on your main
/* USER CODE END ET */

#define ON GPIO_PIN_SET
#define OFF GPIO_PIN_RESET

Nrf::Nrf(uint16_t CE, uint16_t CSN, GPIO_TypeDef *pinGroup) // GPIO_TypeDef* is like variable but it is a pointer for your gpio pin, you must use TypeDef instead of normal variables cause it will look for a GPIO PIN not a variable the contains your gpio pin
{
    this->CE = CE;         // put your CE pin here ex. GPIO_PIN_3
    this->CSN = CSN;       // put your CSN pin here ex. GPIO_PIN_4
    this->GPIO = pinGroup; // put your GPIO pin group here ex. GPIOB, GPIOA
    // This will do:  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_SET) -> HAL_GPIO_WritePin(GPIO, CSN, OFF)
    payLoadSize = 0;
}

void Nrf::begin()
{
    HAL_Delay(11);
    HAL_GPIO_WritePin(this->GPIO, this->CE, OFF);
    HAL_GPIO_WritePin(this->GPIO, this->CSN, ON);
}

void Nrf::setPowerLevel(powerLevel level) // setting power level adress (put only levels: testingRange, shortRange, midRange, longRange)
{
    uint8_t adress = 0x20 | 0x06;
    uint8_t levelConfig = (uint8_t)level;

    HAL_GPIO_WritePin(this->GPIO, this->CSN, OFF);
    HAL_SPI_Transmit(&hspi1, &adress, 1, 10); // assuming we use spi1 we do &hspi1
    HAL_SPI_Transmit(&hspi1, &levelConfig, 1, 100);
    HAL_GPIO_WritePin(this->GPIO, this->CSN, ON);

    // 0000 0[00]0
    // 00[0]0 [0][00]0
    // 5,3 can be change to get high range
    // 7654 3 21 0 -- only change bit 1 and 2 for level
    // level
    // min - 00
    // low - 01
    // high - 10
    // max - 11
    // 5,3 config range:
    // high 01 fast but short range 2mbps
    // mid 00 standard 1mbps
    // low 10 low but long range 250kbs
    // no need to mask using AND

    // example configs:

    // short range high mbps
    // 0000 1000 -> min - PA level
    // 0000 1010 -> low - PA level
    // 0000 1100 -> high - PA level
    // 0000 1110 -> max - PA level

    // mid range standart mbps
    // 0000 0000 -> min - PA level
    // 0000 0010 -> low - PA level
    // 0000 0100 -> high - PA level
    // 0000 0110 -> max - PA level

    // high range low mbps
    // 0010 0000 -> min - PA level
    // 0010 0010 -> low - PA level
    // 0010 0100 -> high - PA level
    // 0010 0110 -> max - PA level

    // high mbps means past transfer of data good for short range bad for long range.

    // high PA level means the signal can pass through walls but eats more battery -- you need to use external power source for your nrf but same power source as your MCU
    // to avoid overheating the on board regulator of your devBoard.

    // adress 0x06
}

void Nrf::setTxAdress(uint8_t *adress) // setting tx adress
{
    uint8_t regAdress = 0x20 | 0x10;

    HAL_GPIO_WritePin(this->GPIO, this->CSN, OFF);
    HAL_SPI_Transmit(&hspi1, &regAdress, 1, 10);
    HAL_SPI_Transmit(&hspi1, adress, 5, 100);
    HAL_GPIO_WritePin(this->GPIO, this->CSN, ON);
}

void Nrf::setRxAdress(uint8_t *adress) // setting rx address
{
    uint8_t regAdress = 0x20 | 0x0A;

    HAL_GPIO_WritePin(this->GPIO, this->CSN, OFF);
    HAL_SPI_Transmit(&hspi1, &regAdress, 1, 10);
    HAL_SPI_Transmit(&hspi1, adress, 5, 100);
    HAL_GPIO_WritePin(this->GPIO, this->CSN, ON);
}

void Nrf::setPayLoad(uint8_t payLoadSize, uint8_t pipeNum) // setting payload size
{

    if (payLoadSize > 32)
        payLoadSize = 32;
    if (pipeNum > 5)
        pipeNum = 5;

    uint8_t regAdress = 0x20 | (0x11 + pipeNum);
    this->payLoadSize = payLoadSize;

    HAL_GPIO_WritePin(this->GPIO, this->CSN, OFF);
    HAL_SPI_Transmit(&hspi1, &regAdress, 1, 10);
    HAL_SPI_Transmit(&hspi1, &payLoadSize, 1, 10);
    HAL_GPIO_WritePin(this->GPIO, this->CSN, ON);
}

void Nrf::write(uint8_t *data)
{
    uint8_t command = 0xA0;

    HAL_GPIO_WritePin(this->GPIO, this->CSN, OFF);
    HAL_SPI_Transmit(&hspi1, &command, 1, 10);
    HAL_SPI_Transmit_DMA(&hspi1, data, payLoadSize);
}

