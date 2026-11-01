// Тесты AlashRFID без железа: имитация читателя карт (драйвер RC522 внутри библиотеки общественного достояния и тут не проверяется).
//   g++ -std=c++17 -Wall -DALASH_RFID_NO_CORE -I mock -I ../../src test_rfid.cpp ../../src/AlashRFID.cpp && ./a.out
#include <stdio.h>
#include "Arduino.h"
#include "AlashRFID.h"

static int fails = 0;
#define CHECK(cond) do { if (!(cond)) { printf("FAIL line %d: %s\n", __LINE__, #cond); fails++; } } while (0)

struct FakeReader : public AlashRFIDReader {
  uint8_t ver = 0x92; bool present = false; uint8_t uid[10] = {0}; uint8_t size = 4, sak = 0x08; bool readFails = false; int halts = 0, inits = 0;
  bool init() override { inits++; return true; }
  uint8_t version() override { return ver; }
  bool newCardPresent() override { return present; }
  bool readSerial(uint8_t *u, uint8_t &s, uint8_t &k) override { if (readFails) return false; memcpy(u, uid, 10); s = size; k = sak; return true; }
  void halt() override { halts++; }
  void card(std::initializer_list<int> b, uint8_t sk = 0x08) { size = 0; for (int x : b) uid[size++] = (uint8_t)x; sak = sk; present = true; }
};

