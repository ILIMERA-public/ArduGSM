/*
 * ArduGSM — 02 Röle ve Buzzer Testi
 *
 * Kart üstündeki iki röleyi ve buzzer'ı sırayla çalıştırır. GSM gerekmez;
 * kartın ilk kurulumunda donanımı kontrol etmek için kullanın.
 *
 * Not: Röleler yalnız 12 V / 1 A adaptör takılıyken çeker. Yalnız USB ile
 * çalışırken röle LED'leri yanmaz, bu bir arıza değildir.
 *
 * Bağlantılar (kart üzerinde sabit):
 *   D7 → Buzzer   (HIGH = ses)
 *   D8 → Röle 1   (HIGH = röle çeker)
 *   D9 → Röle 2   (HIGH = röle çeker)
 *
 * EN: Cycles relay 1, relay 2 and the buzzer. Relays need the 12 V supply.
 */

#define BUZZER_PIN 7
#define ROLE1_PIN  8
#define ROLE2_PIN  9

void bip(uint16_t sure_ms) {
  digitalWrite(BUZZER_PIN, HIGH);
  delay(sure_ms);
  digitalWrite(BUZZER_PIN, LOW);
}

void setup() {
  Serial.begin(115200);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(ROLE1_PIN, OUTPUT);
  pinMode(ROLE2_PIN, OUTPUT);
  digitalWrite(ROLE1_PIN, LOW);
  digitalWrite(ROLE2_PIN, LOW);
  digitalWrite(BUZZER_PIN, LOW);
  Serial.println(F("ArduGSM role ve buzzer testi"));
}

void loop() {
  Serial.println(F("Role 1 ON"));
  digitalWrite(ROLE1_PIN, HIGH);
  bip(100);
  delay(1500);
  digitalWrite(ROLE1_PIN, LOW);

  Serial.println(F("Role 2 ON"));
  digitalWrite(ROLE2_PIN, HIGH);
  bip(100);
  delay(1500);
  digitalWrite(ROLE2_PIN, LOW);

  Serial.println(F("Buzzer"));
  bip(400);
  delay(2000);
}
