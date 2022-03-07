#include <Arduino.h>
#include <WiFi.h>

#include "wifi_config.h"

#include <MultiCom.h>
#include <MultiComUdp.h>


MultiCom api(
    (MultiComChannel *) new MultiComUdp(5021)
);

int example_val = 0;

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

    api.onGet("example", [](MultiComPacket packet, MultiComReplyFn reply) {
        Serial.println((char*)packet.user_data);
        reply((void*)"yes", 3);
    });

    api.onGet("getval", [](MultiComPacket packet, MultiComReplyFn reply) {
        char res_msg[5] = {0};
        sprintf(res_msg, "%d", example_val);
        reply((void*)res_msg, strlen(res_msg));
    });

    api.onSend("setval", [](MultiComPacket packet) {
        example_val = atoi((char*)packet.user_data);
    });

    api.onPost("setval", [](MultiComPacket packet) {
        example_val = atoi((char*)packet.user_data);
    });

    if (api.startAll()) {
        Serial.println("Started MultiCom");
    } else {
        Serial.println("Failed starting MultiCom");
    }
}

void loop() {
    //
}
