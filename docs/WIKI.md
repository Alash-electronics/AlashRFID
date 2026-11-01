# AlashRFID: руководство / нұсқаулық / guide

[Русский](#русский) | [Қазақша](#қазақша) | [English](#english)

---

## Русский

Простая библиотека для **считывателя карт RC522** (MIFARE, брелоки, карты из набора): `rfid.available()` даёт номер карты (UID) один раз за поднесение, а `rfid.isAllowed()` проверяет, есть ли карта в вашем **списке разрешённых**, например для замка. Проверяет, что RC522 вообще отвечает, и подсказывает, что исправить (питание 3.3 В, провода SPI). Драйвер RC522 внутри: отдельная библиотека MFRC522 **не нужна**. Сделана для уроков и наборов Alash Electronics.

### Установка
Arduino IDE: Скетч, Подключить библиотеку, Управлять библиотеками, найти **AlashRFID** и нажать Установить. Или скачайте ZIP с GitHub: Скетч, Подключить библиотеку, Добавить .ZIP библиотеку.

### Платы
Arduino Uno / Nano / Mega (AVR), ESP32, ESP8266 и другие, где работает Arduino-ядро.

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

### Быстрый старт
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

### Справочник по методам
| Метод | Что делает |
|---|---|
| `AlashRFID(ssPin, rstPin)` | Считыватель RC522 на SPI |
| `begin()` | Запуск и проверка связи |
| `available()` | Карта поднесена (один раз) |
| `uidString() / uid() / uidSize() / uidNumber()` | UID строкой, байтами, числом |
| `cardType()` | Тип карты |
| `addCard(s) / removeCard(s) / addCurrent() / clearCards()` | Список разрешённых карт |
| `isAllowed() / cardCount() / printCards(Serial)` | Проверка и печать списка |
| `setCooldownMs(ms)` | Пауза для той же карты |
| `diagnose(Serial)` | Проверка RC522 и ожидание карты |

### Если что-то не работает
**ERR_NO_READER: считыватель не отвечает**

Питание только 3.3 В. Проверьте пины SPI: SCK, MOSI, MISO и SDA(SS) должны идти на нужные пины платы (таблица подключения выше). Пины SS и RST в `AlashRFID rfid(SS, RST)` должны совпадать с проводами. Запустите пример Diagnose.

**Карта не читается**

Поднесите вплотную к антенне (до 3 см), уберите металл рядом. Убедитесь, что метка на 13.56 МГц (MIFARE): метки 125 кГц (EM4100) этим считывателем не читаются.

**Конфликт при установке вместе с MFRC522**

Конфликта нет: драйвер внутри AlashRFID переименован (класс AlashRC522Core). Но не подключайте обе библиотеки к одним пинам одновременно.

### Самопроверка
В библиотеке есть `diagnose(Serial)`: она печатает, что видит плата, и подсказывает, что проверить. Язык подсказок: `setLanguage(LANG_RU / LANG_KK / LANG_EN)`.

### Примеры
`Basic`, `AccessControl`, `LearnCards`, `Diagnose`

---

## Қазақша

**RC522 карта оқу құрылғысына** (MIFARE, брелоктар) арналған қарапайым кітапхана: `rfid.available()` картаны жақындатқанда бір рет UID береді, `rfid.isAllowed()` картаның **рұқсат етілген тізімде** екенін тексереді (мысалы, құлып үшін). RC522 жауап беретінін тексеріп, нені түзету керегін айтады. RC522 драйвері ішінде: бөлек MFRC522 кітапханасы **қажет емес**.

### Орнату
Arduino IDE: Sketch, Include Library, Manage Libraries, **AlashRFID** табып, Install басыңыз. Немесе GitHub-тан ZIP жүктеп, Sketch, Include Library, Add .ZIP Library.

### Тақталар
Arduino Uno / Nano / Mega (AVR), ESP32, ESP8266 және Arduino ядросы бар басқалар.

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

### Жылдам бастау
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

### Әдістер анықтамасы
| Әдіс | Не істейді |
|---|---|
| `AlashRFID(ssPin, rstPin)` | SPI-дағы RC522 |
| `begin()` | Іске қосу және байланысты тексеру |
| `available()` | Карта жақындатылды (бір рет) |
| `uidString() / uid() / uidSize() / uidNumber()` | UID жол, байт, сан |
| `cardType()` | Карта түрі |
| `addCard(s) / removeCard(s) / addCurrent() / clearCards()` | Рұқсат етілген карталар |
| `isAllowed() / cardCount() / printCards(Serial)` | Тексеру және тізім шығару |
| `setCooldownMs(ms)` | Бір карта үшін үзіліс |
| `diagnose(Serial)` | RC522 тексеру |

### Бірдеңе жұмыс істемесе
**ERR_NO_READER**

Қуат тек 3.3 В. SPI пиндерін тексеріңіз.

**Карта оқылмайды**

Антеннаға тақап жақындатыңыз. Белгі 13.56 МГц болуы керек.

**MFRC522-мен бірге орнатқанда**

Қайшылық жоқ: драйвер атауы өзгертілген.

### Өзін-өзі тексеру
Кітапханада `diagnose(Serial)` бар: тақта не көргенін басып шығарады және нені тексеру керегін айтады. Кеңес тілі: `setLanguage(LANG_RU / LANG_KK / LANG_EN)`.

### Мысалдар
`Basic`, `AccessControl`, `LearnCards`, `Diagnose`

---

## English

A simple library for the **RC522 card reader** (MIFARE cards and key fobs from kits): `rfid.available()` gives the card number (UID) once per tap, and `rfid.isAllowed()` checks whether the card is in your **allowed list**, for example for a door lock. It checks that the RC522 answers at all and tells what to fix (3.3 V power, SPI wires). The RC522 driver is built in: a separate MFRC522 library is **not needed**. Made for classes and Alash Electronics kits.

### Installation
Arduino IDE: Sketch, Include Library, Manage Libraries, search **AlashRFID** and click Install. Or download the ZIP from GitHub: Sketch, Include Library, Add .ZIP Library.

### Boards
Arduino Uno / Nano / Mega (AVR), ESP32, ESP8266 and others with an Arduino core.

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

### Quick start
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

### Method reference
| Method | What it does |
|---|---|
| `AlashRFID(ssPin, rstPin)` | An RC522 reader on SPI |
| `begin()` | Start and link check |
| `available()` | A card was tapped (once) |
| `uidString() / uid() / uidSize() / uidNumber()` | UID as text, bytes, number |
| `cardType()` | Card type |
| `addCard(s) / removeCard(s) / addCurrent() / clearCards()` | The allowed-card list |
| `isAllowed() / cardCount() / printCards(Serial)` | Check and print the list |
| `setCooldownMs(ms)` | Pause for the same card |
| `diagnose(Serial)` | Check the RC522 and wait for a card |

### If something does not work
**ERR_NO_READER: the reader does not answer**

Power only 3.3 V. Check the SPI pins: SCK, MOSI, MISO and SDA(SS) must go to the right board pins (see the wiring table). The SS and RST pins in `AlashRFID rfid(SS, RST)` must match the wires. Run the Diagnose example.

**The card is not read**

Hold it right against the antenna (within 3 cm), remove metal nearby. Make sure the tag is 13.56 MHz (MIFARE): 125 kHz (EM4100) tags are not read by this reader.

**A clash when installed together with MFRC522**

There is no clash: the driver inside AlashRFID is renamed (class AlashRC522Core). Just do not wire both libraries to the same pins at once.

### Self-check
The library has `diagnose(Serial)`: it prints what the board sees and tells what to check. Tips language: `setLanguage(LANG_RU / LANG_KK / LANG_EN)`.

### Examples
`Basic`, `AccessControl`, `LearnCards`, `Diagnose`

---

MIT License, Alash Electronics. https://github.com/Alash-electronics/AlashRFID
