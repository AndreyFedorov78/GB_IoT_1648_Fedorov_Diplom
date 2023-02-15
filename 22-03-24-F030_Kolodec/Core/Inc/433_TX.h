#ifndef TX_433_h
#define TX_433_h


// скорость передачи данных
#define D433_SPEED 2400

// порт и пин передачи данных
#ifndef D433_TX_Pin
#define D433_TX_Pin GPIO_PIN_4
#endif
#ifndef D433_TX_GPIO_Port
#define D433_TX_GPIO_Port GPIOA
#endif


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
// количество синхроимульсов
#define TRAINING_PULSES 50

#define D433_BUFF_SIZE 100 // размер буфера без учета контрольной суммы





char d433_buff[D433_BUFF_SIZE+1];  // необязательно использовать это буфер





void delay_micros(uint32_t us)
{
    TIM3->CNT= 0; while(TIM3->CNT < us) {}
}



uint8_t D433_crc8(char *buffer, uint8_t size)
{
    uint8_t crc = 0;
    for (uint8_t i = 0; i < size; i++){
        uint8_t j = 8;
        uint8_t data = buffer[i];
        while (j--) {
            crc = ((crc ^ data) & 1) ? (crc >> 1) ^ 0x8C : (crc >> 1);
            data >>= 1;
        }
    }
    return crc;
}



void D433_write(char* buf, uint16_t size)
{
    for (uint16_t i = 0; i < TRAINING_PULSES; i++) {
    	HAL_GPIO_WritePin(D433_TX_GPIO_Port,D433_TX_Pin,GPIO_PIN_SET);
    	delay_micros(FRAME_TIME);
    	HAL_GPIO_WritePin(D433_TX_GPIO_Port,D433_TX_Pin,GPIO_PIN_RESET);
    	delay_micros(FRAME_TIME);
    }
    HAL_GPIO_WritePin(D433_TX_GPIO_Port,D433_TX_Pin,GPIO_PIN_SET);
    delay_micros(START_PULSE);    // ждём
    HAL_GPIO_WritePin(D433_TX_GPIO_Port,D433_TX_Pin,GPIO_PIN_RESET);

      for (uint16_t n = 0; n < size; n++) {
        uint8_t data = buf[n];
        for (uint8_t b = 0; b < 8; b++) {           //короткий импульс 0 длинный 1
            if (data & 1) delay_micros(FRAME_TIME);
            else delay_micros(HALF_FRAME);
            // HAL_GPIO_WritePin(D433_TX_GPIO_Port,D433_TX_Pin,!(b%2));
             HAL_GPIO_TogglePin(D433_TX_GPIO_Port,D433_TX_Pin);
            data >>= 1;
        }
      }
      delay_micros(FRAME_TIME*100);
}

int D433_sendData(char * buffer, uint16_t size)
{
	if (size > D433_BUFF_SIZE) return 0;
    buffer[size] = D433_crc8(buffer, size);
    for (int i=0; i<5; i++)
    D433_write(buffer, size+1);
    return 1;
}



#endif
