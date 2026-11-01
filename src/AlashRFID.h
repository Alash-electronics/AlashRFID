// AlashRFID: считыватель карт и брелоков RC522 (MIFARE): UID строкой, список разрешённых карт, "учимся" новым картам,
// защита от повторного срабатывания, проверка подключения. Без зависимостей: драйвер RC522 внутри (общественное достояние).
// MIT License для кода AlashRFID. Alash Electronics, https://github.com/Alash-electronics/AlashRFID
#ifndef ALASH_RFID_H
#define ALASH_RFID_H

#include <Arduino.h>

// Читатель карт: на плате это драйвер RC522, в тестах без железа его заменяет имитация
class AlashRFIDReader {
 public:
  virtual ~AlashRFIDReader() {}
  virtual bool init() = 0;                       // инициализация микросхемы
  virtual uint8_t version() = 0;                 // регистр версии (0x91, 0x92 настоящие; 0x88, 0x12 клоны; 0x00 и 0xFF нет связи)
  virtual bool newCardPresent() = 0;             // в поле появилась карта
  virtual bool readSerial(uint8_t *uid, uint8_t &size, uint8_t &sak) = 0;   // прочитать UID (4, 7 или 10 байт) и SAK
  virtual void halt() = 0;                       // усыпить карту, пока её не уберут
};

class AlashRFID {
 public:
  enum Error : uint8_t {
    ERR_OK = 0,
    ERR_NO_READER,        // нет связи с RC522: SPI-провода, питание 3.3 В, пины SS и RST
    ERR_FAKE_CHIP,        // микросхема отвечает необычно (клон или сбои питания), но может работать
    ERR_LIST_FULL,        // список разрешённых карт заполнен
    ERR_BAD_UID           // строка UID записана неверно (нужны пары шестнадцатеричных цифр: "04:A1:B2:C3")
  };
  enum Language : uint8_t { LANG_RU = 0, LANG_KK = 1, LANG_EN = 2 };

  static const uint8_t MAX_CARDS = 16;           // сколько карт помещается в списке разрешённых

#if !defined(ALASH_RFID_NO_CORE)
  // ssPin: SDA/SS модуля, rstPin: RST (-1 если не подключён). Остальные пины SPI платы: Uno MOSI 11, MISO 12, SCK 13; ESP32 MOSI 23, MISO 19, SCK 18.
  AlashRFID(uint8_t ssPin, int8_t rstPin = -1);
#endif
  explicit AlashRFID(AlashRFIDReader &reader) : _r(&reader) {}
  ~AlashRFID();

  bool begin();                                  // запуск; false, если читатель не отвечает
#if (defined(ESP32) || defined(ESP8266)) && !defined(ALASH_RFID_NO_CORE)
  bool begin(int8_t sck, int8_t miso, int8_t mosi);   // свои пины SPI
#endif

  // ---- Чтение --------------------------------------------------------------
  bool available();                              // карта поднесена (true один раз за поднесение; повтор той же карты не раньше setCooldownMs)
  const uint8_t *uid() const { return _uid; }    // байты UID последней карты
  uint8_t uidSize() const { return _size; }
  const char *uidString() const { return _str; } // "04:A1:B2:C3"
  uint32_t uidNumber() const;                    // первые четыре байта числом (для коротких UID)
  const __FlashStringHelper *cardType() const;   // "MIFARE Classic 1K", "MIFARE Ultralight / NTAG"...
  void halt() { _r->halt(); }
  void setCooldownMs(uint16_t ms) { _cool = ms; }      // как долго не реагировать на ту же карту (по умолчанию 1000 мс)

  // ---- Список разрешённых карт ---------------------------------------------------------------
  bool addCard(const char *uidHex);              // "04:A1:B2:C3" (двоеточия, пробелы и дефисы можно опустить или оставить)
  bool addCard(const uint8_t *uid, uint8_t size);
  bool addCurrent();                             // добавить карту, которая только что считана
  bool removeCard(const char *uidHex);
  void clearCards() { _n = 0; }
  bool isAllowed();                              // текущая карта в списке?
  uint8_t cardCount() const { return _n; }
  void printCards(Print &out);                   // строки addCard("...") для вставки в скетч

  // ---- Диагностика -----------------------------------------------------------------
  Error error() const { return _err; }
  void setLanguage(Language lang) { _lang = lang; }
  const __FlashStringHelper *errorText();
  void diagnose(Print &out);

  static bool parseUid(const char *s, uint8_t *out, uint8_t &size);
  static void formatUid(const uint8_t *uid, uint8_t size, char *out);      // out минимум 3 * size байт

 private:
  AlashRFIDReader *_r;
  bool _own = false;
  uint8_t _uid[10] = {0}, _size = 0, _sak = 0;
  char _str[32] = {0};
  uint16_t _cool = 1000;
  uint32_t _lastAt = 0;
  uint8_t _lastUid[10] = {0}, _lastSize = 0;
  uint8_t _cards[MAX_CARDS][11];                 // [0] = длина, дальше байты UID
  uint8_t _n = 0;
  Error _err = ERR_OK;
  Language _lang = LANG_RU;
};

#endif  // ALASH_RFID_H
