// TODO: САМАЯ ГЛАВНАЯ ЗАДАЧА РАЗОБРАТЬСЯ С ЗАВИСАНИЕМ MQTT!!!
// ТАКЖЕ: заменить CONSOLE LOG на очередь

#include "functions.h"

#include "dns.h"
#include "mqtt_interface.h"
#include "MQTTClient.h"
#include "queue.h"
#include "common.h"
#include "task.h"
#include "stdlib.h"
#include "semphr.h"



extern osSemaphoreId SemSaveSettingsHandle;


int rc_tmp = 0;

// TODO: раскитать функции по разным файлам
// TODO: по максимуму заметить printf на memcpy

// Переменные для дебага
uint32_t buff_size = 0;
TaskStatus_t debug_tasks_info[10];
uint8_t debug_tasks_count = 0;

extern osMessageQId MQTT_MessagesHandle;
extern osMessageQId modbusQueueHandle;
extern osTimerId everySecondHandle;

char MqttRXTopic[100];

// флаги
uint8_t boot_flag = 0;

char log_string[LOG_STRING_SIZE];
char mlog_string[LOG_STRING_SIZE];

// сторожевая собака на задачу работы с сетью
uint32_t local_wd = 0;

dev_settings system_settings;

uint8_t dhcp_buffer[DATA_BUF_SIZE];
uint8_t dns_buffer[DATA_BUF_SIZE];
uint8_t mqtt_buffer[DATA_BUF_SIZE];
wiz_NetInfo net_info;
volatile uint8_t ip_assigned, dhcp_status;

// HTTP SERVER
uint8_t HTTP_RX_BUF[DATA_BUF_SIZE];
uint8_t HTTP_TX_BUF[DATA_BUF_SIZE];

// MQTT
uint16_t mes_id;
Network n;
MQTTClient c;
uint8_t buf[100];
int32_t rc;

// Chip ID
uint16_t *idBase0 = (uint16_t*) (UID_BASE);
uint32_t *idBase3 = (uint32_t*) (UID_BASE + 0x08);
// uint16_t *idBase1 = (uint16_t*) (UID_BASE + 0x02);
// uint32_t *idBase2 = (uint32_t*) (UID_BASE + 0x04);

const uint8_t MAX7219_SYMBOLS_ARR[10] = { MAX7219_SYM_0, MAX7219_SYM_1,
		MAX7219_SYM_2, MAX7219_SYM_3, MAX7219_SYM_4, MAX7219_SYM_5,
		MAX7219_SYM_6, MAX7219_SYM_7, MAX7219_SYM_8, MAX7219_SYM_9 };

void mqtt_send_message(char topic[], char message[]) {
	// mqtt_init();
	if (c.isconnected) {
		MQTTMessage pubMessage;
		pubMessage.qos = QOS0;
		pubMessage.id = mes_id++; // TODO: может поместить в структуру (не во флэш)???
		pubMessage.payload = message;
		pubMessage.payloadlen = strlen(message);
		MQTTPublish(&c, topic, &pubMessage);

	}
	// MQTTDisconnect(&c);
}

