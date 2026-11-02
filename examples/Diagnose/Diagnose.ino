// [ru] AlashRFID: считыватель не работает? Скетч проверит связь с RC522, версию микросхемы, подождёт карту и подскажет, что не так.
//      Подключение RC522 (питание ТОЛЬКО 3.3 В!):  Uno / Nano:  SDA(SS) D10, SCK D13, MOSI D11, MISO D12, RST D9, 3.3V, GND
//                                                ESP32:        SDA(SS) GPIO5, SCK GPIO18, MOSI GPIO23, MISO GPIO19, RST GPIO22, 3.3V, GND
// [kk] AlashRFID: оқу құрылғысы жұмыс істемей ме? Скетч RC522-мен байланысты, микросхема нұсқасын тексереді, картаны күтеді және не болғанын айтады.
//      RC522 қосу (қуат ТЕК 3.3 В!):  Uno / Nano:  SDA(SS) D10, SCK D13, MOSI D11, MISO D12, RST D9, 3.3V, GND
//                                    ESP32:        SDA(SS) GPIO5, SCK GPIO18, MOSI GPIO23, MISO GPIO19, RST GPIO22, 3.3V, GND
// [en] AlashRFID: the reader does not work? The sketch checks the link to the RC522, the chip version, waits for a card and tells what is wrong.
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
  delay(500);
  rfid.setLanguage(AlashRFID::LANG_RU);    // LANG_KK: қазақша, LANG_EN: English
  rfid.diagnose(Serial);
}

void loop() {}
