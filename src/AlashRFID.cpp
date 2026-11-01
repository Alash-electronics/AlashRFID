// MIT License. Alash Electronics.
#include "AlashRFID.h"

#define T(ru, kk, en) (_lang == LANG_KK ? F(kk) : _lang == LANG_EN ? F(en) : F(ru))

#if !defined(ALASH_RFID_NO_CORE)
#include <SPI.h>
#include "AlashRC522Core.h"

// Адаптер драйвера RC522 (AlashRC522Core) к интерфейсу читателя
class AlashRC522Reader : public AlashRFIDReader {
 public:
  AlashRC522Reader(uint8_t ss, int8_t rst) : _c(ss, rst < 0 ? (uint8_t)0xFF : (uint8_t)rst) {}   // 0xFF = пин RST не используется
  bool init() override { _c.PCD_Init(); return true; }
  uint8_t version() override { return _c.PCD_ReadRegister(AlashRC522Core::VersionReg); }
  bool newCardPresent() override { return _c.PICC_IsNewCardPresent(); }
  bool readSerial(uint8_t *uid, uint8_t &size, uint8_t &sak) override {
    if (!_c.PICC_ReadCardSerial()) return false;
    size = _c.uid.size;
    sak = _c.uid.sak;
    memcpy(uid, _c.uid.uidByte, size > 10 ? 10 : size);
    return true;
  }
  void halt() override { _c.PICC_HaltA(); _c.PCD_StopCrypto1(); }
 private:
  AlashRC522Core _c;
};

AlashRFID::AlashRFID(uint8_t ssPin, int8_t rstPin) : _r(new AlashRC522Reader(ssPin, rstPin)), _own(true) {}

#if defined(ESP32) || defined(ESP8266)
bool AlashRFID::begin(int8_t sck, int8_t miso, int8_t mosi) {
#if defined(ESP32)
  SPI.begin(sck, miso, mosi);
#else
  SPI.begin();
  (void)sck; (void)miso; (void)mosi;
#endif
  return begin();
}
#endif
#endif  // ALASH_RFID_NO_CORE

AlashRFID::~AlashRFID() {
  if (_own) delete _r;
}

bool AlashRFID::begin() {
#if !defined(ALASH_RFID_NO_CORE)
  if (_own) SPI.begin();
#endif
  _r->init();
  delay(50);
  uint8_t v = _r->version();
  if (v == 0x00 || v == 0xFF) {                  // шина не отвечает: нулевые или единичные байты
    _err = ERR_NO_READER;
    return false;
  }
  _err = (v == 0x91 || v == 0x92) ? ERR_OK : ERR_FAKE_CHIP;     // 0x88 / 0x12 это клоны: работают, но это стоит знать
  return true;
}

// ---- Формат UID ----------------------------------------------------------------------------------

void AlashRFID::formatUid(const uint8_t *uid, uint8_t size, char *out) {
  static const char hex[] = "0123456789ABCDEF";
  uint8_t p = 0;
  for (uint8_t i = 0; i < size; i++) {
    if (i) out[p++] = ':';
    out[p++] = hex[uid[i] >> 4];
    out[p++] = hex[uid[i] & 15];
  }
  out[p] = 0;
}

static int hexVal(char c) {
  if (c >= '0' && c <= '9') return c - '0';
  if (c >= 'a' && c <= 'f') return c - 'a' + 10;
  if (c >= 'A' && c <= 'F') return c - 'A' + 10;
  return -1;
}

bool AlashRFID::parseUid(const char *s, uint8_t *out, uint8_t &size) {
  size = 0;
  int hi = -1;
  for (; *s; s++) {
    if (*s == ':' || *s == ' ' || *s == '-') {
      if (hi >= 0) return false;                  // разделитель посреди байта
      continue;
    }
    int v = hexVal(*s);
    if (v < 0) return false;
    if (hi < 0) hi = v;
    else {
      if (size >= 10) return false;
      out[size++] = (uint8_t)(hi * 16 + v);
      hi = -1;
    }
  }
  return hi < 0 && (size == 4 || size == 7 || size == 10);        // UID бывает только 4, 7 или 10 байт
}