void StartLanTask(void const *argument) {
	strcpy(log_string, "DEBUG: device is running");

	HAL_GPIO_WritePin(W5500_RST_GPIO_Port, W5500_RST_Pin, GPIO_PIN_RESET);
	osDelay(1);
	HAL_GPIO_WritePin(W5500_RST_GPIO_Port, W5500_RST_Pin, GPIO_PIN_SET);
	osDelay(1000);

	/* Регистрируем колбэки */
	reg_wizchip_cs_cbfunc(W5500_Select, W5500_Unselect);
	reg_wizchip_spi_cbfunc(W5500_ReadByte, W5500_WriteByte);
	reg_wizchip_spiburst_cbfunc(W5500_ReadBuff, W5500_WriteBuff);

	// распредиление памяти между сокетами
	uint8_t rx_tx_buff_sizes[] = { 2, 2, 2, 2, 2, 2, 2, 2 };
	wizchip_init(rx_tx_buff_sizes, rx_tx_buff_sizes);

	ip_assigned = 0;
	// назначаем макадрес
	uint64_t *mac;
	mac = (uint64_t*) &net_info.mac;
	*mac = system_settings.device_mac_address;
	setSHAR(net_info.mac);
	DHCP_init(DHCP_SOCKET, dhcp_buffer);

	reg_dhcp_cbfunc(Callback_IPAssigned, Callback_IPAssigned,
			Callback_IPConflict);
	osDelay(500);

	// получаем IP адрес без него дальше двигаться бесполезно
	while ((!ip_assigned)) {
		DHCP_run();
		DHCP_time_handler(); // надо вызавать раз в секунду но так работает лучше
		osDelay(100); // не блокируем остальные задачи
	}

	uint8_t http_soc[HTTP_SOCKETS_Q];
	for (uint8_t i = 0; i < HTTP_SOCKETS_Q; i++)
		http_soc[i] = HTTP_SOCKET + i;
	httpServer_init(HTTP_TX_BUF, HTTP_RX_BUF, HTTP_SOCKETS_Q, http_soc);
	uint32_t *pagesQty = (uint32_t*) HTML_PAGES_ADDRES;

	fileInfo *fileProfile = (fileInfo*) (pagesQty + 1);
	for (int i = 0; (i < *pagesQty) && (*pagesQty < 50); i++) {
		reg_httpServer_webContent((uint8_t*) fileProfile->name,
				(uint8_t*) fileProfile->address);
		fileProfile++;
	}
	DNS_init(DNS_SOCKET, dns_buffer);
	mqtt_init();
	boot_flag |= 0b0100;
	// MQTTDisconnect(&c);

	for (;;) {

		DHCP_run();
		if (LINK_ON) {
			ip_assigned = 1;
			for (uint8_t i = 0; i < HTTP_SOCKETS_Q; i++) {
				httpServer_run(i);
				osDelay(1);
			}
			if (!c.isconnected) {
				osDelay(1);
				//MQTTDisconnect(&c);
				//osDelay(100);
				mqtt_init();
			}

			if (c.isconnected) {
				osDelay(1);
				rc_tmp=MQTTYield(&c, 20);

				for (uint8_t i = 0; i < HTTP_SOCKETS_Q; i++) {
					osDelay(1);
					httpServer_run(i);
				}
				MQTT_Packet msg;
				if (xQueueReceive(MQTT_MessagesHandle, &msg, 1) == pdPASS) {
					osDelay(1);
					mqtt_send_message(&(msg.topic[0]), msg.message);

				}
			}

		} else {
			ip_assigned = 0;
		}
		local_wd = 0;
	}

}

/* функция отображения IP, и работы с кнопкой */
void StartDefaultTask(void const *argument) {

	/* Инициируем экран и включаем тестовый режим */
	max7219_Init();
	max7219_SendData(REG_DISPLAY_TEST, 1);
	osDelay(2000);
	max7219_SendData(REG_DISPLAY_TEST, 0);

	/* Если кнопка зажата то планируем сброс конфигурации*/
	if (!HAL_GPIO_ReadPin(KEY_GPIO_Port, KEY_Pin)) {
		max7219_TurnOn();
		max7219_SetIntensivity(0xf);
		uint16_t i, flag = 1;
		for (i = 1; i < 5; i++)
			max7219_SendData(i, MAX7219_SEG_G);
		/* снижаем яркость дисплея */
		for (i = 0; i < 200; i++) {
			max7219_SetIntensivity(0xf - i * 0xf / 200);
			if (HAL_GPIO_ReadPin(KEY_GPIO_Port, KEY_Pin)) {
				/* если кнопка отпущена отменяем сброс*/
				flag = 0;
				break;
			}
			osDelay(10);
		}
		/* выполняем сброс конфигурации */
		if (flag) {
			max7219_TurnOff();
			osDelay(200);
			setupReset(&system_settings);
			NVIC_SystemReset();
		}
	}
	boot_flag |= 1;
	max7219_TurnOff();
	/* ждем получения  IP */
	while (!ip_assigned) {
		osDelay(500);
	}
	osDelay(1500);

	for (;;) {

		max7219_TurnOn();
		max7219_SetIntensivity(5);
		uint8_t buff[27] = { 0, 0, 0, 0, MAX7219_SYM_I, MAX7219_SYM_P, 0, };
		// заполняем массив информацией об адресе
		for (uint8_t j = 0; j < 4; j++) {
			buff[j * 4 + 7] = MAX7219_SYMBOLS_ARR[net_info.ip[j] / 100];
			buff[j * 4 + 8] = MAX7219_SYMBOLS_ARR[net_info.ip[j] % 100 / 10];
			buff[j * 4 + 9] = MAX7219_SYMBOLS_ARR[net_info.ip[j] % 10]
					+ ((j == 3) ? 0 : MAX7219_SEG_DP);
		}
		/* выводим буфер бегущей строкой*/
		for (int8_t j = -1; j < 23; j++) {
			for (uint8_t i = 1; i < 5; i++) {
				max7219_SendData(i, buff[i + j]);
			}
			osDelay(200);
			if (j == 5 || j == 9 || j == 13 || j == 17)
				osDelay(1500);
		}
		max7219_TurnOff();

		/* ждем нажатия кнопки либо восстановления связи после её потери */
		uint8_t online = ip_assigned;
		while (HAL_GPIO_ReadPin(KEY_GPIO_Port, KEY_Pin)) {
			if ((!online) && ip_assigned)
				break;
			online = ip_assigned;
			osDelay(50);
		};
		debug2log();

	}

}