int main() {
  // 1) запуск: настоящий чип, клон, нет связи (шина вернула все нули или все единицы)
  FakeReader r; AlashRFID rf(r);
  CHECK(rf.begin() && rf.error() == AlashRFID::ERR_OK && r.inits == 1);
  r.ver = 0x88; CHECK(rf.begin() && rf.error() == AlashRFID::ERR_FAKE_CHIP);
  r.ver = 0x12; CHECK(rf.begin() && rf.error() == AlashRFID::ERR_FAKE_CHIP);
  r.ver = 0x00; CHECK(!rf.begin() && rf.error() == AlashRFID::ERR_NO_READER);
  r.ver = 0xFF; CHECK(!rf.begin() && rf.error() == AlashRFID::ERR_NO_READER);
  r.ver = 0x92; rf.begin();

  // 2) чтение: нет карты, поднесли, строка UID и число, тип карты
  CHECK(!rf.available());
  r.card({0x04, 0xA1, 0xB2, 0xC3});
  CHECK(rf.available() && strcmp(rf.uidString(), "04:A1:B2:C3") == 0 && rf.uidSize() == 4 && rf.uidNumber() == 0x04A1B2C3UL && r.halts == 1);
  CHECK(strcmp((const char *)rf.cardType(), "MIFARE Classic 1K") == 0);
  CHECK(rf.uid()[0] == 0x04 && rf.uid()[3] == 0xC3);

  // 3) та же карта у считывателя: во время паузы не срабатывает, после паузы срабатывает; другая карта сразу
  g_ms += 300; CHECK(!rf.available());
  g_ms += 500; CHECK(!rf.available());
  g_ms += 400; CHECK(rf.available());
  r.card({0xDE, 0xAD, 0xBE, 0xEF}); g_ms += 10; CHECK(rf.available() && strcmp(rf.uidString(), "DE:AD:BE:EF") == 0);
  r.card({0x04, 0xA1, 0xB2, 0xC3}); g_ms += 10; CHECK(rf.available());                 // вернулась первая, а до этого была другая: срабатывает
  rf.setCooldownMs(0); g_ms += 1; CHECK(rf.available() && rf.available());               // без паузы каждый опрос
  rf.setCooldownMs(1000);

  // 4) карты с 7-байтным UID, разные типы SAK
  r.card({0x04, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66}, 0x00); g_ms += 10;
  CHECK(rf.available() && rf.uidSize() == 7 && strcmp(rf.uidString(), "04:11:22:33:44:55:66") == 0 && strcmp((const char *)rf.cardType(), "MIFARE Ultralight / NTAG") == 0);
  r.card({1, 2, 3, 4}, 0x18); g_ms += 10; rf.available(); CHECK(strcmp((const char *)rf.cardType(), "MIFARE Classic 4K") == 0);
  r.card({1, 2, 3, 5}, 0x20); g_ms += 10; rf.available(); CHECK(strcmp((const char *)rf.cardType(), "MIFARE DESFire / ISO 14443-4") == 0);
  r.readFails = true; r.card({9, 9, 9, 9}); g_ms += 10; CHECK(!rf.available()); r.readFails = false;       // не смог прочитать UID: не срабатывает

  // 5) список разрешённых карт: строки с разделителями и без, регистр, дубли, повторное добавление
  AlashRFID acl(r);
  CHECK(acl.addCard("04:A1:B2:C3") && acl.addCard("de-ad-be-ef") && acl.addCard("11 22 33 44") && acl.addCard("AABBCCDD") && acl.cardCount() == 4);
  CHECK(acl.addCard("04:a1:b2:c3") && acl.cardCount() == 4);                              // тот же UID в другом написании это дубль
  r.card({0x04, 0xA1, 0xB2, 0xC3}); g_ms += 2000; CHECK(acl.available() && acl.isAllowed());
  r.card({0xDE, 0xAD, 0xBE, 0xEF}); g_ms += 10; CHECK(acl.available() && acl.isAllowed());
  r.card({0x01, 0x02, 0x03, 0x04}); g_ms += 10; CHECK(acl.available() && !acl.isAllowed());
  CHECK(acl.addCurrent() && acl.isAllowed() && acl.cardCount() == 5);
  CHECK(acl.removeCard("11:22:33:44") && acl.cardCount() == 4 && !acl.removeCard("11:22:33:44"));
  r.card({0x11, 0x22, 0x33, 0x44}); g_ms += 10; acl.available(); CHECK(!acl.isAllowed());
  // 7-байтная карта в списке не совпадает с 4-байтной с теми же первыми байтами
  CHECK(acl.addCard("04:A1:B2:C3:D4:E5:F6")); r.card({0x04, 0xA1, 0xB2, 0xC3}); g_ms += 2000; acl.available(); CHECK(acl.isAllowed());
  r.card({0x04, 0xA1, 0xB2, 0xC3, 0xD4, 0xE5, 0xF6}); g_ms += 10; acl.available(); CHECK(acl.isAllowed() && acl.uidSize() == 7);
  acl.clearCards(); CHECK(acl.cardCount() == 0 && !acl.isAllowed());

  // 8) ошибки разбора и переполнение списка
  AlashRFID er(r);
  CHECK(!er.addCard("04:A1:B2") && er.error() == AlashRFID::ERR_BAD_UID);                 // 3 байта
  CHECK(!er.addCard("04:A1:B2:C") && !er.addCard("ZZ:A1:B2:C3") && !er.addCard("") && !er.addCard("04A1B2C3D4"));
  uint8_t u[4] = {0, 0, 0, 0};
  for (int i = 0; i < 16; i++) { u[3] = i; CHECK(er.addCard(u, 4)); }
  u[3] = 99; CHECK(!er.addCard(u, 4) && er.error() == AlashRFID::ERR_LIST_FULL && er.cardCount() == 16);
  CHECK(!er.addCard(u, 5));                                                                 // 5 байт не бывает

  // 9) печать списка и формат
  AlashRFID pr(r); pr.addCard("04:A1:B2:C3"); pr.addCard("DEADBEEF"); Print out; pr.printCards(out);
  CHECK(out.log == "rfid.addCard(\"04:A1:B2:C3\");\nrfid.addCard(\"DE:AD:BE:EF\");\n");
  char s[32]; uint8_t b[3] = {0x00, 0x0F, 0xFF}; AlashRFID::formatUid(b, 3, s); CHECK(strcmp(s, "00:0F:FF") == 0);

  // 10) diagnose: нет читателя, есть читатель с картой, есть читатель без карты
  FakeReader nr; nr.ver = 0x00; AlashRFID d1(nr); Print o1; d1.diagnose(o1); CHECK(o1.log.find("3.3 В") != std::string::npos);
  FakeReader dr; dr.card({0x04, 0xA1, 0xB2, 0xC3}); AlashRFID d2(dr); Print o2; d2.diagnose(o2); CHECK(o2.log.find("04:A1:B2:C3") != std::string::npos);
  FakeReader nc; AlashRFID d3(nc); d3.setLanguage(AlashRFID::LANG_EN); Print o3; d3.diagnose(o3); CHECK(o3.log.find("No card was read") != std::string::npos);
  d3.setLanguage(AlashRFID::LANG_KK); CHECK(strlen((const char *)d3.errorText()) > 1);

  if (fails) { printf("FAILED: %d\n", fails); return 1; }
  printf("ALL TESTS PASSED\n");
  return 0;
}