// ---- Чтение ---------------------------------------------------------------------------------------------

bool AlashRFID::available() {
  if (!_r->newCardPresent()) return false;
  uint8_t uid[10], size = 0, sak = 0;
  if (!_r->readSerial(uid, size, sak)) return false;
  uint32_t now = millis();
  // Та же карта, оставленная у считывателя, не должна срабатывать каждый опрос
  bool same = size == _lastSize && memcmp(uid, _lastUid, size) == 0;
  if (same && _lastAt && (uint32_t)(now - _lastAt) < _cool) {
    _r->halt();
    return false;
  }
  memcpy(_uid, uid, size);
  memcpy(_lastUid, uid, size);
  _size = _lastSize = size;
  _sak = sak;
  _lastAt = now;
  formatUid(_uid, _size, _str);
  _r->halt();                                    // карта "засыпает" до следующего появления в поле
  return true;
}

uint32_t AlashRFID::uidNumber() const {
  uint32_t v = 0;
  for (uint8_t i = 0; i < 4 && i < _size; i++) v = (v << 8) | _uid[i];
  return v;
}

const __FlashStringHelper *AlashRFID::cardType() const {
  switch (_sak & 0x7F) {
    case 0x09: return F("MIFARE Mini");
    case 0x08: return F("MIFARE Classic 1K");
    case 0x18: return F("MIFARE Classic 4K");
    case 0x00: return F("MIFARE Ultralight / NTAG");
    case 0x10: case 0x11: return F("MIFARE Plus");
    case 0x20: return F("MIFARE DESFire / ISO 14443-4");
    case 0x04: return T("UID не полный", "UID толық емес", "UID not complete");
    default: return T("неизвестный тип", "белгісіз түрі", "unknown type");
  }
}

// ---- Список разрешённых карт --------------------------------------------------------------------------------

bool AlashRFID::addCard(const uint8_t *uid, uint8_t size) {
  if (size != 4 && size != 7 && size != 10) { _err = ERR_BAD_UID; return false; }
  for (uint8_t i = 0; i < _n; i++)
    if (_cards[i][0] == size && memcmp(_cards[i] + 1, uid, size) == 0) return true;      // уже есть
  if (_n >= MAX_CARDS) { _err = ERR_LIST_FULL; return false; }
  _cards[_n][0] = size;
  memcpy(_cards[_n] + 1, uid, size);
  _n++;
  _err = ERR_OK;
  return true;
}

bool AlashRFID::addCard(const char *uidHex) {
  uint8_t b[10], n;
  if (!parseUid(uidHex, b, n)) { _err = ERR_BAD_UID; return false; }
  return addCard(b, n);
}

bool AlashRFID::addCurrent() { return _size ? addCard(_uid, _size) : false; }

bool AlashRFID::removeCard(const char *uidHex) {
  uint8_t b[10], n;
  if (!parseUid(uidHex, b, n)) { _err = ERR_BAD_UID; return false; }
  for (uint8_t i = 0; i < _n; i++) {
    if (_cards[i][0] == n && memcmp(_cards[i] + 1, b, n) == 0) {
      for (uint8_t j = i; j + 1 < _n; j++) memcpy(_cards[j], _cards[j + 1], 11);
      _n--;
      return true;
    }
  }
  return false;
}

bool AlashRFID::isAllowed() {
  for (uint8_t i = 0; i < _n; i++)
    if (_cards[i][0] == _size && memcmp(_cards[i] + 1, _uid, _size) == 0) return true;
  return false;
}

void AlashRFID::printCards(Print &out) {
  char s[32];
  for (uint8_t i = 0; i < _n; i++) {
    formatUid(_cards[i] + 1, _cards[i][0], s);
    out.print(F("rfid.addCard(\""));
    out.print(s);
    out.println(F("\");"));
  }
}

