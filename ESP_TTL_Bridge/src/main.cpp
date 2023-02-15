#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <string>
#include "keys.h"
#include "wifi-pass.h"

// топики личного MQTT и Яндекса
const char *head_topic_tl = "my_scada";
const char *head_topic_ya = "gb_iot/1648__FAN";

// Данные WIFI
const char *ssid = "F***************";
const char *password = WIFI_PASS;

// Яндекс книент на отправку сообщенией
WiFiClientSecure espClientPub;
PubSubClient client_pub(espClientPub);

// Яндекс книент на приемку сообщенией
WiFiClientSecure espClientSub;
PubSubClient client_sub(espClientSub);


// Клиент моей системы
WiFiClient espClient;
PubSubClient client_tl(espClient);


// Прототипы

// Callback Яндекса
void receivedCallback(char *topic, byte *payload, unsigned int length);

// Callback моего MQTT
void receivedCallbackTL(char *topic, byte *payload, unsigned int length);

// Установка связи с MQTT
void mqttconnect(PubSubClient *client, const char *topic);


void setup() {

    Serial.begin(115200);
    Serial.println();
// подключаемся к WiFi
    Serial.print("Connecting to ");
    Serial.println(ssid);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println();

    // Устанавливаем сертификаты на публикацию
    espClientPub.setCACert(ca_cert);
    espClientPub.setCertificate(cert_devices);
    espClientPub.setPrivateKey(key_devices);

    // Устанавливаем сертификаты на подписку
    espClientSub.setCACert(ca_cert);
    espClientSub.setCertificate(cert_registries);
    espClientSub.setPrivateKey(key_registries);

    // Сервера MQTT
    client_pub.setServer("mqtt.cloud.yandex.net", 8883);
    client_sub.setServer("mqtt.cloud.yandex.net", 8883);
    client_tl.setServer("***********.ru", 1883);    /// *********!!!!!!!!!!

    // Привязываем callback'и
    client_sub.setCallback(receivedCallback);
    client_tl.setCallback(receivedCallbackTL);

}

void loop() { //побежали

    // Коннектим все три клиента
    if (!client_sub.connected()) { mqttconnect(&client_sub, "gb_iot/1648__FAN/RX/#"); }
    if (!client_tl.connected()) { mqttconnect(&client_tl, "my_scada/#"); }
    if (!client_pub.connected()) { mqttconnect(&client_pub, "0"); }

    // И поддерживаем связь с сервером
    client_sub.loop();
    client_pub.loop();
    client_tl.loop();
    delay(50);

}

// Всю информацию (не команды) из внутреннего MQTT форвардим в Яндекс
void receivedCallbackTL(char *topic, byte *payload, unsigned int length) {
    char new_topic[200];
    char msg[200];
    strncpy(msg, (char *) payload, length);
    msg[length]=0;
    snprintf(new_topic, 200, "%s%s", head_topic_ya, (topic + strlen(head_topic_tl)));
    client_pub.publish(new_topic, msg);
}


// Команды из Яндекса форвардим во внутреннюю сеть
void receivedCallback(char *topic, byte *payload, unsigned int length) {
    char new_topic[200];
    char msg[200];
    strncpy(msg, (char *) payload, length);
    msg[length]=0;
    snprintf(new_topic, 200, "%s%s", head_topic_tl, (topic + strlen(head_topic_ya) + 1));
    client_tl.publish(new_topic, msg);

}


void mqttconnect(PubSubClient *client, const char *topic) {
    while (!client->connected()) {
        Serial.print("MQTT connecting ...");
        String clientId = "ESP32Client";
        if (client->connect(clientId.c_str())) {
            Serial.println("connected");
            if (strlen(topic) > 1) {
                client->subscribe(topic);

            }
        } else {
            Serial.print("failed, status code =");
            Serial.print(client->state());
            Serial.println("try again in 5 seconds");
            delay(5000);
        }
    }
}
