#include <Arduino.h>
#include <WiFi.h>

#include "wifi_config.h"

#include <MultiCom.h>
#include <MultiComUdp.h>


MultiCom api(
    (MultiComChannel *) new MultiComUdp(5021)
);

void setup() {
    /*
    *   Set firmware id, device id and api version
    */
    String mac_str = WiFi.macAddress();
    mac_str.toLowerCase();
    mac_str.replace(':', '_');
    api.setDiscoveryResponse(
        "multicom_example",   // firmware identifier
        mac_str.c_str(),      // device id
        0                     // api version
    );

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

    api.tmp_callback = [](MultiComPacket packet, MultiComReplyFn reply) {
        Serial.println("-------------> User packet callback called!");
    };

    if (api.startAll()) {
        Serial.println("Started MultiCom");
    } else {
        Serial.println("Failed starting MultiCom");
    }
}

void loop() {
    //
}
