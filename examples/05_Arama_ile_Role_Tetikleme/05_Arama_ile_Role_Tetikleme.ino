/*
 * ArduGSM — 05 Arama ile Röle Tetikleme (ör. kapı / bariyer açma)
 *
 * Yetkili bir numara kartı aradığında arama ücretsiz olarak reddedilir ve
 * Röle 1 kısa bir süre çeker (darbe). Kapı otomatiği, bariyer veya garaj
 * kapısı gibi "butona basma" girişleri için uygundur.
 * Yetkisiz numaralardan gelen aramalar da reddedilir ama röle çekmez.
 *
 * Ayarlar: YETKILI_NUMARALAR (+90... biçiminde) ve ROLE_DARBE_MS.
 * Gereken: 12 V / 1 A adaptör, SMA anten, sesli arama alabilen Micro SIM.
 *
 * Bağlantılar (kart üzerinde sabit):
 *   D7 Buzzer, D8 Röle 1, D10 ← SIM800C TX, D11 → SIM800C RX
 *
 * EN: An authorised caller is rejected (no call charge) and relay 1 is pulsed,
 *     e.g. to open a gate. Calls from other numbers are rejected without action.
 */

#include <SoftwareSerial.h>

// ─────────── KULLANICI AYARLARI ───────────
const char* const YETKILI_NUMARALAR[] = {
  "+905XXXXXXXXX",
  // "+905YYYYYYYYY",
};
const uint16_t ROLE_DARBE_MS = 1000;   // röle ne kadar süre çeksin
// ──────────────────────────────────────────

#define GSM_RX_PIN 10
#define GSM_TX_PIN 11
#define BUZZER_PIN 7
#define ROLE1_PIN  8

SoftwareSerial gsm(GSM_RX_PIN, GSM_TX_PIN);
String satir;

String at(const __FlashStringHelper* komut, uint32_t sure_ms = 2000) {
  while (gsm.available()) gsm.read();
  gsm.println(komut);
  String cevap;
  uint32_t t = millis();
  while (millis() - t < sure_ms) {
    while (gsm.available()) cevap += (char)gsm.read();
    if (cevap.indexOf(F("OK")) >= 0 || cevap.indexOf(F("ERROR")) >= 0) break;
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
  for (;;) {
    String r = at(F("AT+CREG?"));
    if (r.indexOf(F(",1")) >= 0 || r.indexOf(F(",5")) >= 0) break;
    if (millis() - t > 60000) return false;
    delay(2000);
  }
  at(F("AT+CLIP=1"));   // gelen aramada arayan numarayı göster (+CLIP)
  return true;
}

bool yetkiliMi(const String& numara) {
  for (const char* yetkili : YETKILI_NUMARALAR) {
    if (numara == yetkili) return true;
  }
  return false;
}

// Gelen arama iki satır üretir:  RING  ve  +CLIP: "+905XXXXXXXXX",145,"",0,"",0
void satirIsle(const String& s) {
  if (!s.startsWith(F("+CLIP:"))) return;
  int bas = s.indexOf('"');
  int son = s.indexOf('"', bas + 1);
  String arayan = (bas >= 0 && son > bas) ? s.substring(bas + 1, son) : String();

  at(F("ATH"));         // aramayı reddet: arayan ücret ödemez
  Serial.print(F("Arayan: ")); Serial.println(arayan);

  if (yetkiliMi(arayan)) {
    Serial.println(F("Yetkili, Role 1 tetikleniyor."));
    digitalWrite(ROLE1_PIN, HIGH);
    digitalWrite(BUZZER_PIN, HIGH);
    delay(ROLE_DARBE_MS);
    digitalWrite(ROLE1_PIN, LOW);
    digitalWrite(BUZZER_PIN, LOW);
  } else {
    Serial.println(F("Yetkisiz numara."));
  }
}

void setup() {
  Serial.begin(115200);
  gsm.begin(9600);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(ROLE1_PIN, OUTPUT);
  digitalWrite(ROLE1_PIN, LOW);
  delay(3000);

  Serial.println(F("=== ArduGSM arama ile role tetikleme ==="));
  if (!modemHazirla()) {
    Serial.println(F("[HATA] Modem hazir degil: 12 V besleme, anten, SIM ve 2G kapsamasini kontrol edin."));
    return;
  }
  Serial.println(F("Hazir, arama bekleniyor."));
}

void loop() {
  while (gsm.available()) {
    char c = gsm.read();
    if (c == '\n') {
      satir.trim();
      satirIsle(satir);
      satir = "";
    } else if (c != '\r' && satir.length() < 120) {
      satir += c;
    }
  }
}
