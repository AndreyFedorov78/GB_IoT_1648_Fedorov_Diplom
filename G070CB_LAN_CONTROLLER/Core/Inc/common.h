#ifndef G070CB_LAN_CONTROLLER_COMMON_H
#define G070CB_LAN_CONTROLLER_COMMON_H




// размеры буферов
#define LOG_STRING_SIZE 600
#define RADIO_BUFFER_SIZE 50
#define MAX_LENGTH_OF_SITE_NAME 150
#define MAX_LENGTH_OF_MB_PROGPAM 680


typedef  struct MQTT_Packet {
    char topic[50];
    char message[10];
} MQTT_Packet;

typedef struct dev_settings{
    unsigned int device_type;
    uint64_t device_mac_address;
    char *device_ip;
    unsigned int mqtt_port;
    uint32_t rs485BaudRate;
    char mqtt_address[MAX_LENGTH_OF_SITE_NAME];
    char mqtt_client_name[MAX_LENGTH_OF_SITE_NAME];
    char mqtt_topic[MAX_LENGTH_OF_SITE_NAME];
    char mqtt_name[MAX_LENGTH_OF_SITE_NAME];
    char mqtt_password[MAX_LENGTH_OF_SITE_NAME];
    char scada_address[MAX_LENGTH_OF_SITE_NAME];
    char login[MAX_LENGTH_OF_SITE_NAME];
    char password[MAX_LENGTH_OF_SITE_NAME];
    char mb_program[MAX_LENGTH_OF_MB_PROGPAM];
    uint32_t uart_speed;

    uint8_t firstStart;
} dev_settings;

#endif //G070CB_LAN_CONTROLLER_COMMON_H
