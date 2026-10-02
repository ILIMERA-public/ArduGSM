/*
 * ArduGSM — 03 SMS Gönder
 *
 * Kart açıldığında modemi hazırlar (SIM, şebeke) ve TELEFON_NO'ya bir SMS gönderir.
 * Her adımın sonucu Seri Monitörde (115200 baud) görünür.
 *
 * Ayarlar: TELEFON_NO'yu uluslararası biçimde yazın (+90...).
 * Gereken: 12 V / 1 A adaptör, SMA anten, SMS gönderebilen Micro SIM.
 *
 * Bağlantılar (kart üzerinde sabit): D10 ← SIM800C TX, D11 → SIM800C RX
 *
 * EN: Prepares the modem and sends one SMS to TELEFON_NO (international format).
 */

#include <SoftwareSerial.h>

// ─────────── KULLANICI AYARLARI ───────────
const char TELEFON_NO[] = "+905XXXXXXXXX";        // SMS gidecek numara
const char MESAJ[]      = "ArduGSM'den merhaba!";  // Türkçe karakter kullanmayın (GSM 7-bit)
// ──────────────────────────────────────────

#define GSM_RX_PIN 10
#define GSM_TX_PIN 11
#define BUZZER_PIN 7

SoftwareSerial gsm(GSM_RX_PIN, GSM_TX_PIN);

// Komutu gönderir, OK / ERROR (ya da "bitis") gelene veya süre dolana kadar cevabı toplar.
String at(const __FlashStringHelper* komut, uint32_t sure_ms = 2000, const char* bitis = "OK") {
  while (gsm.available()) gsm.read();
  gsm.println(komut);
  String cevap;
  uint32_t t = millis();
  while (millis() - t < sure_ms) {
    while (gsm.available()) cevap += (char)gsm.read();
    if (cevap.indexOf(bitis) >= 0 || cevap.indexOf(F("ERROR")) >= 0) break;
  }
  cevap.trim();
  Serial.print(F(">> ")); Serial.print(komut); Serial.print(F("  |  ")); Serial.println(cevap);
  return cevap;
}

// Modemi kullanıma hazırlar: AT → SIM hazır → şebeke kaydı.
bool modemHazirla() {
  bool cevapVar = false;
  for (uint8_t i = 0; i < 10 && !cevapVar; i++) {   // otomatik baud eşleme + açılış beklemesi
    cevapVar = at(F("AT"), 1000).indexOf(F("OK")) >= 0;
  }
  if (!cevapVar) return false;
  at(F("ATE0"));                                     // komut yankısını kapat
  at(F("AT+CMEE=2"));                                // hataları metin olarak ver

  uint32_t t = millis();
  while (at(F("AT+CPIN?")).indexOf(F("READY")) < 0) { // SIM hazır mı?
    if (millis() - t > 20000) return false;
    delay(1000);
  }
  t = millis();
  for (;;) {                                         // şebeke kaydı: 0,1 = ev, 0,5 = dolaşım
    String r = at(F("AT+CREG?"));
    if (r.indexOf(F(",1")) >= 0 || r.indexOf(F(",5")) >= 0) break;
    if (millis() - t > 60000) return false;
    delay(2000);
  }
  at(F("AT+CSQ"));                                   // sinyal gücü: 10 ve üzeri yeterli
  return true;
}

bool smsGonder(const char* numara, const char* metin) {
  at(F("AT+CMGF=1"));                                // metin modu
  at(F("AT+CSCS=\"GSM\""));

  while (gsm.available()) gsm.read();
  gsm.print(F("AT+CMGS=\""));
  gsm.print(numara);
  gsm.println(F("\""));
  gsm.setTimeout(5000);
  if (!gsm.find((char*)">")) {                       // modem metin için ">" istemini verir
    Serial.println(F("[HATA] Modem '>' istemini vermedi."));
    return false;
  }
  gsm.print(metin);
  gsm.write(26);                                     // Ctrl+Z: mesajı gönder

  String cevap;
  uint32_t t = millis();
  while (millis() - t < 60000) {                     // gönderim 60 sn'ye kadar sürebilir
    while (gsm.available()) cevap += (char)gsm.read();
    if (cevap.indexOf(F("+CMGS:")) >= 0 || cevap.indexOf(F("ERROR")) >= 0) break;
  }
  Serial.print(F("SMS cevabi: ")); Serial.println(cevap);
  return cevap.indexOf(F("+CMGS:")) >= 0;
}

void setup() {
  Serial.begin(115200);
  gsm.begin(9600);
  pinMode(BUZZER_PIN, OUTPUT);
  delay(3000);                                       // SIM800C açılışı

  Serial.println(F("=== ArduGSM SMS ornegi ==="));
  if (!modemHazirla()) {
    Serial.println(F("[HATA] Modem hazir degil: 12 V besleme, anten, SIM ve 2G kapsamasini kontrol edin."));
    return;
  }
  if (smsGonder(TELEFON_NO, MESAJ)) {
    Serial.println(F("SMS gonderildi."));
    digitalWrite(BUZZER_PIN, HIGH); delay(150); digitalWrite(BUZZER_PIN, LOW);
  } else {
    Serial.println(F("[HATA] SMS gonderilemedi."));
  }
}

void loop() {}
