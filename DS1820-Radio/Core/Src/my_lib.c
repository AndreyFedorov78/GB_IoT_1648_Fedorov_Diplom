#include "my_lib.h"
#include "main.h"

/*
 * Переменные и функции для работы с датчиком CO2 "MH-Z19".
 * Возможна работа с датчиками A,B и С
 */



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



/****************DS1820***************/