/* Секция програмных таймеров */

/*
 * Управление индикацией через шим
 * управление осуществляется посредство програмного таймера
 * Шим генерируется третьим таймером, каналом 2 (нога PB5)
 *
 */

/*
 * Функции max7219
 */

void max7219_TurnOn(void) {
	max7219_SendData(REG_SHUTDOWN, 0x01);
}

void max7219_TurnOff(void) {
	max7219_SendData(REG_SHUTDOWN, 0x00);
}

void max7219_Clean() {
	for (int i = 1; i < 9; i++)
		max7219_SendData(i, 4);
}

void max7219_SendData(uint8_t addr, uint8_t data) {
	HAL_GPIO_WritePin(DIG_CS_GPIO_Port, DIG_CS_Pin, GPIO_PIN_RESET);
	HAL_SPI_Transmit(&hspi2, &addr, 1, HAL_MAX_DELAY);
	HAL_SPI_Transmit(&hspi2, &data, 1, HAL_MAX_DELAY);
	HAL_GPIO_WritePin(DIG_CS_GPIO_Port, DIG_CS_Pin, GPIO_PIN_SET);
}

void max7219_Init() {
	max7219_SendData(REG_DISPLAY_TEST, 0);
	max7219_TurnOff();
	max7219_SendData(REG_SCAN_LIMIT, 3);
	max7219_SendData(REG_DECODE_MODE, 0x00); // decode off
	max7219_Clean();
}

void max7219_SetIntensivity(uint8_t intensivity) {
	if (intensivity > 0x0F)
		return;

	max7219_SendData(REG_INTENSITY, intensivity);
}

void settings_init(dev_settings *settings) {
	dev_settings *saved_settings = SETTINGS_FLASH_AREA;
	if (saved_settings->firstStart) {
		setupReset(settings);
	} else
		*settings = *saved_settings;
	return;
}

void setupReset(dev_settings *settings) {
	settings->device_type = 255;
	settings->firstStart = 0;
	settings->device_mac_address = *idBase3 + *idBase0;
	settings->device_mac_address <<= 8;
	settings->device_mac_address += +*idBase0;
	settings->device_mac_address <<= 8;
	settings->device_type = 1;
	settings->uart_speed = 9600;
	settings->mqtt_port = 1883;
	sprintf(settings->mqtt_address, "tldev.ru");
	sprintf(settings->mqtt_name, "");
	sprintf(settings->mqtt_password, "");
	sprintf(settings->mqtt_topic, "my_scada");
	sprintf(settings->scada_address, "scada.tldev.ru");
	sprintf(settings->mqtt_client_name, "TL_%ld",
			(uint32_t) settings->device_mac_address);
	sprintf(settings->login, "admin");
	sprintf(settings->password, "admin");
	memset(settings->mb_program, 0, 100);
	saveSettings();
	return;
}

