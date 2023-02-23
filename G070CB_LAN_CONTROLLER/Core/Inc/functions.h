/*
 * functions.h
 *
 *  Created on: Nov 13, 2022
 *      Author: andrei.fedorov
 */

#ifndef INC_FUNCTIONS_H_
#define INC_FUNCTIONS_H_

#include "FreeRTOS.h"
#include "cmsis_os.h"
#include "tim.h"
#include "spi.h"
#include "gpio.h"
#include "iwdg.h"
#include "w5500.h"
#include "dhcp.h"
#include <stdio.h>
#include "httpServer.h"
#include "httpParser.h"
#include "httpUtil.h"
#include <string.h>
#include "common.h"
#include "app_Timers.h"



// проверка соедиения
#define LINK_ON  wizphy_getphylink()




// W5500

#define SETTINGS_FLASH_ADDRESS  0x08018800
#define SETTINGS_FLASH_AREA (dev_settings*) SETTINGS_FLASH_ADDRESS
#define SETTINGS_FLASH_PAGE 49

#define W5500_SPI hspi1
// размер http буферов менять нельзя!!! 2кб!!!
#define DATA_BUF_SIZE   2048





//HTML_DATA
#define HTML_PAGES_ADDRES  0x08019000
typedef struct fileInfo {
    char name[20];
    uint32_t address;
} fileInfo;



#define DHCP_SOCKET     0
#define DNS_SOCKET      1
#define MQTT_SOCKET     2
#define HTTP_SOCKET     3
#define HTTP_SOCKETS_Q  4



/*
 *  Константы радио протокола
 */





void MX_USART1_UART_Init(void);



void debug2log();



void saveSettings(void);







typedef enum {
    REG_NO_OP           = 0x00,
    REG_DIGIT_0         = 0x01,
    REG_DIGIT_1         = 0x02,
    REG_DIGIT_2         = 0x03,
    REG_DIGIT_3         = 0x04,
    REG_DIGIT_4         = 0x05,
    REG_DIGIT_5         = 0x06,
    REG_DIGIT_6         = 0x07,
    REG_DIGIT_7         = 0x08,
    REG_DECODE_MODE     = 0x09,
    REG_INTENSITY       = 0x0A,
    REG_SCAN_LIMIT      = 0x0B,
    REG_SHUTDOWN        = 0x0C,
    REG_DISPLAY_TEST    = 0x0F,
} MAX7219_REGISTERS;




typedef enum {
    MAX7219_SEG_G = (1<<0),
    MAX7219_SEG_F = (1<<1),
    MAX7219_SEG_E = (1<<2),
    MAX7219_SEG_D = (1<<3),
    MAX7219_SEG_C = (1<<4),
    MAX7219_SEG_B = (1<<5),
    MAX7219_SEG_A = (1<<6),
    MAX7219_SEG_DP = (1<<7),
} MAX7219_SEGMENTS;

