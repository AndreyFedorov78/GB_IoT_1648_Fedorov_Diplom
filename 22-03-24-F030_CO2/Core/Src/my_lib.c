#include "my_lib.h"
#include "main.h"

/*
 * Переменные и функции для работы с датчиком CO2 "MH-Z19".
 * Возможна работа с датчиками A,B и С
 */

extern UART_HandleTypeDef MHZ19UART; // подтягиваем указатель на UART из main.c
uint8_t mhz19_resive[9] = { 0 }; // буфер в который будем получать данные с датчика
uint8_t mhz19_getppm[] =
		{ 0xff, 0x01, 0x86, 0x00, 0x00, 0x00, 0x00, 0x00, 0x79 }; // команда получения концентрации
uint8_t mhz19_CalibOff[9] = { 0xFF, 0x01, 0x79, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x86 }; // отключение автокалибровки
uint8_t mhz19_CalibOn[] =
		{ 0xFF, 0x01, 0x79, 0xA0, 0x00, 0x00, 0x00, 0x00, 0xE6 }; // включение автокалибровки
uint8_t mhz19_Range[] = { 0xFF, 0x01, 0x99, 0x00, 0x00, 0x00, 0x00, 0x00, 0xE6 }; // установка предела измерений 2000/5000/10000
uint8_t mhz19_Calib0[] =
		{ 0xFF, 0x01, 0x87, 0x00, 0x00, 0x00, 0x00, 0x00, 0x78 }; // Калибровка по 400ppm

uint8_t mhz19CheckSum(uint8_t *packet) {   // расчет контрольной суммы.
	uint8_t i, checksum;
	for (i = 1; i < 8; i++) {
		checksum += packet[i];
	}
	checksum = 0xff - checksum;
	checksum += 1;
	return checksum;
}

void mhz19Range(uint32_t range) { // возможны аргументы только 2000, 5000 или 10000
	for (uint8_t i = 7; i > 3; i--) {
		mhz19_Range[i] = range % 256;
		range /= 256;
	}
	mhz19_Range[8] = mhz19CheckSum(mhz19_Range);
	HAL_UART_Transmit(&MHZ19UART, mhz19_Range, 9, 1000);
	HAL_StatusTypeDef result = HAL_UART_Receive(&MHZ19UART, mhz19_resive, 9,
			3000);
	return;
}

void mhz19Calib0() {
	HAL_StatusTypeDef result = HAL_UART_Transmit(&MHZ19UART, mhz19_Calib0, 9,
			3000);
	if (result != HAL_OK)
		HAL_UART_Receive(&MHZ19UART, mhz19_resive, 9, 3000);
	return;
}

int mhz19Check() {
	HAL_UART_Transmit(&MHZ19UART, mhz19_getppm, 9, 1000);
	return HAL_UART_Receive(&MHZ19UART, mhz19_resive, 9, 3000);
}

void mhz19CalibOff() {
	HAL_StatusTypeDef result = HAL_UART_Transmit(&MHZ19UART, mhz19_CalibOff, 9,
			3000);
	HAL_UART_Receive(&MHZ19UART, mhz19_resive, 9, 3000);
	return;
}

int mhz19GetPPM() {
	if (HAL_UART_Transmit(&MHZ19UART, mhz19_getppm, 9, 1000) != HAL_OK)
		return -1;
	HAL_StatusTypeDef result = HAL_UART_Receive(&MHZ19UART, mhz19_resive, 9,
			3000);
	if (result != HAL_OK)
		return -1;
	if (mhz19_resive[8] != mhz19CheckSum(mhz19_resive))
		return -2;
	return mhz19_resive[2] * 256 + mhz19_resive[3];

}

/*
 * Вспомогательные функции вормирования строки (вместо  sprintf)
 * */

char* buff_copy(char *goal, char *source, int length) {
	for (uint8_t i = 0; i < length; i++) {
		goal[0] = source[0];
		goal++;
		source++;
	}
	return goal;
}

char* long_to_buff(char *goal, int32_t source) {
	if (source < 0) {
		goal[0] = '-';
		goal++;
		source = -source;
	}
	uint64_t i = 1;
	while ((long) source / i > 0)
		i *= 10;
	i = (i == 1) ? 1 : i / 10;
	while (i > 0) {
		goal[0] = (source / i) % 10 + '0';
		i /= 10;
		goal++;

	}
	return goal;
}