void saveSettings(void) {
	xSemaphoreGive(SemSaveSettingsHandle);

/*
	static FLASH_EraseInitTypeDef EraseInitStruct;
	 //EraseInitStruct.TypeErase = 0;
	 EraseInitStruct.TypeErase = FLASH_TYPEERASE_PAGES;
	//EraseInitStruct.TypeErase = FLASH_TYPEERASE_MASS;
	EraseInitStruct.Banks = 0;
	EraseInitStruct.Page = SETTINGS_FLASH_PAGE;
	EraseInitStruct.NbPages = 1;


	HAL_StatusTypeDef rc = HAL_FLASH_Unlock();
	rc = HAL_FLASHEx_Erase(&EraseInitStruct, &error);
	HAL_FLASH_Lock();
	if (rc != HAL_OK) {
		strlcpy(log_string, "DEBUG: FLASH WRITING ERROR", LOG_STRING_SIZE);
	} else {
		uint64_t *pointer = (uint64_t*) &system_settings;

		uint32_t steps = sizeof(dev_settings) / 8
				+ ((sizeof(dev_settings) % 8) ? 1 : 0);
		HAL_FLASH_Unlock();
		for (int i = 0; i < steps; i++) {
			HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD,
			SETTINGS_FLASH_ADDRESS + i * 8, *pointer);
			pointer++;
		}
		HAL_FLASH_Lock();
		strlcpy(log_string, "DEBUG: CHANGES SAVED", LOG_STRING_SIZE);
	};

*/
}

unsigned int getDec(uint8_t *buff, int len) {
	unsigned int result = 0;
	for (int i = 0; i < len; i++) {
		if (buff[i] >= '0' && buff[i] <= '9')
			result = result * 10 + buff[i] - '0';
	}

	return result;
}

int findValue(uint8_t buff1[], uint8_t buff2[], int len, uint8_t **buff) {
	*buff = 0;
	uint32_t i;
	uint32_t pos;
	uint32_t len2 = strlen((char*) buff2);
	uint8_t flag = 0;
	pos = 0;
	for (i = 0; i < len - 2; i++) {
		if (buff1[i] != buff2[pos]) {
			pos = 0;
		} else {
			pos++;
		}
		if (pos == len2 && buff1[i + 1] == '=') {
			flag = 1;

			*buff = &buff1[i + 2];
			break;
		}
	}
	len2 = 0;
	i += 2;
	if (flag) {
		while (((i + len2) < len) && (buff1[i + len2] != '&')) {
			len2++;
		}

	}

	return len2;
}

