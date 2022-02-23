#include <Arduino.h>
#include <WiFi.h>

#include "wifi_config.h"

#include <UdpApi.h>

UdpApi api(5021);

void setup() {
    Serial.begin(115200);

    Serial.print("Connecting");

    WiFi.mode(WIFI_MODE_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASS);

    while (WiFi.status() != WL_CONNECTED) {
        delay(250);
        Serial.print(".");
    }

    Serial.println("\nWiFi connected");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());

    api.start();
}

void loop() {
    //
}
