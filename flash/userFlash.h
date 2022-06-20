#ifndef __USER_FLASH_H
#define __USER_FLASH_H
#include "main.h"
#include "stm32f2xx_hal_flash.h"



void flashInit();
uint8_t flashRead(uint32_t address, uint8_t *buf, uint16_t len);
uint8_t flashErase(uint32_t address);
uint8_t flashWrite(uint32_t address, uint8_t *buf, uint16_t len);

#endif