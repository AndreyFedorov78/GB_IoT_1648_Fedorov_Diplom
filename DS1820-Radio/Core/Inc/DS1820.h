/*
 * D1820.h
 *
 *  Created on: Jun 17, 2022
 *      Author: andrei.fedorov
 */

#ifndef INC_DS1820_H_
#define INC_DS1820_H_
#include <stdint.h>
// Байты, отправляемые по UART для эмуляции различных сигналов 1-Wire.
#define OW_RESET            0xF0
#define OW_W0               0x00
#define OW_W1               0xFF
#define OW_R                0xFF

// Скорости UART для выполнения сброса 1-Wire и для передачи данных.
#define OW_RESET_SPEED      9600
#define OW_TRANSFER_SPEED   115200

// Значения, возвращаемые функцией ow_reset()
#define OW_NO_DEVICE        0
#define OW_OK               1


int DS1820_GetTemp();


#endif /* INC_DS1820_H_ */
