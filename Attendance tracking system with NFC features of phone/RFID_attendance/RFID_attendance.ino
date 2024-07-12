#include <SPI.h>
#include <MFRC522.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>

#define RST_PIN  0     // Configurable, see typical pin layout above
#define SS_PIN   2     // Configurable, see typical pin layout above

MFRC522 mfrc522(SS_PIN, RST_PIN);  // Instance of the class
MFRC522::MIFARE_Key key;
ESP8266WiFiMulti WiFiMulti;

String data1 = "https://script.google.com/macros/s/AKfycbwnjuJrx1wNQSW_Q55wrgBexCeT87YvozP6hcxN5V1gxy7sFsWyCK5Zn4S5FhMyeFLiiQ/exec?uid=";

void setup() {
  Serial.begin(9600);

  WiFi.mode(WIFI_STA);
  WiFiMulti.addAP("iot", "project1234");

  SPI.begin();
  mfrc522.PCD_Init();
  delay(4);
  mfrc522.PCD_DumpVersionToSerial();
  Serial.println(F("Scan PICC to see UID, SAK, type, and data blocks..."));
}

void loop() {
  if (!mfrc522.PICC_IsNewCardPresent()) {
    return;
  }

  if (!mfrc522.PICC_ReadCardSerial()) {
    return;
  }

  byte bufferLen = 18;
  byte readBlockData[18];

  Serial.println();
  Serial.println(F("Reading UID from RFID..."));
  String uid = ReadUID();
  Serial.println(F("UID read successfully: "));
  Serial.println(uid);

  if ((WiFiMulti.run() == WL_CONNECTED)) {
    std::unique_ptr<BearSSL::WiFiClientSecure> client(new BearSSL::WiFiClientSecure);

    client->setInsecure();

    String url = data1 + uid;
    HTTPClient https;

    Serial.print(F("[HTTPS] begin...\n"));
    if (https.begin(*client, url)) {  
      Serial.print(F("[HTTPS] GET...\n"));
      int httpCode = https.GET();

      if (httpCode > 0) {
        Serial.printf("[HTTPS] GET... code: %d\n", httpCode);
      } else {
        Serial.printf("[HTTPS] GET... failed, error: %s\n", https.errorToString(httpCode).c_str());
      }
      https.end();
      delay(1000);
    } else {
      Serial.printf("[HTTPS] Unable to connect\n");
    }
  }
}

String ReadUID() {
  String uid = "";

  for (byte i = 0; i < mfrc522.uid.size; i++) {
    if (mfrc522.uid.uidByte[i] < 0x10) {
      uid += "0";
    }
    uid += String(mfrc522.uid.uidByte[i], HEX);
  }

  return uid;
}
