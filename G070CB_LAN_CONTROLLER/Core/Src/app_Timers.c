#include <stdint.h>
#include "app_Timers.h"
#include "dhcp.h"
#include "httpServer.h"
#include "dns.h"
#include "iwdg.h"
#include "gpio.h"
#include "common.h"
#include "MQTTClient.h"

extern uint32_t local_wd;
extern uint8_t ip_assigned;
extern uint8_t boot_flag;
extern MQTTClient c;

uint16_t minutes_counter = 0;

/*
 Тут содержатся колбэки программных таймеров
 */

//  Ежесекундный таймер
void everySecondCallback(void const *argument) {
    HAL_IWDG_Refresh(&hiwdg);
	DHCP_time_handler();
	httpServer_time_handler();
	DNS_time_handler();
	minutes_counter++;
	if (minutes_counter > 180) {
		c.isconnected = 0;
	}

	// если основная задача зависла на 60 секунд
	local_wd++;
	 if (local_wd > 60) {
	  //NVIC_SystemReset();
	 }
}

/*  */
void led_indication(void const *argument) {
	static uint32_t button_delay;



	// перезагрузка по трехсекундному нажатию кнопки
	if (!HAL_GPIO_ReadPin(KEY_GPIO_Port, KEY_Pin) && boot_flag) {
		if (HAL_GetTick() - button_delay > 3000) {
			NVIC_SystemReset();
		}
	} else
		button_delay = HAL_GetTick();

	// мирцание диода
	if (ip_assigned) {
		static int8_t direction = 10;
		if (TIM17->CCR1 >= 600)
			direction = -10;
		if (TIM17->CCR1 == 0)
			direction = 10;
		TIM17->CCR1 += direction;
	} else {
		TIM17->CCR1 = (TIM17->CCR1 == 0) ? 600 : 0;
	}
}