uint8_t get_cgi_processor(uint8_t *uri_name, uint8_t *buf, uint16_t *len) {

	if (strcmp((const char*) uri_name, "get_info.cgi") == 0) {
		*len =
				sprintf((char*) buf,
						"{\"device_mac_address\":\"%02x %02x %02x %02x %02x %02x\", \"mqtt_port\":\"%d\",\"mqtt_address\":\"%s\",\"mqtt_topic\":\"%s\",\"scada_address\":\"%s\",\"mqtt_name\":\"%s\",\"mqtt_password\":\"%s\",\"modbus_speed\":\"%d\"}",
						(uint8_t) system_settings.device_mac_address,
						(uint8_t) (system_settings.device_mac_address >> 8 * 1),
						(uint8_t) (system_settings.device_mac_address >> 8 * 2),
						(uint8_t) (system_settings.device_mac_address >> 8 * 3),
						(uint8_t) (system_settings.device_mac_address >> 8 * 4),
						(uint8_t) (system_settings.device_mac_address >> 8 * 5),
						system_settings.mqtt_port, system_settings.mqtt_address,
						system_settings.mqtt_topic,
						system_settings.scada_address,
						system_settings.mqtt_name,
						system_settings.mqtt_password,
						system_settings.uart_speed);
		return 1;
	} else if (strcmp((const char*) uri_name, "form.cgi") == 0) {
		if (!checkLogin(buf)) {
			*len = (uint16_t) sprintf((char*) buf,
					"{\"result\":\"wrong login\"}");
			return 1;
		}
		uint8_t *pointer = NULL;
		int l, l1;
		l1 = strlen((char*) buf);

		l = findValue(buf, (uint8_t*) "mqtt_name", l1, &pointer);
		if (l) {
			strlcpy(system_settings.mqtt_name, (char*) pointer, l + 1);
		}
		l = findValue(buf, (uint8_t*) "mqtt_password", l1, &pointer);
		if (l) {
			strlcpy(system_settings.mqtt_password, (char*) pointer, l + 1);
		}

		l = findValue(buf, (uint8_t*) "modbus_speed", l1, &pointer);
		if (l) {
			char *endptr;
			uint32_t result = (uint32_t) strtoul((char*) pointer, &endptr, 10);
			if (endptr == (pointer + l)) {
				system_settings.uart_speed = result;
			}

		}

		l = findValue(buf, (uint8_t*) "mqtt_address", l1, &pointer);
		if (l) {
			strlcpy(system_settings.mqtt_address, (char*) pointer, l + 1);
		}

		l = findValue(buf, (uint8_t*) "mqtt_topic", l1, &pointer);
		if (l) {
			strlcpy(system_settings.mqtt_topic, (char*) pointer, l + 1);
		}
		l = findValue(buf, (uint8_t*) "scada_address", l1, &pointer);
		if (l) {
			strlcpy(system_settings.scada_address, (char*) pointer, l + 1);
		}
		l = findValue(buf, (uint8_t*) "mqtt_port", l1, &pointer);
		if (l) {
			system_settings.mqtt_port = getDec(pointer, l);
		}
		l = findValue(buf, (uint8_t*) "login_new", l1, &pointer);
		if (l) {
			strlcpy(system_settings.login, (char*) pointer, l + 1);

			l = findValue(buf, (uint8_t*) "password_new", l1, &pointer);
			strlcpy(system_settings.password, (char*) pointer, l + 1);
		}
        osDelay(10);
        // taskENTER_CRITICAL();
		saveSettings();
		// taskEXIT_CRITICAL();
		c.isconnected =0;

		*len = (uint16_t) sprintf((char*) buf, "{\"result\":\"ok\"}");
		return 1;

	} else if (strcmp((const char*) uri_name, "reboot.cgi") == 0) {
		if (HAL_GetTick() > 20000)
			NVIC_SystemReset();
		*len = (uint16_t) sprintf((char*) buf, "{\"result\":\"ok\"}");
		return 1;
	} else if (strcmp((const char*) uri_name, "debuginfo.cgi") == 0) {
		debug2log();
		*len = (uint16_t) strlcpy((char*) buf, "{\"result\":\"ok\"}",
		DATA_BUF_SIZE);
		return 1;

	} else if (strcmp((const char*) uri_name, "log.cgi") == 0) {
		if (log_string[0] != 0) {
			*len = (uint16_t) sprintf((char*) buf, "{\"log\":\"%s\",\"mlog\":\"%s\"}", log_string, mlog_string);
		} else
			buf[0] = 0;
		return 1;

	} else if (strcmp((const char*) uri_name, "login.cgi") == 0) {
		/* вход в систему по логину и паролю */
		if (checkLogin(buf)) {
			*len = sprintf((char*) buf, "{\"login\":\"1\"}");
		} else {
			*len = sprintf((char*) buf, "{\"login\":\"0\"}");
		}
		return 1;
	} else if (strcmp((const char*) uri_name, "modbus.cgi") == 0) {

		uint8_t *pointer = NULL;
		int l, l1;
		l1 = strlen((char*) buf);
		l = findValue(buf, (uint8_t*) "modbuscommand", l1, &pointer);
		if (12 == l) {
			uint8_t mb_command[8];
			for (int i = 0; i < 12; i += 2) {
				char hex_byte[3];
				hex_byte[0] = pointer[i];
				hex_byte[1] = pointer[i + 1];
				hex_byte[2] = '\0';
				mb_command[i / 2] = (uint8_t) strtol(hex_byte, NULL, 16);
			}
			xQueueSend(modbusQueueHandle, mb_command, 5);
		}

		l = findValue(buf, (uint8_t*) "modbusprogram", l1, &pointer);
		if (l) {
			if (l > 10) {
				memcpy(system_settings.mb_program, pointer, l);
				system_settings.mb_program[l] = 0;
			} else
				system_settings.mb_program[0] = 0;
			//taskENTER_CRITICAL();
					saveSettings();
			//		taskEXIT_CRITICAL();
		}

		*len = sprintf((char*) buf, "%s", system_settings.mb_program);

		return 1;
	}

	return 1;
}

