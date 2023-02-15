/*
 * DS1820.c
 *
 *  Created on: Jun 17, 2022
 *      Author: andrei.fedorov
 */

#include "DS1820.h"
#include "main.h"

extern UART_HandleTypeDef huart1;

void usart_change_speed(uint32_t speed) {
	USART1->BRR = SystemCoreClock / speed;
}

int ow_reset() {
	uint8_t data = OW_RESET;
	// Для формирования сигнала сброса требуется переключить
	// USART на скорость 9600.
	usart_change_speed(OW_RESET_SPEED);
	// Формируем сигнал сброса отправкой через USART байта 0xF0.
	HAL_UART_Transmit(&huart1, &data, 1, 1000);
	//usart2_send_byte(OW_RESET);
	// Проверяем наличие сигнала присутствия: если полученный байт совпадает
	// с отправленным - сигнала присутствия нет;
	// если не совпадает - сигнал присутствия получен.
	HAL_UART_Receive(&huart1, &data, 1, 1000);
	// Возвращаем скорость USART к значению по умолчанию.
	usart_change_speed(OW_TRANSFER_SPEED);

	return (data == OW_RESET) ? OW_NO_DEVICE : OW_OK;
}

// Функция отправляет байт ch через USART2;
// возврат из функции происходит после успешной записи байта в буфер
// передаваемых данных (если буфер не пуст, функция ждёт его освобождения).
void usart_send_byte(uint8_t ch) {
	HAL_UART_Transmit(&huart1, &ch, 1, 1000);
}

// Функция возвращает байт, принятый USART2;
// если буфер принимаемых данных USART2 пуст, функция ждёт получения данных.
char usart_receive_byte() {
	uint8_t data;
	HAL_UART_Receive(&huart1, &data, 1, 1000);
	return data;
}

// Функция отправляет бит ведомым устройствам 1-Wire (посылается младший бит
// аргумента b).
// Для получения бита от ведомого устройства, вызвать функцию с аргументом 1.
uint8_t ow_send_bit(uint8_t b) {
	usart_send_byte((b & 1) ? OW_W1 : OW_W0);
	return (usart_receive_byte() == OW_W1) ? 1 : 0;
}

// Функция отправляет байт b ведомым устройствам 1-Wire.
// Получение байта эквивалентно отправке байта 0xFF.
uint8_t ow_send_byte(uint8_t b) {
	int r = 0;
	for (int i = 0; i < 8; i++, b >>= 1) {
		r >>= 1;
		if (ow_send_bit(b))
			r |= 0x80;
	}
	return r;
}

// Отправить содержимое буфера buf размера size байт ведомым устройствам 1-Wire
void ow_send(const void *buf, unsigned int size) {
	const uint8_t *p = (const uint8_t*) buf;
	for (unsigned int i = 0; i < size; i++, p++)
		ow_send_byte(*p);
}

// Получить байт от ведомого устройства 1-Wire;
// эквивалентно ow_send(0xFF)
inline uint8_t ow_receive_byte() {
	return ow_send_byte(0xFF);
}

// Получить size байт от ведомого устройства и поместить их в буфер buf.
void ow_receive(void *buf, unsigned int size) {
	uint8_t *p = (uint8_t*) buf;
	for (unsigned int i = 0; i < size; i++, p++)
		*p = ow_send_byte(0xFF);
}

int DS1820_GetTemp() {
	if(ow_reset()!=OW_OK)
	            return(-2000);
	ow_send("\xCC\x44", 2);

	// Ждём завершения преобразования в ADC;
	// для определения момента завершения можем воспользоваться тем, что
	// при внешнем питании во время преобразование при чтении бита
	// возвращается 0, после завершения преобразования считывается 1.
	while (ow_send_bit(1) == 0)
		HAL_Delay(50);

	// Выполняем сброс и посылаем команды
	// SKIP ROM (0xCC), READ SCRATCHPAD (0xBE)
	// для считывания результата измерения.
	ow_reset();
	ow_send("\xCC\xBE", 2);

	// Читаем два байта, получим целое со знаком - температуру в
	// градусах по Цельсию, умноженную на 16.
	// При желании можно прочитать всё содержимое SCRATCHPAD для
	// проверки CRC.
	int16_t tx16 = 0;
	ow_receive(&tx16, 2);

	// Используем полученное значение, при желании можем преобразовать в
	// вещественный формат.
	// float t=tx16/16.0;
return tx16*10/16;

}

