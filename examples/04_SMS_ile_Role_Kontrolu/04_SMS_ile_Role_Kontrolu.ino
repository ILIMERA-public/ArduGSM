/*
 * ArduGSM — 04 SMS ile Röle Kontrolü
 *
 * Yetkili numaralardan gelen SMS komutlarıyla iki röleyi açar/kapatır ve
 * durumu SMS ile geri bildirir. Her komutta buzzer kısa bir bip verir.
 *
 * Komutlar (büyük/küçük harf fark etmez):
 *   ROLE1 AC   ROLE1 KAPAT   ROLE2 AC   ROLE2 KAPAT   DURUM
 *
 * Ayarlar: YETKILI_NUMARALAR listesine numaraları uluslararası biçimde (+90...) yazın.
 * Listede olmayan numaralardan gelen SMS'ler yok sayılır.
 * Gereken: 12 V / 1 A adaptör, SMA anten, Micro SIM.
 *
 * Bağlantılar (kart üzerinde sabit):
 *   D7 Buzzer, D8 Röle 1, D9 Röle 2, D10 ← SIM800C TX, D11 → SIM800C RX
 *
 * EN: Controls both relays by SMS from authorised numbers and replies with the state.
 *     Commands: ROLE1 AC / ROLE1 KAPAT / ROLE2 AC / ROLE2 KAPAT / DURUM.
 */

#include <SoftwareSerial.h>

// ─────────── KULLANICI AYARLARI ───────────
const char* const YETKILI_NUMARALAR[] = {
  "+905XXXXXXXXX",
  // "+905YYYYYYYYY",
};
// ──────────────────────────────────────────

#define GSM_RX_PIN 10
#define GSM_TX_PIN 11
#define BUZZER_PIN 7
#define ROLE1_PIN  8
#define ROLE2_PIN  9

SoftwareSerial gsm(GSM_RX_PIN, GSM_TX_PIN);

String satir;          // modemden okunan satır
String gonderen;       // son +CMT satırındaki numara
bool mesajBekleniyor = false;

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
  at(F("AT+CMGF=1"));             // metin modu
  at(F("AT+CSCS=\"GSM\""));
  at(F("AT+CNMI=2,2,0,0,0"));     // gelen SMS'i saklamadan doğrudan seri hatta ver (+CMT)
  at(F("AT+CMGDA=\"DEL ALL\""), 10000);  // SIM'de birikmiş eski mesajları sil
  return true;
}

void bip(uint16_t sure_ms) {
  digitalWrite(BUZZER_PIN, HIGH);
  delay(sure_ms);
  digitalWrite(BUZZER_PIN, LOW);
}

bool yetkiliMi(const String& numara) {
  for (const char* yetkili : YETKILI_NUMARALAR) {
    if (numara == yetkili) return true;
  }
  return false;
}

void smsGonder(const String& numara, const String& metin) {
  gsm.print(F("AT+CMGS=\""));
  gsm.print(numara);
  gsm.println(F("\""));
  gsm.setTimeout(5000);
  if (!gsm.find((char*)">")) return;
  gsm.print(metin);
  gsm.write(26);                  // Ctrl+Z
  gsm.setTimeout(60000);
  gsm.find((char*)"+CMGS:");      // gönderim onayını bekle
  Serial.print(F("Cevap SMS gonderildi: ")); Serial.println(metin);
}

String durumMetni() {
  String s = F("ROLE1: ");
  s += digitalRead(ROLE1_PIN) ? F("ACIK") : F("KAPALI");
  s += F(", ROLE2: ");
  s += digitalRead(ROLE2_PIN) ? F("ACIK") : F("KAPALI");
  return s;
}

void komutIsle(String metin) {
  metin.trim();
  metin.toUpperCase();
  Serial.print(F("SMS [")); Serial.print(gonderen); Serial.print(F("]: ")); Serial.println(metin);

  if (!yetkiliMi(gonderen)) {
    Serial.println(F("Yetkisiz numara, yok sayildi."));
    return;
  }
  if      (metin == F("ROLE1 AC"))    digitalWrite(ROLE1_PIN, HIGH);
  else if (metin == F("ROLE1 KAPAT")) digitalWrite(ROLE1_PIN, LOW);
  else if (metin == F("ROLE2 AC"))    digitalWrite(ROLE2_PIN, HIGH);
  else if (metin == F("ROLE2 KAPAT")) digitalWrite(ROLE2_PIN, LOW);
  else if (metin != F("DURUM")) {
    smsGonder(gonderen, F("Bilinmeyen komut. ROLE1 AC / ROLE1 KAPAT / ROLE2 AC / ROLE2 KAPAT / DURUM"));
    return;
  }
  bip(80);
  smsGonder(gonderen, durumMetni());
}

// Modemden gelen her satırı işler. Yeni SMS iki satırdır:
//   +CMT: "+905XXXXXXXXX","","26/10/02,10:00:00+12"
//   ROLE1 AC
void satirIsle(const String& s) {
  if (s.startsWith(F("+CMT:"))) {
    int bas = s.indexOf('"');
    int son = s.indexOf('"', bas + 1);
    gonderen = (bas >= 0 && son > bas) ? s.substring(bas + 1, son) : String();
    mesajBekleniyor = true;
  } else if (mesajBekleniyor && s.length() > 0) {
    mesajBekleniyor = false;
    komutIsle(s);
  }
}

void setup() {
  Serial.begin(115200);
  gsm.begin(9600);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(ROLE1_PIN, OUTPUT);
  pinMode(ROLE2_PIN, OUTPUT);
  digitalWrite(ROLE1_PIN, LOW);
  digitalWrite(ROLE2_PIN, LOW);
  delay(3000);

  Serial.println(F("=== ArduGSM SMS ile role kontrolu ==="));
  if (!modemHazirla()) {
    Serial.println(F("[HATA] Modem hazir degil: 12 V besleme, anten, SIM ve 2G kapsamasini kontrol edin."));
    return;
  }
  bip(80); delay(120); bip(80);
  Serial.println(F("Hazir, SMS bekleniyor."));
}

void loop() {
  while (gsm.available()) {
    char c = gsm.read();
    if (c == '\n') {
      satir.trim();
      satirIsle(satir);
      satir = "";
    } else if (c != '\r' && satir.length() < 160) {
      satir += c;
    }
  }
}
