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
    data = 0b00000000;
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

void Nrf::setMaxTry()
{
    // already set for 15 tries and 1500µs
    uint8_t regAdress = 0x20 | 0x04;
    uint8_t data = 0b01011111;

    HAL_GPIO_WritePin(this->GPIO, this->CSN, OFF);
    HAL_SPI_Transmit(&hspi1, &regAdress, 1, 10);
    HAL_SPI_Transmit(&hspi1, &data, 1, 10);
    HAL_GPIO_WritePin(this->GPIO, this->CSN, ON);
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

void Nrf::read(uint8_t *data)
{
    uint8_t command = 0x61;

    HAL_GPIO_WritePin(this->GPIO, this->CSN, OFF);
    HAL_SPI_Transmit(&hspi1, &command, 1, 10);
    HAL_SPI_Receive_DMA(&hspi1, data, payLoadSize);
}

void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi)
{
    if (hspi == &hspi1)
    {
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);
        // you can change GPIOA, must ensure that this is your CSN pin.
    }
}

// on HAL_SPI_Transmit_DMA()there are only 3 arguments the time is removed
// tx flush 0xE1
// rx flush 0xE2

//===========================================================================//
// status reg 0x07
//---------------------------------------------------------------------------//
//  bit 6 -> rx - flag if succes
//  bit 5 -> tx - flag if succes
//  bit 4 -> max rt - flag if it fails
//===========================================================================//

//===========================================================================//
// note to do!!!
//---------------------------------------------------------------------------//
// --make a MAX_RT function -> register 0x04 -> (fig 1) ------- [done]
// --make status checker for tx and rx -> check bit 5 or 6 to see the flag, flip bit to 1 to clear
// --modify tx write function to check status bit 0, if bit 0 is 1 it contains something if it is 0 then safe to send data
//===========================================================================//

//===========================================================================//
// for status checker
//---------------------------------------------------------------------------//
// do transmitReceive register adress 0x07 receive status
// for tx -> if status bit 5 is 1
// send 1 to bit 5 to clear -> means succes
// else if status bit 4 is 1
// send 1 to bit 4 to clear -> clear fifo 0xE1 if tx
//---------------------------------------------------------------------------//
// no need to do much on receiver no need to check but must flip the rx dx to 1
// for rx -> if status bit 6 is 1
// send 1 to bit 6 to clear -> means succes
//===========================================================================//

//===========================================================================//
// fig 1
//---------------------------------------------------------------------------//
// bit 7 - 4 -> time
// bit 3 - 0 -> max tries
//===========================================================================//

//===========================================================================//
// for MAX_RT time bits 7 - 4
//---------------------------------------------------------------------------//
// 0000 = 250µs
// 0001 = 500µs
// 0101 = 1500µs
// 1111 = 4000µs
//===========================================================================//

//===========================================================================//
// for MAX_RT Tries bits 3 - 0
//---------------------------------------------------------------------------//
// just do math
// every bit position represents a number
// bit 0 = 1
// bit 1 = 2
// bit 2 = 4
// bit 3 = 8
// formula -> 2^n where n is the position of bit. for example bit 3 = 2^3 meaning bit 3 = 2x2x2 = 8
// note!! max of 15 tries
//===========================================================================//
