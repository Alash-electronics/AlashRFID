// [ru] AlashRFID: самый простой пример. Поднесите карту или брелок к RC522: в мониторе порта появится её номер (UID).
//      Подключение RC522 (питание ТОЛЬКО 3.3 В!):  Uno / Nano:  SDA(SS) D10, SCK D13, MOSI D11, MISO D12, RST D9, 3.3V, GND
//                                                ESP32:        SDA(SS) GPIO5, SCK GPIO18, MOSI GPIO23, MISO GPIO19, RST GPIO22, 3.3V, GND
// [kk] AlashRFID: ең қарапайым мысал. Картаны немесе брелокты RC522-ге жақындатыңыз: порт мониторында оның нөмірі (UID) шығады.
//      RC522 қосу (қуат ТЕК 3.3 В!):  Uno / Nano:  SDA(SS) D10, SCK D13, MOSI D11, MISO D12, RST D9, 3.3V, GND
//                                    ESP32:        SDA(SS) GPIO5, SCK GPIO18, MOSI GPIO23, MISO GPIO19, RST GPIO22, 3.3V, GND
// [en] AlashRFID: the simplest example. Bring a card or key fob to the RC522: its number (UID) appears in the serial monitor.
//      Wiring of the RC522 (power ONLY 3.3 V!):  Uno / Nano:  SDA(SS) D10, SCK D13, MOSI D11, MISO D12, RST D9, 3.3V, GND
//                                                ESP32:        SDA(SS) GPIO5, SCK GPIO18, MOSI GPIO23, MISO GPIO19, RST GPIO22, 3.3V, GND
#if defined(ESP32)
const uint8_t SS_PIN = 5, RST_PIN = 22;
#elif defined(ESP8266)
const uint8_t SS_PIN = 15, RST_PIN = 0;     // D8 и D3
#else
const uint8_t SS_PIN = 10, RST_PIN = 9;
#endif
#include <AlashRFID.h>

AlashRFID rfid(SS_PIN, RST_PIN);

void setup() {
  Serial.begin(115200);
  if (!rfid.begin()) {                     // проверяет, что RC522 вообще отвечает
    Serial.println(rfid.errorText());      // объяснит, что проверить (питание 3.3 В, провода SPI)
    while (true) delay(1000);
  }
  Serial.println("Поднесите карту...");
}

void loop() {
  if (rfid.available()) {                  // true один раз за поднесение карты (та же карта, оставленная у считывателя, не повторяется)
    Serial.print("UID: ");
    Serial.print(rfid.uidString());        // 04:A1:B2:C3
    Serial.print("   тип: ");
    Serial.println(rfid.cardType());
  }
}
