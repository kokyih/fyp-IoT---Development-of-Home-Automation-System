/*
 * This ESP32 code is created by esp32io.com
 *
 * This ESP32 code is released in the public domain
 *
 * For more detail (instruction and wiring diagram), visit https://esp32io.com/tutorials/esp32-rfid-nfc-door-lock-system
 */

#include <SPI.h>
#include <MFRC522.h>

#define BLYNK_TEMPLATE_ID "TMPLKTxxwmI1"
#define BLYNK_DEVICE_NAME "Door"

#define BLYNK_FIRMWARE_VERSION        "0.1.0"
#define BLYNK_PRINT Serial
#define APP_DEBUG

#include "BlynkEdgent.h"


#define SS_PIN    5  // ESP32 pin GIOP5 
#define RST_PIN   27 // ESP32 pin GIOP27 
#define RELAY_PIN 26 // ESP32 pin GIOP26 connects to relay

MFRC522 rfid(SS_PIN, RST_PIN);

byte keyTagUID[4] = {0x13, 0xB9, 0xFD, 0x16};//13 B9 FD 16
int door;
//83 85 2F 11 this for token

void setup() {
  Serial.begin(9600);
  SPI.begin(); // init SPI bus
  rfid.PCD_Init(); // init MFRC522
  pinMode(RELAY_PIN, OUTPUT); // initialize pin as an output.
  digitalWrite(RELAY_PIN, HIGH); // lock the door

  Serial.println("Tap an RFID/NFC tag on the RFID-RC522 reader");
  BlynkEdgent.begin();
}

void loop() {
  if (door == 1){
    Serial.println("Opening door");
    digitalWrite(RELAY_PIN, LOW);
    delay(5000);
    digitalWrite(RELAY_PIN, HIGH);
    Blynk.virtualWrite(V5, "0");
    door=0;
  }

  else if (rfid.PICC_IsNewCardPresent()) { // new tag is available
    if (rfid.PICC_ReadCardSerial()) { // NUID has been readed
      MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);

      if ((rfid.uid.uidByte[0] == keyTagUID[0] &&
          rfid.uid.uidByte[1] == keyTagUID[1] &&
          rfid.uid.uidByte[2] == keyTagUID[2] &&
          rfid.uid.uidByte[3] == keyTagUID[3])) {
        Serial.println("Access is granted");
        door=1;
        Blynk.virtualWrite(V5, "1");
        digitalWrite(RELAY_PIN, LOW);  // unlock the door for 2 seconds
        delay(5000);
        digitalWrite(RELAY_PIN, HIGH); // lock the door
        Blynk.virtualWrite(V5, "0");
        door =0;
      }
      else
      {
        Serial.print("Access denied, UID:");
        for (int i = 0; i < rfid.uid.size; i++) {
          Serial.print(rfid.uid.uidByte[i] < 0x10 ? " 0" : " ");
          Serial.print(rfid.uid.uidByte[i], HEX);
        }
        Serial.println();
      }

      rfid.PICC_HaltA(); // halt PICC
      rfid.PCD_StopCrypto1(); // stop encryption on PCD
    }
  }
  BlynkEdgent.run();
}

BLYNK_WRITE(V5)
{
  door = param.asInt();
}
