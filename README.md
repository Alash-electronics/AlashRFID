# AlashRFID

[Русский](#русский) | [Қазақша](#қазақша) | [English](#english)

---

## Русский

Простая библиотека для **считывателя карт RC522** (MIFARE, брелоки, карты из набора): `rfid.available()` даёт номер карты (UID) один раз за поднесение, а `rfid.isAllowed()` проверяет, есть ли карта в вашем **списке разрешённых**, например для замка. Проверяет, что RC522 вообще отвечает, и подсказывает, что исправить (питание 3.3 В, провода SPI). Драйвер RC522 внутри: отдельная библиотека MFRC522 **не нужна**. Сделана для уроков и наборов Alash Electronics.

```cpp
#include <AlashRFID.h>
AlashRFID rfid(10, 9);                    // SS, RST

void setup() {
  Serial.begin(115200);
  if (!rfid.begin()) { Serial.println(rfid.errorText()); while (true); }
  rfid.addCard("04:A1:B2:C3");           // разрешённая карта / allowed card
}
void loop() {
  if (rfid.available()) {
    Serial.println(rfid.uidString());     // 04:A1:B2:C3
    Serial.println(rfid.isAllowed() ? "Открыто" : "Чужая");
  }
}
```

### Что умеет
- **Карта одной строкой:** `available()` срабатывает один раз за поднесение; `uidString()` даёт `04:A1:B2:C3`, `uidNumber()` число, `cardType()` тип.
- **Не повторяется:** та же карта, забытая у считывателя, не срабатывает каждый опрос, а раз в `setCooldownMs()` (по умолчанию секунда); другая карта сразу.
- **Список разрешённых карт:** `addCard("04:A1:B2:C3")` (с двоеточиями, пробелами или без), `isAllowed()`, `addCurrent()`, `removeCard()`, `printCards()`; до 16 карт; UID на 4, 7 и 10 байт.
- **Проверка связи:** `begin()` читает версию RC522; нет связи (питание, провода SPI) это `ERR_NO_READER` с подсказкой, клон это `ERR_FAKE_CHIP`, но он обычно работает.
- **Встроенный драйвер** RC522 (общественное достояние): ставить отдельно MFRC522 не нужно, и конфликта с ней нет.
- **Понятные подсказки** на трёх языках: `errorText()`, `diagnose(Serial)` проверяет микросхему и ждёт карту.
- Работает на **Arduino AVR, ESP32, ESP8266**; свои пины SPI на ESP32: `begin(sck, miso, mosi)`.

### Подключение
| RC522 | Uno / Nano | ESP32 |
|---|---|---|
| 3.3V | 3.3 В (НЕ 5 В) | 3.3 В |
| GND | GND | GND |
| SDA (SS) | D10 | GPIO5 |
| SCK | D13 | GPIO18 |
| MOSI | D11 | GPIO23 |
| MISO | D12 | GPIO19 |
| RST | D9 | GPIO22 |
| IRQ | не подключается | не подключается |

**RC522 питается только от 3.3 В.** Пять вольт на питании сжигают микросхему. Сигнальные ножки рассчитаны на 3.3 В: к 5 В Uno их часто подключают напрямую, и это работает, но надёжнее делитель напряжения или преобразователь уровней. Считыватель читает карты на 13.56 МГц (MIFARE, NTAG); метки 125 кГц (EM4100) он не читает.

### Чем отличается от других библиотек
Мы посмотрели MFRC522 (miguelbalboa), MFRC522_I2C и rdm6300:
- В MFRC522 нужно самому писать `PICC_IsNewCardPresent()` + `PICC_ReadCardSerial()`, собирать UID в строку циклом и сравнивать с массивами. У нас `available()`, `uidString()`, `isAllowed()`.
- Забытая у считывателя карта срабатывает каждый опрос; у нас пауза.
- Нет проверки, что RC522 подключён: библиотека молча не видит карт. У нас `begin()` возвращает false с подсказкой.
- Мы включили её проверенный драйвер (общественное достояние), поэтому ничего не потеряно в надёжности.

### Примеры
`Basic`, `AccessControl`, `LearnCards`, `Diagnose`

### Тесты
Логика библиотеки (проверка связи по версии микросхемы, срабатывание один раз за поднесение, пауза для той же карты, разные карты, UID на 4 и 7 байт, типы по SAK, список разрешённых карт с разбором разных написаний, дубли, переполнение, удаление, печать, диагностика) проверена без железа на имитации читателя: `extras/test`. Сам драйвер RC522 это неизменённый код MFRC522 1.4.12 и без железа не тестируется.

Подробное руководство: [docs/WIKI.md](docs/WIKI.md).

---

## Қазақша

**RC522 карта оқу құрылғысына** (MIFARE, брелоктар) арналған қарапайым кітапхана: `rfid.available()` картаны жақындатқанда бір рет UID береді, `rfid.isAllowed()` картаның **рұқсат етілген тізімде** екенін тексереді (мысалы, құлып үшін). RC522 жауап беретінін тексеріп, нені түзету керегін айтады. RC522 драйвері ішінде: бөлек MFRC522 кітапханасы **қажет емес**.

```cpp
#include <AlashRFID.h>
AlashRFID rfid(10, 9);                    // SS, RST

void setup() {
  Serial.begin(115200);
  if (!rfid.begin()) { Serial.println(rfid.errorText()); while (true); }
  rfid.addCard("04:A1:B2:C3");           // разрешённая карта / allowed card
}
void loop() {
  if (rfid.available()) {
    Serial.println(rfid.uidString());     // 04:A1:B2:C3
    Serial.println(rfid.isAllowed() ? "Открыто" : "Чужая");
  }
}
```

### Мүмкіндіктері
- **Карта бір жолмен:** `available()` жақындатқанда бір рет; `uidString()` `04:A1:B2:C3` береді.
- **Қайталанбайды:** оқу құрылғысында ұмытылған карта әр сұрауда емес, `setCooldownMs()` сайын.
- **Рұқсат етілген карталар тізімі:** `addCard("04:A1:B2:C3")`, `isAllowed()`, `addCurrent()`, 16 картаға дейін.
- **Байланысты тексеру:** `begin()` RC522 нұсқасын оқиды; байланыс жоқ болса `ERR_NO_READER`.
- **Кіріктірілген RC522 драйвері.**
- **Түсінікті кеңестер** үш тілде.
- **Arduino AVR, ESP32, ESP8266**.

### Қосу
| RC522 | Uno / Nano | ESP32 |
|---|---|---|
| 3.3V | 3.3 В (НЕ 5 В) | 3.3 В |
| GND | GND | GND |
| SDA (SS) | D10 | GPIO5 |
| SCK | D13 | GPIO18 |
| MOSI | D11 | GPIO23 |
| MISO | D12 | GPIO19 |
| RST | D9 | GPIO22 |
| IRQ | не подключается | не подключается |

**RC522 тек 3.3 В-пен қоректенеді.** Бес вольт микросхеманы өртейді. 13.56 МГц карталарын (MIFARE, NTAG) оқиды; 125 кГц белгілерін оқымайды.

### Басқа кітапханалардан айырмашылығы
MFRC522 (miguelbalboa), MFRC522_I2C және rdm6300 қарадық:
- MFRC522-де UID-ті жолға өзің жинайсың; бізде `available()`, `uidString()`, `isAllowed()`.
- Ұмытылған карта әр сұрауда істейді; бізде үзіліс.
- RC522 қосылғанын тексеру жоқ; бізде `begin()`.
- Біз оның сенімді драйверін қостық.

### Мысалдар
`Basic`, `AccessControl`, `LearnCards`, `Diagnose`

### Тесттер
Кітапхана логикасы жалған оқу құрылғысында тексерілген: `extras/test`. RC522 драйвері өзгертілмеген MFRC522 1.4.12 коды.

Толық нұсқаулық: [docs/WIKI.md](docs/WIKI.md).

---

## English

A simple library for the **RC522 card reader** (MIFARE cards and key fobs from kits): `rfid.available()` gives the card number (UID) once per tap, and `rfid.isAllowed()` checks whether the card is in your **allowed list**, for example for a door lock. It checks that the RC522 answers at all and tells what to fix (3.3 V power, SPI wires). The RC522 driver is built in: a separate MFRC522 library is **not needed**. Made for classes and Alash Electronics kits.

```cpp
#include <AlashRFID.h>
AlashRFID rfid(10, 9);                    // SS, RST

void setup() {
  Serial.begin(115200);
  if (!rfid.begin()) { Serial.println(rfid.errorText()); while (true); }
  rfid.addCard("04:A1:B2:C3");           // разрешённая карта / allowed card
}
void loop() {
  if (rfid.available()) {
    Serial.println(rfid.uidString());     // 04:A1:B2:C3
    Serial.println(rfid.isAllowed() ? "Открыто" : "Чужая");
  }
}
```

### Features
- **A card in one line:** `available()` fires once per tap; `uidString()` gives `04:A1:B2:C3`, `uidNumber()` a number, `cardType()` the type.
- **No repeats:** a card forgotten on the reader does not fire on every poll, only once per `setCooldownMs()` (a second by default); a different card fires right away.
- **Allowed-card list:** `addCard("04:A1:B2:C3")` (with colons, spaces or none), `isAllowed()`, `addCurrent()`, `removeCard()`, `printCards()`; up to 16 cards; UIDs of 4, 7 and 10 bytes.
- **Link check:** `begin()` reads the RC522 version; no link (power, SPI wires) is `ERR_NO_READER` with a tip, a clone is `ERR_FAKE_CHIP` but usually works.
- **Built-in RC522 driver** (public domain): no separate MFRC522 library to install and no clash with it.
- **Clear tips** in three languages: `errorText()`, `diagnose(Serial)` checks the chip and waits for a card.
- Works on **Arduino AVR, ESP32, ESP8266**; custom SPI pins on ESP32: `begin(sck, miso, mosi)`.

### Wiring
| RC522 | Uno / Nano | ESP32 |
|---|---|---|
| 3.3V | 3.3 В (НЕ 5 В) | 3.3 В |
| GND | GND | GND |
| SDA (SS) | D10 | GPIO5 |
| SCK | D13 | GPIO18 |
| MOSI | D11 | GPIO23 |
| MISO | D12 | GPIO19 |
| RST | D9 | GPIO22 |
| IRQ | не подключается | не подключается |

**The RC522 is powered from 3.3 V only.** Five volts will burn the chip. It reads 13.56 MHz cards (MIFARE, NTAG); it does not read 125 kHz tags (EM4100).

### How it differs from other libraries
We looked at MFRC522 (miguelbalboa), MFRC522_I2C and rdm6300:
- In MFRC522 you write `PICC_IsNewCardPresent()` + `PICC_ReadCardSerial()` yourself, build the UID string in a loop and compare arrays. We have `available()`, `uidString()`, `isAllowed()`.
- A card forgotten on the reader fires on every poll; we have a pause.
- No check that the RC522 is connected: the library silently sees no cards. Our `begin()` returns false with a tip.
- We built in its proven public-domain driver, so nothing is lost in reliability.

### Examples
`Basic`, `AccessControl`, `LearnCards`, `Diagnose`

### Tests
The library logic (link check from the chip version, firing once per tap, the pause for the same card, different cards, 4 and 7 byte UIDs, card types from the SAK, an allowed-card list with several spellings, duplicates, overflow, removal, printing, diagnostics) is checked without hardware on a reader simulator: `extras/test`. The RC522 driver itself is unchanged MFRC522 1.4.12 code and is not tested without hardware.

Full guide: [docs/WIKI.md](docs/WIKI.md).

---

MIT License, Alash Electronics.
