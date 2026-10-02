/*
 * ArduGSM — 06 GPRS ile HTTP GET
 *
 * SIM800C'nin dahili HTTP istemcisiyle bir adrese GET isteği atar; durum kodunu
 * ve cevabın ilk kısmını Seri Monitöre basar. Sensör verisini bir sunucuya
 * göndermek için URL'ye sorgu parametresi ekleyebilirsiniz:
 *   http://ornek.com/kayit?sicaklik=23.5
 *
 * Ayarlar: APN (operatörünüze göre) ve URL.
 *   Türkiye'de Turkcell, Vodafone ve Türk Telekom için APN genellikle "internet"tir.
 * Not: SIM800C 2G GPRS kullanır; TLS desteği sınırlı olduğundan http:// adres kullanın.
 * Gereken: 12 V / 1 A adaptör, SMA anten, veri paketi tanımlı Micro SIM.
 *
 * Bağlantılar (kart üzerinde sabit): D10 ← SIM800C TX, D11 → SIM800C RX
 *
 * EN: Opens a GPRS bearer and performs an HTTP GET with the SIM800C's built-in
 *     HTTP client. Set APN for your operator; use plain http:// URLs.
 */

#include <SoftwareSerial.h>

// ─────────── KULLANICI AYARLARI ───────────
#define APN "internet"
#define URL "http://example.com/"
// ──────────────────────────────────────────

#define GSM_RX_PIN 10
#define GSM_TX_PIN 11

SoftwareSerial gsm(GSM_RX_PIN, GSM_TX_PIN);

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

bool modemHazirla() {
  bool cevapVar = false;
  for (uint8_t i = 0; i < 10 && !cevapVar; i++) cevapVar = at(F("AT"), 1000).indexOf(F("OK")) >= 0;
  if (!cevapVar) return false;
  at(F("ATE0"));
  at(F("AT+CMEE=2"));
  uint32_t t = millis();
  while (at(F("AT+CPIN?")).indexOf(F("READY")) < 0) {
    if (millis() - t > 20000) return false;
    delay(1000);
  }
  t = millis();
  for (;;) {                                  // GPRS kaydı: +CGREG: 0,1 veya 0,5
    String r = at(F("AT+CGREG?"));
    if (r.indexOf(F(",1")) >= 0 || r.indexOf(F(",5")) >= 0) break;
    if (millis() - t > 60000) return false;
    delay(2000);
  }
  return true;
}

bool gprsAc() {
  at(F("AT+SAPBR=3,1,\"Contype\",\"GPRS\""));
  at(F("AT+SAPBR=3,1,\"APN\",\"" APN "\""));
  at(F("AT+SAPBR=1,1"), 30000);               // taşıyıcıyı aç (zaten açıksa ERROR döner, sorun değil)
  return at(F("AT+SAPBR=2,1")).indexOf(F("+SAPBR: 1,1")) >= 0;   // 1,1 = bağlı; ardından IP
}

void httpGet() {
  at(F("AT+HTTPTERM"));                       // önceki oturum kaldıysa kapat
  at(F("AT+HTTPINIT"));
  at(F("AT+HTTPPARA=\"CID\",1"));
  at(F("AT+HTTPPARA=\"URL\",\"" URL "\""));
  at(F("AT+HTTPPARA=\"REDIR\",1"));           // yönlendirmeleri izle

  // +HTTPACTION: 0,<durum>,<uzunluk>   (200 = başarılı)
  String r = at(F("AT+HTTPACTION=0"), 60000, "+HTTPACTION:");
  if (r.indexOf(F("+HTTPACTION:")) < 0) {
    gsm.setTimeout(60000);
    String urc = gsm.readStringUntil('\n');   // URC, OK'dan sonra ayrı satırda gelir
    Serial.print(F("HTTP sonucu: ")); Serial.println(urc);
  }

  // Cevabın ilk 200 baytı (Uno'nun 2 KB RAM'i için sınırlı)
  at(F("AT+HTTPREAD=0,200"), 10000);
  at(F("AT+HTTPTERM"));
}

void setup() {
  Serial.begin(115200);
  gsm.begin(9600);
  delay(3000);

  Serial.println(F("=== ArduGSM GPRS HTTP GET ==="));
  if (!modemHazirla()) {
    Serial.println(F("[HATA] Modem hazir degil: 12 V besleme, anten, SIM ve 2G kapsamasini kontrol edin."));
    return;
  }
  if (!gprsAc()) {
    Serial.println(F("[HATA] GPRS acilamadi: APN ve SIM'in veri paketini kontrol edin."));
    return;
  }
  httpGet();
  at(F("AT+SAPBR=0,1"), 10000);               // taşıyıcıyı kapat
  Serial.println(F("=== Bitti ==="));
}

void loop() {}
