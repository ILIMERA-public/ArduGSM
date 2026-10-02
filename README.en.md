# ArduGSM — Arduino and SIM800C GSM Control Board

[Türkçe](README.md) · [Product page](https://ilimera.com/en/urunler/gelistirme-kartlari/ardugsm) · [Technical document (PDF, Turkish)](docs/ArduGSM_teknik_dokuman_v1.pdf)

![ArduGSM](docs/images/ardugsm-main.webp)

ArduGSM combines an **Arduino UNO (ATmega328P)**, a **SIM800C GSM/GPRS** module, **2 relays**, a **buzzer**,
microphone/speaker connections and a **3 A power stage** on one board. No external Arduino, shield or extra
wiring is needed; it is programmed from the Arduino IDE over USB Type-C. It is built for remote control and
monitoring projects over SMS, voice calls and GPRS.

This repository contains example sketches and an AT command reference so you can start right away.

## Specifications

| Feature | Value |
| --- | --- |
| Microcontroller | ATmega328P (Arduino UNO architecture) |
| Cellular module | SIM800C, quad-band **2G GSM/GPRS** |
| SIM | Micro SIM, TVS diode ESD protection |
| Antenna | SMA connector (external antenna) |
| Relay outputs | 2 independent relays (ON / COM / NC terminals) |
| Audible alert | On-board buzzer |
| Audio | On-board microphone, speaker (HP) connection |
| Programming | USB Type-C, Arduino IDE / PlatformIO |
| Power | 12 V / 1 A DC adapter, 3 A buck regulator, reverse-polarity protection |
| Size | 97 × 85 mm |

## Fixed pin assignments

| Arduino pin | Connected to | Usage |
| --- | --- | --- |
| D7 | Buzzer | `HIGH` → sounds |
| D8 | Relay 1 | `HIGH` → relay on |
| D9 | Relay 2 | `HIGH` → relay on |
| D10 | SIM800C TX | `SoftwareSerial` **RX** |
| D11 | SIM800C RX | `SoftwareSerial` **TX** |

```cpp
#include <SoftwareSerial.h>
SoftwareSerial gsm(10, 11);   // RX = D10 (SIM800C TX), TX = D11 (SIM800C RX)
```

D0–D6, D12, D13, A0–A5, 3.3 V, 5 V and GND are available on the **Arduino Pins** header. D0/D1 are shared with
the USB serial line. Relay terminals: **COM** common, **ON** normally open, **NC** normally closed.

## Power

| Connection | What works |
| --- | --- |
| USB Type-C only | Arduino programming and peripherals. **SIM800C and relays do not run.** |
| 12 V adapter | SIM800C and all peripherals |
| USB + 12 V | The board uses the 12 V input; GSM works while programming |

The SIM800C draws current bursts of up to **2 A** during network registration and data transfer. Keep a
**12 V / 1 A** adapter connected for SMS, call, GPRS and relay tests.

## Quick start

1. Attach the SMA antenna and the Micro SIM, power the board with the 12 V adapter.
2. Connect it to your computer with a USB Type-C cable.
3. In the Arduino IDE select **Tools → Board: Arduino Uno** and the board's port.
4. Upload [`examples/01_AT_Komut_Terminali`](examples/01_AT_Komut_Terminali).
5. Open the Serial Monitor at **115200 baud, Both NL & CR**, type `AT` → `OK`.
6. `AT+CPIN?` → `READY`, `AT+CSQ` → 10 or more, `AT+CREG?` → `0,1` means the board is on the network.

PlatformIO: `board = uno`, `framework = arduino`.

## Examples

| Example | What it does |
| --- | --- |
| [01_AT_Komut_Terminali](examples/01_AT_Komut_Terminali) | AT command terminal between the Serial Monitor and the SIM800C |
| [02_Role_ve_Buzzer_Testi](examples/02_Role_ve_Buzzer_Testi) | Cycles both relays and the buzzer (no GSM needed) |
| [03_SMS_Gonder](examples/03_SMS_Gonder) | Prepares the modem and sends an SMS |
| [04_SMS_ile_Role_Kontrolu](examples/04_SMS_ile_Role_Kontrolu) | Controls the relays by SMS from authorised numbers and replies with the state |
| [05_Arama_ile_Role_Tetikleme](examples/05_Arama_ile_Role_Tetikleme) | Rejects a call from an authorised number (free) and pulses relay 1, e.g. to open a gate |
| [06_GPRS_HTTP_GET](examples/06_GPRS_HTTP_GET) | Connects over GPRS and performs an HTTP GET |

Change phone numbers, APN and other values in the **KULLANICI AYARLARI** (user settings) block at the top of
each sketch. Code comments are in Turkish with an English summary in each header. All sketches compile for the
Arduino UNO with no extra libraries.

AT command reference (Turkish, commands are universal): [docs/AT_KOMUTLARI.md](docs/AT_KOMUTLARI.md).

## Troubleshooting

| Symptom | Fix |
| --- | --- |
| No reply to `AT` | Connect the 12 V adapter; the SIM800C does not run on USB alone |
| Relays do not switch | Same reason: the relays are powered from 12 V |
| `+CREG: 0,2` (searching) for a long time | Check the antenna; make sure 2G is available in your area and on your operator |
| `+CPIN: SIM PIN` | Disable the SIM PIN on a phone or send `AT+CPIN="1234"` |
| The modem keeps restarting | Insufficient supply: use at least 12 V / 1 A |
| GPRS does not open | Set your operator's APN and check the SIM has a data plan |

## Support

Documentation, updates and support: [ilimera.com](https://ilimera.com/en/urunler/gelistirme-kartlari/ardugsm).
Found a bug or have a suggestion? Open an **Issue** in this repository.

ArduGSM is developed by İLİMERA Technology.

## License

The example code is provided under the [MIT License](LICENSE); you are free to use it in your own products.
Technical documents and images are the property of İLİMERA Technology.
