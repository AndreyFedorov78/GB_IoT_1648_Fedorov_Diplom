#include "functions.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "common.h"
#include "stm32g070xx.h"

extern dev_settings system_settings;
extern osSemaphoreId SemSaveSettingsHandle;
extern char log_string[LOG_STRING_SIZE];

void SaveSettingsTask(void const *argument) {
	settings_init(&system_settings);

	for (;;) {
		if (pdPASS == xSemaphoreTake(SemSaveSettingsHandle, portMAX_DELAY)){
        /*pdPASS*/
		uint32_t error = 0;

		static FLASH_EraseInitTypeDef EraseInitStruct;
		//EraseInitStruct.TypeErase = 0;
		EraseInitStruct.TypeErase = FLASH_TYPEERASE_PAGES;
		//EraseInitStruct.TypeErase = FLASH_TYPEERASE_MASS;
		EraseInitStruct.Banks = 0;
		EraseInitStruct.Page = SETTINGS_FLASH_PAGE;
		EraseInitStruct.NbPages = 1;

		HAL_FLASH_Unlock();
		HAL_StatusTypeDef rc= HAL_FLASHEx_Erase(&EraseInitStruct, &error);
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

	}
	}
	__NOP();
}