// ---- Диагностика -------------------------------------------------------------------------------------------------

const __FlashStringHelper *AlashRFID::errorText() {
  switch (_err) {
    case ERR_OK: return F("OK");
    case ERR_NO_READER:
      return T("Нет связи с RC522. Проверьте: питание только 3.3 В (не 5 В!), все провода SPI (SDA/SS, SCK, MOSI, MISO) на нужные пины платы, пины SS и RST указаны верно, GND общий",
               "RC522-мен байланыс жоқ. Тексеріңіз: қуат тек 3.3 В (5 В емес!), барлық SPI сымдары (SDA/SS, SCK, MOSI, MISO), SS және RST пиндері, ортақ GND",
               "No link to the RC522. Check: power is 3.3 V only (not 5 V!), all SPI wires (SDA/SS, SCK, MOSI, MISO) on the right board pins, the SS and RST pins are right, a common GND");
    case ERR_FAKE_CHIP:
      return T("Микросхема отвечает необычной версией (клон RC522). Обычно работает; если карты читаются плохо, проверьте питание и поднесите карту ближе",
               "Микросхема әдеттен тыс нұсқа береді (RC522 клоны). Әдетте жұмыс істейді; карта нашар оқылса, қуатты тексеріңіз",
               "The chip reports an unusual version (an RC522 clone). It usually works; if cards read badly, check the power and bring the card closer");
    case ERR_LIST_FULL:
      return T("Список разрешённых карт заполнен (до 16 карт)", "Рұқсат етілген карталар тізімі толды (16 картаға дейін)", "The allowed-card list is full (up to 16 cards)");
    case ERR_BAD_UID:
      return T("UID записан неверно: нужны пары шестнадцатеричных цифр на 4, 7 или 10 байт, например \"04:A1:B2:C3\"",
               "UID қате жазылған: 4, 7 не 10 байт он алтылық жұптар керек, мысалы \"04:A1:B2:C3\"",
               "The UID is malformed: it needs hexadecimal pairs for 4, 7 or 10 bytes, for example \"04:A1:B2:C3\"");
  }
  return F("");
}

void AlashRFID::diagnose(Print &out) {
  out.println(T("AlashRFID: диагностика", "AlashRFID: диагностика", "AlashRFID: diagnostics"));
  bool ok = begin();
  uint8_t v = _r->version();
  out.print(T("Версия микросхемы: 0x", "Микросхема нұсқасы: 0x", "Chip version: 0x"));
  out.println((int)v);
  if (!ok) {
    out.println(errorText());
    return;
  }
  out.println(_err == ERR_OK ? T("RC522 найден и отвечает.", "RC522 табылды.", "RC522 found and answering.") : errorText());
  out.println(T("Поднесите карту (ждём 10 секунд)...", "Картаны жақындатыңыз (10 секунд күтеміз)...", "Bring a card close (waiting 10 seconds)..."));
  uint32_t t0 = millis();
  while ((uint32_t)(millis() - t0) < 10000) {
    if (available()) {
      out.print(T("Карта: ", "Карта: ", "Card: "));
      out.print(uidString());
      out.print(F("  "));
      out.println(cardType());
      return;
    }
    delay(50);
  }
  out.println(T("Карта не прочиталась. Поднесите вплотную к антенне (до 3 см), уберите металл рядом, попробуйте другую карту; метки на 125 кГц (EM4100) этот считыватель не читает: нужен RC522 на 13.56 МГц.",
                "Карта оқылмады. Антеннаға тақап жақындатыңыз (3 см-ге дейін), жақын жердегі металды алыңыз; 125 кГц (EM4100) белгілерін бұл оқу құрылғысы оқымайды: 13.56 МГц керек.",
                "No card was read. Hold it right against the antenna (within 3 cm), remove metal nearby, try another card; 125 kHz (EM4100) tags are not read by this reader: it needs 13.56 MHz."));
}
