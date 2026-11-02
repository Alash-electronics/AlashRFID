// [ru] AlashRFID: «обучение» карт. Первая карта, поднесённая в течение 10 секунд после включения, считается главной;
//      потом любая карта, поднесённая сразу после главной, добавляется в список. Список печатается строками addCard(...)
//      для вставки в скетч (в памяти платы он не хранится после перезагрузки).
//      Подключение RC522 (питание ТОЛЬКО 3.3 В!):  Uno / Nano:  SDA(SS) D10, SCK D13, MOSI D11, MISO D12, RST D9, 3.3V, GND
//                                                ESP32:        SDA(SS) GPIO5, SCK GPIO18, MOSI GPIO23, MISO GPIO19, RST GPIO22, 3.3V, GND
// [kk] AlashRFID: карталарды «үйрету». Қосқаннан кейін 10 секунд ішінде жақындатылған бірінші карта басты болып саналады;
//      содан кейін басты картадан тікелей кейін жақындатылған кез келген карта тізімге қосылады. Тізім скетчке қою үшін addCard(...) жолдарымен
//      басылады (қайта жүктегеннен кейін тақта жадында сақталмайды).
//      RC522 қосу (қуат ТЕК 3.3 В!):  Uno / Nano:  SDA(SS) D10, SCK D13, MOSI D11, MISO D12, RST D9, 3.3V, GND
//                                    ESP32:        SDA(SS) GPIO5, SCK GPIO18, MOSI GPIO23, MISO GPIO19, RST GPIO22, 3.3V, GND
// [en] AlashRFID: "learning" cards. The first card presented within 10 seconds after power-on is the master;
//      then any card presented right after the master is added to the list. The list is printed as addCard(...) lines
//      to paste into the sketch (it is not kept in the board memory after a reboot).
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
bool learning = false;
uint32_t learnUntil = 0;

void setup() {
  Serial.begin(115200);
  if (!rfid.begin()) {
    Serial.println(rfid.errorText());
    while (true) delay(1000);
  }
  rfid.setCooldownMs(0);
  Serial.println("Поднесите главную карту...");
  while (!rfid.available()) delay(50);
  rfid.addCurrent();                       // главная карта
  Serial.print("Главная: ");
  Serial.println(rfid.uidString());
}

void loop() {
  if (!rfid.available()) return;
  if (rfid.isAllowed() && !learning) {     // главная карта включает обучение на 10 секунд
    learning = true;
    learnUntil = millis() + 10000;
    Serial.println("Обучение: подносите новые карты (10 секунд)");
  } else if (learning && millis() < learnUntil) {
    if (rfid.addCurrent()) {
      Serial.print("Добавлена: ");
      Serial.println(rfid.uidString());
    }
  } else {
    learning = false;
    Serial.println(rfid.isAllowed() ? "Известная карта" : "Неизвестная карта");
  }
  if (learning && millis() >= learnUntil) {
    learning = false;
    Serial.println("Обучение закончено. Список для скетча:");
    rfid.printCards(Serial);
  }
}
