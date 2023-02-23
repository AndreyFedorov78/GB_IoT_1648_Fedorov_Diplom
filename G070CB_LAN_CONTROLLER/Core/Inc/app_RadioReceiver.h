//
// Created by Andrei Fedorov on 20.11.2022.
//

#ifndef G070CB_LAN_CONTROLLER_APP_RADIO_RECEIVER_H
#define G070CB_LAN_CONTROLLER_APP_RADIO_RECEIVER_H


#include <stdint.h>


#define D433_SPEED 2400

#define FRAME_TIME (1000000ul / D433_SPEED)
#define HALF_FRAME (FRAME_TIME / 2)
#define START_PULSE (FRAME_TIME * 2)
// окно времени для обработки импульса
#define START_MIN (START_PULSE * 3 / 4)
#define START_MAX (START_PULSE * 5 / 4)
#define FRAME_MIN (FRAME_TIME * 3 / 4)
#define FRAME_MAX (FRAME_TIME * 5 / 4)
#define HALF_FRAME_MIN (HALF_FRAME * 3 / 4)
#define HALF_FRAME_MAX (HALF_FRAME * 5 / 4)

#define RADIO_TIM TIM16
#define RADIO_TIM_H &htim16

// количество синхроимульсов
#define TRAINING_PULSES 3

#include "common.h"

uint8_t check_crc8(uint8_t *buffer, uint8_t size);
uint8_t buffToMsg(MQTT_Packet *msg, uint8_t *buff);


#endif //G070CB_LAN_CONTROLLER_APP_RADIO_RECEIVER_H
