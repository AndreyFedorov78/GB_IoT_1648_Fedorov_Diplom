//
// Created by Andrei Fedorov on 20.11.2022.
//

#include "app_RadioReceiver.h"
#include "tim.h"
#include "FreeRTOS.h"
#include "cmsis_os.h"
#include "queue.h"
#include  "common.h"
#include <string.h>
#include <stdio.h>


extern osMessageQId PeriodsHandle;
extern osMessageQId MQTT_MessagesHandle;
extern char log_string[LOG_STRING_SIZE];
uint8_t flagRadioEnable = 0;
uint8_t radioBuffer[RADIO_BUFFER_SIZE];
uint8_t radioBufferOld[RADIO_BUFFER_SIZE];
uint8_t *radioBufferPointer = radioBuffer;

BaseType_t debResult;


extern dev_settings system_settings;

/*
 Эта задача получает временные интервалы от радио передатчика
 и преобразовывает их в данные для SQL пакета.
 */
_Noreturn void StartRadioReceiver(void const *argument) {
    HAL_TIM_Base_Start(RADIO_TIM_H);
    uint32_t timer;
    osDelay(3000); // даем 3 секунды остальным приложениям на установление связи и инициализации
    flagRadioEnable = 1;
    for (;;) {

        uint8_t start_count = 1;
        uint8_t bufferSize = 0;
        uint8_t mode = 0, byteMask = 0;
        radioBufferPointer = radioBuffer;
        BaseType_t result = pdPASS;

        // начинаем декодирование пока идут пакеты из очереди
        while (result == pdPASS) {
            result = xQueueReceive(PeriodsHandle, &timer, 10);
            // если передача закончилась
            if (errQUEUE_EMPTY == result || (timer > START_MAX) || (timer < HALF_FRAME_MIN)) {
                radioBufferPointer = radioBuffer;
                if (!(2 == mode && bufferSize > 5 &&
                      check_crc8(radioBuffer, bufferSize - 1) == radioBuffer[bufferSize - 1])) {
                    break;
                }
                radioBuffer[bufferSize - 1] = 0;

                // лог
                if (strcmp((char *) radioBuffer, log_string)) {
                    osDelay(1);
                    strlcpy(log_string, (char *) radioBuffer, LOG_STRING_SIZE);
                    MQTT_Packet msg;
                    if (buffToMsg(&msg, radioBuffer)) xQueueSend(MQTT_MessagesHandle, &msg, 5);
                    break;
                }
            }


            switch (mode) {
                case 0:  // проверяем не началась ли передача тактового сигнала
                    if ((timer < START_MAX) && (timer > FRAME_MIN)) {
                        start_count++;
                    } else start_count = 0;
                    if (start_count > TRAINING_PULSES) {
                        *radioBufferPointer = 0;
                        byteMask = 0;
                        mode = 1;
                        start_count = 0;
                    }
                    break;
                case 1: // жем стартового сигнала
                    if ((timer < FRAME_MAX) && (timer > FRAME_MIN)) {
                        break;
                    } else if ((timer < START_MAX) && (timer > START_MIN)) mode = 2;
                    else mode = 0;
                    break;
                case 2: // читаем биты
                    *radioBufferPointer += (timer < HALF_FRAME_MAX) ? 0 : 1 << byteMask;
                    byteMask++;
                    if (byteMask == 8) {
                        byteMask = 0;
                        radioBufferPointer++;
                        *radioBufferPointer = 0;
                        bufferSize++;
                        break;
                    }
            }

            if (bufferSize == RADIO_BUFFER_SIZE) break;
        }

        osDelay(1);
    }

}


void HAL_GPIO_EXTI_Rising_Callback(uint16_t GPIO_Pin) {
    static uint32_t period;
    if (RADIO_PIN_Pin == GPIO_Pin) {
        period = RADIO_TIM->CNT;
        RADIO_TIM->CNT = 0;
        if (flagRadioEnable) { xQueueSendFromISR(PeriodsHandle, &period, NULL); }
    }
}

void HAL_GPIO_EXTI_Falling_Callback(uint16_t GPIO_Pin) {
    static uint32_t period;
    if (RADIO_PIN_Pin == GPIO_Pin) {
        period = RADIO_TIM->CNT;
        RADIO_TIM->CNT = 0;
        if (flagRadioEnable) { xQueueSendFromISR(PeriodsHandle, &period, NULL); }
    }
}


uint8_t check_crc8(uint8_t *buffer, uint8_t size) {
    uint8_t crc = 0;
    for (uint8_t i = 0; i < size; i++) {
        uint8_t j = 8;
        uint8_t data = buffer[i];
        while (j--) {
            crc = ((crc ^ data) & 1) ? (crc >> 1) ^ 0x8C : (crc >> 1);
            data >>= 1;
        }
    }
    return crc;
}


uint8_t buffToMsg(MQTT_Packet *msg, uint8_t *buff) {
    int8_t step = 0;
    char *pointer = &(msg->topic[0]);


    if (memcmp(buff, "ID;", 3)) {
        return 0;
    }
    buff += 3;
    // тут надо скопировать перффикс топика;
    pointer += strlcpy(pointer, system_settings.mqtt_topic, 20);
    *pointer = '/';
    pointer++;


    while (*buff != 0) {
        if (step < 2) {
            if (*buff != ';') {

                *pointer = *buff;
                pointer++;
            } else {
                step++;

                if (step == 2) {
                    *pointer = 0;
                    pointer = &(msg->message[0]);
                } else {
                    *pointer = '/';
                    pointer++;
                }
            }
        } else {
            if ((*buff >= '0' && *buff <= '9') || *buff == '-' ) {
                *pointer = *buff;
                pointer++;
            } else return 0;
        }

        buff++;
    }

    *pointer = 0;
    return (step == 2) ? 1 : 0;
}
