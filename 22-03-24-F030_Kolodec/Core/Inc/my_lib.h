#ifndef __MYLIB_H
#define __MYLIB_H

#include <stdint.h>
#define MHZ19UART huart1

/* команды MHZ, записываются во второй байт после 0xff и 0x01
 * помещены тут в качестве справки и не используются в коде
 * в будущем можно использовать */
#define MHZ_READ 0x86 // чтение концентрации
#define MHZ_CALIB_ZERO 0x87 // калибровка по 400ppm (на улице)
#define MHZ_CALIB_SPAN 0x88 // калибровка по значению (передается после команды двумя байтами)
#define MHZ_CALIB_ON_OFF 0x79 // включение / отключение автокалибровки
#define MHZ_RANGE 0x99 // Режим работы 0-2000, 0-5000, 0-10000


uint32_t  water_level(uint32_t dat);

char* buff_copy(char *goal, char *source, int length); // копирование буфера source в буфер goal, возвращает ссылку на символ GOAL идущий за source
char* long_to_buff(char *goal, int32_t source); // запись целого числа в буфер, возвращает ссылку на элемент следующий за числом.

#endif