typedef enum {
    MAX7219_SYM_0 = MAX7219_SEG_A | MAX7219_SEG_B | MAX7219_SEG_C | MAX7219_SEG_D | MAX7219_SEG_E | MAX7219_SEG_F,
    MAX7219_SYM_1 = MAX7219_SEG_B | MAX7219_SEG_C,
    MAX7219_SYM_2 = MAX7219_SEG_A | MAX7219_SEG_B | MAX7219_SEG_D | MAX7219_SEG_E | MAX7219_SEG_G,
    MAX7219_SYM_3 = MAX7219_SEG_A | MAX7219_SEG_B | MAX7219_SEG_C | MAX7219_SEG_D | MAX7219_SEG_G,
    MAX7219_SYM_4 = MAX7219_SEG_B | MAX7219_SEG_C | MAX7219_SEG_F | MAX7219_SEG_G,
    MAX7219_SYM_5 = MAX7219_SEG_A | MAX7219_SEG_C | MAX7219_SEG_D | MAX7219_SEG_F | MAX7219_SEG_G,
    MAX7219_SYM_6 = MAX7219_SEG_A | MAX7219_SEG_C | MAX7219_SEG_D | MAX7219_SEG_E | MAX7219_SEG_F | MAX7219_SEG_G,
    MAX7219_SYM_7 = MAX7219_SEG_A | MAX7219_SEG_B | MAX7219_SEG_C,
    MAX7219_SYM_8 = MAX7219_SEG_A | MAX7219_SEG_B | MAX7219_SEG_C | MAX7219_SEG_D | MAX7219_SEG_E | MAX7219_SEG_F | MAX7219_SEG_G,
    MAX7219_SYM_9 = MAX7219_SEG_A | MAX7219_SEG_B | MAX7219_SEG_C | MAX7219_SEG_D | MAX7219_SEG_F | MAX7219_SEG_G,
    MAX7219_SYM_A = MAX7219_SEG_A | MAX7219_SEG_B | MAX7219_SEG_C | MAX7219_SEG_E | MAX7219_SEG_F | MAX7219_SEG_G,
    MAX7219_SYM_B = MAX7219_SEG_C | MAX7219_SEG_D | MAX7219_SEG_E | MAX7219_SEG_F | MAX7219_SEG_G,
    MAX7219_SYM_C = MAX7219_SEG_A | MAX7219_SEG_D | MAX7219_SEG_E | MAX7219_SEG_F,
    MAX7219_SYM_D = MAX7219_SEG_B | MAX7219_SEG_C | MAX7219_SEG_D | MAX7219_SEG_E | MAX7219_SEG_G,
    MAX7219_SYM_E = MAX7219_SEG_A | MAX7219_SEG_D | MAX7219_SEG_E | MAX7219_SEG_F | MAX7219_SEG_G,
    MAX7219_SYM_F = MAX7219_SEG_A | MAX7219_SEG_E | MAX7219_SEG_F | MAX7219_SEG_G,
    MAX7219_SYM_G = MAX7219_SEG_A | MAX7219_SEG_C | MAX7219_SEG_D | MAX7219_SEG_E | MAX7219_SEG_F,
    MAX7219_SYM_H = MAX7219_SEG_B | MAX7219_SEG_C | MAX7219_SEG_E | MAX7219_SEG_F | MAX7219_SEG_G,
    MAX7219_SYM_I = MAX7219_SEG_E | MAX7219_SEG_F,
    MAX7219_SYM_J = MAX7219_SEG_B | MAX7219_SEG_C | MAX7219_SEG_D | MAX7219_SEG_E,
    MAX7219_SYM_L = MAX7219_SEG_D | MAX7219_SEG_E | MAX7219_SEG_F,
    MAX7219_SYM_N = MAX7219_SEG_C | MAX7219_SEG_E | MAX7219_SEG_G,
    MAX7219_SYM_O = MAX7219_SEG_C | MAX7219_SEG_D | MAX7219_SEG_E | MAX7219_SEG_G,
    MAX7219_SYM_P = MAX7219_SEG_A | MAX7219_SEG_B | MAX7219_SEG_E | MAX7219_SEG_F | MAX7219_SEG_G,
    MAX7219_SYM_Q = MAX7219_SEG_A | MAX7219_SEG_B | MAX7219_SEG_C | MAX7219_SEG_F | MAX7219_SEG_G,
    MAX7219_SYM_R = MAX7219_SEG_E | MAX7219_SEG_G,
    MAX7219_SYM_S = MAX7219_SYM_5,
    MAX7219_SYM_T = MAX7219_SEG_D | MAX7219_SEG_E | MAX7219_SEG_F | MAX7219_SEG_G,
    MAX7219_SYM_U = MAX7219_SEG_B | MAX7219_SEG_C | MAX7219_SEG_D | MAX7219_SEG_E | MAX7219_SEG_F,
    MAX7219_SYM_Y = MAX7219_SEG_B | MAX7219_SEG_C | MAX7219_SEG_D | MAX7219_SEG_F | MAX7219_SEG_G,
    MAX7219_SYM_MINUS = MAX7219_SEG_G,
    MAX7219_SYM_DEGREE = MAX7219_SEG_A | MAX7219_SEG_B | MAX7219_SEG_F | MAX7219_SEG_G,
    MAX7219_SYM_DOT = MAX7219_SEG_DP,
    MAX7219_SYM_BLANK = 0x00,
} MAX7219_SYMBOLS;





// MQTT
// Network n;
// MQTTClient c;
// uint8_t buf[100];
// int32_t rc ;



// прототипы

void settings_init(dev_settings *settings);
void led_indication(void const * argument);
void setupReset(dev_settings *settings);
uint8_t checkLogin(uint8_t *buf);



void mqtt_init();

unsigned int get_ip_by_name(char site_name[]);


uint8_t get_cgi_processor(uint8_t * uri_name, uint8_t * buf, uint16_t * len);
uint8_t set_cgi_processor(uint8_t * uri_name, uint8_t * uri, uint8_t * buf, uint16_t * en);



// W5500

void Callback_IPAssigned(void);
void Callback_IPConflict(void);
void W5500_Select(void);
void W5500_Unselect(void);
void W5500_ReadBuff(uint8_t *buff, uint16_t len);
void W5500_WriteBuff(uint8_t *buff, uint16_t len);
uint8_t W5500_ReadByte(void);
void W5500_WriteByte(uint8_t byte);





// max7219
void max7219_Init();
void max7219_SetIntensivity(uint8_t intensivity);
void max7219_Clean(void);
void max7219_SendData(uint8_t addr, uint8_t data);
void max7219_TurnOn(void);
void max7219_TurnOff(void);



void send_log(char * rx);


#endif /* INC_FUNCTIONS_H_ */
