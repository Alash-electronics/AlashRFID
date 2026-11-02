// [ru] AlashRFID: замок по карте. Разрешённые карты записаны в списке; при верной карте включается реле (замок) на 3 секунды,
//      светодиод горит зелёным, при чужой коротко красным. Реле на пине 7 (активный LOW), светодиоды: зелёный 4, красный 3.
//      Номера своих карт узнайте в примере Basic и впишите в addCard().
//      Подключение RC522 (питание ТОЛЬКО 3.3 В!):  Uno / Nano:  SDA(SS) D10, SCK D13, MOSI D11, MISO D12, RST D9, 3.3V, GND
//                                                ESP32:        SDA(SS) GPIO5, SCK GPIO18, MOSI GPIO23, MISO GPIO19, RST GPIO22, 3.3V, GND
// [kk] AlashRFID: карта бойынша құлып. Рұқсат етілген карталар тізімде жазылған; дұрыс карта болса реле (құлып) 3 секундқа қосылады,
//      жарықдиод жасыл жанады, бөтен картада қысқа қызыл. Реле 7 пинде (белсенді LOW), жарықдиодтар: жасыл 4, қызыл 3.
//      Өз карталарыңыздың нөмірін Basic мысалынан біліп, addCard() ішіне жазыңыз.
//      RC522 қосу (қуат ТЕК 3.3 В!):  Uno / Nano:  SDA(SS) D10, SCK D13, MOSI D11, MISO D12, RST D9, 3.3V, GND
//                                    ESP32:        SDA(SS) GPIO5, SCK GPIO18, MOSI GPIO23, MISO GPIO19, RST GPIO22, 3.3V, GND
// [en] AlashRFID: a card lock. The allowed cards are written in a list; with a valid card the relay (lock) switches on for 3 seconds,
//      the LED lights green, with a wrong one briefly red. The relay on pin 7 (active LOW), LEDs: green 4, red 3.
//      Find your card numbers in the Basic example and put them into addCard().
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
const uint8_t RELAY = 7, GREEN = 4, RED = 3;
uint32_t openUntil = 0;

void setup() {
  Serial.begin(115200);
  digitalWrite(RELAY, HIGH);               // реле выключено (активный LOW), потом OUTPUT: без щелчка при старте
  pinMode(RELAY, OUTPUT);
  pinMode(GREEN, OUTPUT);
  pinMode(RED, OUTPUT);
  if (!rfid.begin()) {
    Serial.println(rfid.errorText());
    while (true) delay(1000);
  }
  rfid.addCard("04:A1:B2:C3");             // впишите свои карты
  rfid.addCard("DE:AD:BE:EF");
}

void loop() {
  if (rfid.available()) {
    if (rfid.isAllowed()) {
      Serial.print("Открыто: ");
      Serial.println(rfid.uidString());
      openUntil = millis() + 3000;
    } else {
      Serial.print("Чужая карта: ");
      Serial.println(rfid.uidString());
      digitalWrite(RED, HIGH);
      delay(300);
      digitalWrite(RED, LOW);
    }
  }
  bool open = (int32_t)(millis() - openUntil) < 0;
  digitalWrite(RELAY, open ? LOW : HIGH);
  digitalWrite(GREEN, open);
}
