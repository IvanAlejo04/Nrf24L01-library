/*
 * Copyright (c) 2026 Ivan Alejo
 * Licensed under the MIT License.
 * See LICENSE file in the project root for full license information.
 */
// april 9 2026

#include "Nrf.h"
#include "main.h"

/* USER CODE BEGIN ET */
// extern SPI_HandleTypeDef hspi1;  - type this block of code the exter..... on your main
/* USER CODE END ET */

#define ON GPIO_PIN_SET
#define OFF GPIO_PIN_RESET

Nrf::Nrf(uint16_t CE, uint16_t CSN, GPIO_TypeDef* pinGroup) // GPIO_TypeDef* is like variable but it is a pointer for your gpio pin, you must use TypeDef instead of normal variables cause it will look for a GPIO PIN not a variable the contains your gpio pin
{
    this->CE = CE;         // put your CE pin here ex. GPIO_PIN_3
    this->CSN = CSN;       // put your CSN pin here ex. GPIO_PIN_4
    this->GPIO = pinGroup; // put your GPIO pin group here ex. GPIOB, GPIOA
    // This will do:  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_SET) -> HAL_GPIO_WritePin(GPIO, CSN, OFF)
}

void Nrf::setSignal(uint8_t level)
{   // user input for example [0000 0000] -> min
    uint8_t adress = 0x20 | 0x06;
    

    HAL_GPIO_WritePin(this -> GPIO,this ->  CSN, OFF);
    HAL_SPI_Transmit(&hspi1, &adress, 1, 10); // assumin we use spi1 we do &hspi1
    HAL_SPI_Transmit(&hspi1, &level, 1, 100);
    HAL_GPIO_WritePin(this -> GPIO,this ->  CSN, ON);



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

    // short range
    // 0000 1000 -> min
    // 0000 1010 -> low
    // 0000 1100 -> high
    // 0000 1110 -> max

    // mid range
    //  0000 0000 -> min
    //  0000 0010 -> low
    //  0000 0100 -> high
    //  0000 0110 -> max

    // high range
    //  0010 0000 -> min
    //  0010 0010 -> low
    //  0010 0100 -> high
    //  0010 0110 -> max

    // adress 0x06
}

