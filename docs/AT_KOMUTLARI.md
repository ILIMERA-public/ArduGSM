# SIM800C AT Komutları — Hızlı Başvuru

ArduGSM üzerindeki SIM800C modülünü denemek için en sık kullanılan komutlar.
Komutları [`01_AT_Komut_Terminali`](../examples/01_AT_Komut_Terminali) örneğiyle Seri Monitörden
(115200 baud, **Both NL & CR**) doğrudan yazabilirsiniz.

> Modemin çalışması için kart **12 V / 1 A adaptörle** beslenmeli; yalnız USB ile SIM800C enerjilenmez.
> SIM800C **yalnız 2G (GSM/GPRS)** şebekede çalışır.

## 1. Temel kontrol

| Komut | Ne yapar | Örnek cevap |
| --- | --- | --- |
| `AT` | Modem cevap veriyor mu | `OK` |
| `ATE0` / `ATE1` | Komut yankısını kapat / aç | `OK` |
| `AT+CMEE=2` | Hataları metin olarak göster | `OK` |
| `ATI` | Modül bilgisi | `SIM800 R14.18` |
| `AT+CGMR` | Yazılım sürümü | `Revision:1418B05SIM800C24` |
| `AT+CBC` | Besleme gerilimi (mV) | `+CBC: 0,0,4100` |
| `AT+IPR=9600` + `AT&W` | Seri hızı sabitle ve kaydet | `OK` |

## 2. SIM ve şebeke

| Komut | Ne yapar | Örnek cevap |
| --- | --- | --- |
| `AT+CPIN?` | SIM durumu | `+CPIN: READY` (PIN isteniyorsa `SIM PIN`) |
| `AT+CPIN="1234"` | SIM PIN'ini gir | `OK` |
| `AT+CCID` | SIM kart numarası (ICCID) | `8990...` |
| `AT+CSQ` | Sinyal gücü (0–31, 99 = bilinmiyor) | `+CSQ: 18,0` |
| `AT+CREG?` | GSM şebeke kaydı | `+CREG: 0,1` (1 = ev, 5 = dolaşım, 2 = aranıyor) |
| `AT+COPS?` | Bağlı operatör | `+COPS: 0,0,"TR TURKCELL"` |
| `AT+CGREG?` | GPRS kaydı | `+CGREG: 0,1` |

Sinyal: `+CSQ` değeri 10'un altındaysa anteni ve konumu kontrol edin.

## 3. SMS

| Komut | Ne yapar |
| --- | --- |
| `AT+CMGF=1` | Metin modu (önce bunu verin) |
| `AT+CSCS="GSM"` | Karakter kümesi |
| `AT+CMGS="+905XXXXXXXXX"` | SMS yaz: `>` istemi gelince metni yazın, **Ctrl+Z** (0x1A) ile gönderin → `+CMGS: 12` |
| `AT+CNMI=2,2,0,0,0` | Gelen SMS'i saklamadan seri hatta ver: `+CMT: "+905...","","26/10/02,10:00:00+12"` ve alt satırda metin |
| `AT+CMGL="ALL"` | SIM'deki tüm mesajları listele |
| `AT+CMGR=1` | 1 numaralı mesajı oku |
| `AT+CMGDA="DEL ALL"` | Tüm mesajları sil (metin modunda) |

Türkçe karakterli SMS için `AT+CSCS="UCS2"` ve UCS2 (hex) metin gerekir; örnekler 7-bit GSM metin kullanır.

## 4. Sesli arama

| Komut | Ne yapar |
| --- | --- |
| `ATD+905XXXXXXXXX;` | Numarayı ara (sondaki `;` sesli arama demektir) |
| `ATA` | Gelen aramayı cevapla |
| `ATH` | Aramayı kapat / reddet |
| `AT+CLIP=1` | Gelen aramada arayan numarayı göster: `RING` ardından `+CLIP: "+905...",145,...` |
| `AT+CLVL=80` | Hoparlör seviyesi (0–100) |
| `AT+CMIC=0,10` | Mikrofon kazancı |

## 5. GPRS ve HTTP

```text
AT+SAPBR=3,1,"Contype","GPRS"
AT+SAPBR=3,1,"APN","internet"      ← operatörünüzün APN'i
AT+SAPBR=1,1                       ← taşıyıcıyı aç
AT+SAPBR=2,1                       → +SAPBR: 1,1,"10.x.x.x"  (1,1 = bağlı)
AT+HTTPINIT
AT+HTTPPARA="CID",1
AT+HTTPPARA="URL","http://example.com/"
AT+HTTPACTION=0                    → OK, ardından +HTTPACTION: 0,200,1256  (0 = GET, 200 = durum, 1256 = bayt)
AT+HTTPREAD=0,200                  ← cevabın ilk 200 baytı
AT+HTTPTERM
AT+SAPBR=0,1                       ← taşıyıcıyı kapat
```

POST için: `AT+HTTPPARA="CONTENT","application/json"`, `AT+HTTPDATA=<bayt>,10000` (`DOWNLOAD` gelince
veriyi gönderin), ardından `AT+HTTPACTION=1`.

Türkiye'de Turkcell, Vodafone ve Türk Telekom için APN genellikle `internet`'tir. SIM800C'de TLS desteği
sınırlıdır; `http://` adresler kullanın.

## 6. Sık karşılaşılan durumlar

| Belirti | Olası neden |
| --- | --- |
| `AT`'ye cevap yok | 12 V adaptör takılı değil, baud hızı eşleşmedi (birkaç kez `AT` gönderin) |
| `+CPIN: SIM PIN` | SIM'in PIN kodu açık: `AT+CPIN="1234"` ya da PIN'i telefonda kapatın |
| `+CREG: 0,2` uzun sürüyor | Şebeke aranıyor: anten takılı mı, bölgede 2G var mı |
| `+CREG: 0,3` | Kayıt reddedildi: SIM aktif mi, operatör 2G'yi destekliyor mu |
| Modem kendini yeniden başlatıyor | Besleme yetersiz: 12 V / en az 1 A adaptör kullanın |
| `+HTTPACTION: 0,601,0` | Ağ hatası: APN yanlış ya da veri paketi yok |