uint8_t set_cgi_processor(uint8_t *uri_name, uint8_t *uri, uint8_t *buf,
		uint16_t *en) {
	return 1;
}

/* проверяем логин и пароль */
uint8_t checkLogin(uint8_t *buf) {
	uint8_t *pointer = buf;
	int l1, l = strlen((char*) buf);
	l1 = findValue(buf, (uint8_t*) "login", l, &pointer);
	if (l1 != strlen(system_settings.login))
		return 0;
	if (memcmp(pointer, system_settings.login, l1))
		return 0;
	l1 = findValue(buf, (uint8_t*) "password", l, &pointer);
	if (l1 != strlen(system_settings.password))
		return 0;
	if (memcmp(pointer, system_settings.password, l1))
		return 0;
	return 1;
}

void Callback_IPAssigned(void) {
	getIPfromDHCP(net_info.ip);
	getGWfromDHCP(net_info.gw);
	getSNfromDHCP(net_info.sn);
	uint8_t dns[4];
	getDNSfromDHCP(dns);
	wizchip_setnetinfo(&net_info);
	ctlnetwork(CN_SET_NETINFO, (void*) &net_info);
	ip_assigned = 1;
}

void Callback_IPConflict(void) {
	ip_assigned = 0;
}

void W5500_Select(void) {
	HAL_GPIO_WritePin(W5500_CS_GPIO_Port, W5500_CS_Pin, GPIO_PIN_RESET);
}

void W5500_Unselect(void) {
	HAL_GPIO_WritePin(W5500_CS_GPIO_Port, W5500_CS_Pin, GPIO_PIN_SET);
}

void W5500_ReadBuff(uint8_t *buff, uint16_t len) {
	HAL_SPI_Receive(&W5500_SPI, buff, len, HAL_MAX_DELAY);
}

void W5500_WriteBuff(uint8_t *buff, uint16_t len) {
	HAL_SPI_Transmit(&W5500_SPI, buff, len, HAL_MAX_DELAY);
}

uint8_t W5500_ReadByte(void) {
	uint8_t byte;
	W5500_ReadBuff(&byte, sizeof(byte));
	return byte;
}

void W5500_WriteByte(uint8_t byte) {
	W5500_WriteBuff(&byte, sizeof(byte));
}

void messageArrived(MessageData *md) {
	MQTTMessage *message = md->message;
	uint16_t l = strlen(MqttRXTopic);
	if (md->topicName->lenstring.len < l) {
		return;
	}

	uint8_t dev_addr = 0;
	uint16_t reg_addr = 0;
	uint8_t command = 0;
	uint16_t value = 0;

	l--;
	char *buff = md->topicName->lenstring.data;
	while (buff[l] != '/') {
		if (buff[l] >= '0' && buff[l] <= '9') {
			dev_addr = buff[l] - '0' + dev_addr * 10;
		} else {
			return;
		}

		l++;
		if (l == md->topicName->lenstring.len) {
			return;
		}
	}
	l++;
	if (buff[l] == 'C') {
		command = 05;
	}
	if (buff[l] == 'R') {
		command = 06;
	}
	if (!command) {
		return;
	}
	l += 2;
	while (l < md->topicName->lenstring.len) {
		if (buff[l] >= '0' && buff[l] <= '9') {
			reg_addr = buff[l] - '0' + reg_addr * 10;
		} else {
			return;
		}
		l++;
	}

	l = 0;
	buff = md->message->payload;
	while (l < md->message->payloadlen) {
		if (buff[l] >= '0' && buff[l] <= '9') {
			value = buff[l] - '0' + value * 10;
		} else {
			return;
		}
		l++;
	}

	uint8_t mb_command[8];
	mb_command[0] = dev_addr;
	mb_command[1] = command;
	mb_command[2] = reg_addr >> 16;
	mb_command[3] = reg_addr % 0x100;
	mb_command[4] = value >> 8;
	mb_command[5] = value % 0x100;
	mb_command[6] = 0;
	mb_command[7] = 0;
	xQueueSend(modbusQueueHandle, mb_command, 5);

	/*
	 for (uint8_t i = 0; i < md->topicName->lenstring.len; i++)
	 putchar(*(md->topicName->lenstring.data + i));
	 */
	/*UART_Printf(" (%.*s)\r\n",
	 (int32_t) message->payloadlen,
	 (char*) message->payload);*/
}

