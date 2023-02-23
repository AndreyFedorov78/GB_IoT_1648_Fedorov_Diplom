/*
 * app_ModBus.h
 *
 *  Created on: Jan 8, 2023
 *      Author: andrei.fedorov
 */

#ifndef INC_APP_MODBUS_H_
#define INC_APP_MODBUS_H_

#include "gpio.h"
#include "usart.h"
#include "cmsis_os.h"
#include <stdint.h>


#define MB_BUF_TX_SIZE 20
#define MB_BUF_RX_SIZE 254

#define READ_COIL_STATUS 0x01
#define READ_DISCRETE_INPUTS 0x02
#define READ_HOLDING_REGISTERS 0x03
#define READ_INPUT_REGISTERS 0x04
#define FORCE_SINGLE_COIL 0x05
#define PRESET_SINGLE_REGISTER 0x06
#define READ_EXCEPTION_STATUS 0x07
#define FORCE_MULTIPLE_COILS 0x0F
#define PRESET_MULTIPLE_REGISTERS 0x10
#define MASK_WRITE_REGISTER 0x16
#define READ_FIFO_QUEUE 0x18
#define READ_FILE_RECORD 0x14
#define WRITE_FILE_RECORD 0x15
#define DIAGNOSTIC 0x08
#define GET_COM_EVENT_COUNTER 0x0b
#define GET_COM_EVENT_LOG 0x0c
#define REPORT_SLAVE_ID 0x11
#define ENCAPSULATED_INTERFACE_TRANSPORT 0x2b

#define COMMANDS_QUANTITY 6

uint16_t GetCRC16(uint8_t *buf, uint8_t len);
uint8_t checkCRC16(uint8_t *buf, uint8_t len);
uint32_t readHEX(uint8_t *buf, uint8_t size);

int modbusSendCommand(uint8_t devise, uint8_t command, uint16_t address,
		uint16_t data);
int modbusGetAnswer();

#endif /* INC_APP_MODBUS_H_ */
