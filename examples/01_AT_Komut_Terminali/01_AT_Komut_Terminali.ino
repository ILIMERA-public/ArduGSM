/*
 * ArduGSM — 01 AT Komut Terminali
 *
 * Bilgisayardaki Seri Monitörden yazdığınız AT komutlarını SIM800C'ye iletir,
 * modemin cevaplarını geri gösterir. Yeni bir SIM kartı, anteni veya şebeke
 * kapsamasını denemenin en hızlı yolu budur.
 *
 * Kullanım:
 *   1. Kartı 12 V / 1 A adaptörle besleyin (yalnız USB ile SIM800C çalışmaz),
 *      SMA anteni ve Micro SIM'i takın.
 *   2. Arduino IDE → Araçlar → Kart: "Arduino Uno", Port: kartın COM portu.
 *   3. Yükleyin, Seri Monitörü 115200 baud ve "Both NL & CR" ile açın.
 *   4. AT yazın → OK görmelisiniz. Diğer komutlar için docs/AT_KOMUTLARI.md.
 *
 * Bağlantılar (kart üzerinde sabit):
 *   D10 ← SIM800C TX   (SoftwareSerial RX)
 *   D11 → SIM800C RX   (SoftwareSerial TX)
 *
 * EN: Bridges the USB Serial Monitor (115200 baud) to the SIM800C (9600 baud)
 *     so you can type AT commands directly. Power the board with 12 V.
 */

#include <SoftwareSerial.h>

#define GSM_RX_PIN 10   // SIM800C TX hattı
#define GSM_TX_PIN 11   // SIM800C RX hattı
#define PC_BAUD    115200
#define GSM_BAUD   9600 // SoftwareSerial için güvenilir hız; SIM800C otomatik baud algılar

SoftwareSerial gsm(GSM_RX_PIN, GSM_TX_PIN);

void setup() {
  Serial.begin(PC_BAUD);
  gsm.begin(GSM_BAUD);
  delay(1000);

  Serial.println(F("ArduGSM AT terminali hazir. AT yazip Enter'a basin."));

  // SIM800C otomatik baud algılama yapar: ilk birkaç "AT" hızı eşler.
  for (uint8_t i = 0; i < 5; i++) {
    gsm.println(F("AT"));
    delay(300);
  }
}

void loop() {
  // Modem → bilgisayar
  while (gsm.available()) {
    Serial.write(gsm.read());
  }
  // Bilgisayar → modem
  while (Serial.available()) {
    gsm.write(Serial.read());
  }
}