void mqtt_init() {
	uint8_t targetIP[4];
	unsigned int *ip = (unsigned int*) (&targetIP[0]);
	*ip = get_ip_by_name(system_settings.mqtt_address);
	NewNetwork(&n, MQTT_SOCKET);
	ConnectNetwork(&n, targetIP, 1883);
	MQTTClientInit(&c, &n, 1000, buf, 100, mqtt_buffer, 2048);
	MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
	snprintf(system_settings.mqtt_client_name,MAX_LENGTH_OF_SITE_NAME, "tl_d_%x%x", (uint16_t)system_settings.device_mac_address,(uint16_t)HAL_GetTick());

	data.willFlag = 0;
	data.MQTTVersion = 3;
	data.clientID.cstring = &system_settings.mqtt_client_name[0];
	data.username.cstring = &system_settings.mqtt_name[0];
	data.password.cstring = &system_settings.mqtt_password[0];
	data.keepAliveInterval = 60;
	data.cleansession = 1;

	rc = MQTTConnect(&c, &data);
	// MqttRXTopic = "my_scadaRX/111/#";
	strlcpy(MqttRXTopic, system_settings.mqtt_topic, 100);
	strlcat(MqttRXTopic, "RX/", 100);
	if (strlen(MqttRXTopic) < 80) {
		itoa((uint32_t) system_settings.device_mac_address,
				&(MqttRXTopic[strlen(MqttRXTopic)]), 10);
	}
	strlcat(MqttRXTopic, "/#", 100);

	//snprintf(MqttRXTopic,100, "&sRX/%ul/#", system_settings.mqtt_topic, (uint32_t)system_settings.device_mac_address );
	rc = MQTTSubscribe(&c, MqttRXTopic, QOS2, messageArrived);
	return;
}

unsigned int get_ip_by_name(char site_name[]) {
	uint8_t addr[4];
	uint8_t dns[4];
	getDNSfromDHCP(dns);
	int8_t res = DNS_run(dns, (uint8_t*) &site_name[0], addr);
	if (res != 1) {
		return 0;
	}
	unsigned int *result = (unsigned int*) (&addr[0]);
	return *result;
}

void debug2log() {
	uint32_t _total_runtime;
	uxTaskGetSystemState(&(debug_tasks_info[0]), 7, &_total_runtime);

	osDelay(10); // отдадим управление другим приложениям, чтобы копирование в буфер не было прервано
	strlcpy(log_string, "DEBUG:__n__", LOG_STRING_SIZE);

	for (int x = 6; x >= 0; x--) {
		if (debug_tasks_info[x].xHandle != 0) {

			strlcat(log_string, "            Task ", LOG_STRING_SIZE);
			strlcat(log_string, debug_tasks_info[x].pcTaskName,
			LOG_STRING_SIZE);
			strlcat(log_string, " : free stack ", LOG_STRING_SIZE);
			if (strlen(log_string) < LOG_STRING_SIZE - 5) {
				itoa(debug_tasks_info[x].usStackHighWaterMark,
						&(log_string[strlen(log_string)]), 10);
			}
			strlcat(log_string, "__n__", LOG_STRING_SIZE);

		}
	}
	strlcat(log_string, "            running: ", LOG_STRING_SIZE);
	int mins = (HAL_GetTick() / 60000);

	if (strlen(log_string) < LOG_STRING_SIZE - 10) {
		itoa(mins / 60, &(log_string[strlen(log_string)]), 10);
	}
	strlcat(log_string, " h, ", LOG_STRING_SIZE);

	if (strlen(log_string) < LOG_STRING_SIZE - 10) {
		itoa(mins % 60, &(log_string[strlen(log_string)]), 10);
	}
	strlcat(log_string, " min__n__", LOG_STRING_SIZE);
}

void send_log(char *rx) {
	osDelay(2);
	strlcpy(log_string, rx, LOG_STRING_SIZE);
}

