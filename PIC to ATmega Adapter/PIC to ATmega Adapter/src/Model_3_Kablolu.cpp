//*----PIC to ATmega Adapter V1.0  ----*//
//*---- Uyumlu Anakart: mainboard ver6.3 model  ----*//
//*---- Model_3 Kablolu Kumandaya Uygundur----*//

#include <Arduino.h>
#include <VirtualWire.h> //433RF library
#include <avr/wdt.h>     // Watchdog için gerekli kütüphane
/**************************************************************/

/**************************************************************/
//* ON OFF Sistem için
bool otomatik_aktif = false; // On-Off İçin Değişken
bool sokum_aktif = false;    // On-Off İçin Değişken
bool depo_aktif = false;     // On-Off İçin Değişken
bool lamba_aktif = false;    // On-Off İçin Değişken
/**************************************************************/
//* Değişkenler
bool yetki = false;        // Kumanda Şifrelemede Yetkilendirme Değişkeni
bool flag = false;         // Tuşa basılı tuttuğu sürece saçmalamaması için
bool otomatik_flag = true; // sistem otomatikteyken kumandadan da işlem görmesi için

/**************************************************************/
//* Pin IN
const int RX_data_led = 10; // RF'den Gelen Veri Olduğunda Bildirilecek led

const int depo_sensor = 1;
const int sokum_sensor = 0;
const int asagi_sensor = 24;
const int yukari_sensor = 25;
const int saga_sensor = 26;
const int sola_sensor = 27;
//* Pin Out
const int ortak_valf = 2;               //* Mos-1
const int ok_sol_valf = 4;              //* Mos-2
const int ok_sag_valf = 29;             //* Mos-3
const int bosalt_valf = 31;             //* Mos-4
const int bosalt_dur_valf = 22;         //* Mos-5
const int sepet_yukari_valf = 20;       //* Mos-6
const int sepet_asagi_valf = 18;        //* Mos-7
const int sokum_yukari_valf = 28;       //* Mos-8
const int sokum_asagi_valf = 12;        //* Mos-9
const int hazirlayici_asagi_valf = 3;   //* Mos-10
const int hazirlayici_yukari_valf = 30; //* Mos-11
const int sokum_valf = 23;              //* Mos-12
const int geri_valf = 21;               //* Mos-13
const int ortak_2_valf = 19;            //* Mos-14
const int lamba_valf = 11;              //* Mos-15
const int yedek_1_valf = 15;            //* Mos-16
/**************************************************************/
//* Zamanlayıcı
unsigned long ISR1_Zaman = 10; // ms
unsigned long ISR1_evvelkiMILLIS;
/**************************************************************/

void rf_data(byte bayt_4, byte bayt_5) //* Gelen Datanın Ayıklanması
{
  //* Gelen 3. Data
  byte receivedData1 = bayt_4; // Gelen veri buraya atanacak

  // Gelen veriden her bir biti alıp ayrı değişkenlere atama
  byte bit0 = receivedData1 & 0x01;
  byte bit1 = (receivedData1 >> 1) & 0x01;
  byte bit2 = (receivedData1 >> 2) & 0x01;
  byte bit3 = (receivedData1 >> 3) & 0x01;
  byte bit4 = (receivedData1 >> 4) & 0x01;
  byte bit5 = (receivedData1 >> 5) & 0x01;
  byte bit6 = (receivedData1 >> 6) & 0x01;
  byte bit7 = (receivedData1 >> 7) & 0x01;

  byte binaryData_1 = 0;
  binaryData_1 |= bit0;
  binaryData_1 |= bit1 << 1;
  binaryData_1 |= bit2 << 2;
  binaryData_1 |= bit3 << 3;
  binaryData_1 |= bit4 << 4;
  binaryData_1 |= bit5 << 5;
  binaryData_1 |= bit6 << 6;
  binaryData_1 |= bit7 << 7;
  /**************************************************************/
  //* Gelen 4. Data
  byte receivedData2 = bayt_5; // Gelen veri buraya atanacak

  // Gelen veriden her bir biti alıp ayrı değişkenlere atama
  byte bit8 = receivedData2 & 0x01;
  byte bit9 = (receivedData2 >> 1) & 0x01;
  byte bit10 = (receivedData2 >> 2) & 0x01;
  byte bit11 = (receivedData2 >> 3) & 0x01;
  byte bit12 = (receivedData2 >> 4) & 0x01;
  byte bit13 = (receivedData2 >> 5) & 0x01;
  byte bit14 = (receivedData2 >> 6) & 0x01;
  byte bit15 = (receivedData2 >> 7) & 0x01;

  byte binaryData_2 = 0;
  binaryData_2 |= bit8;
  binaryData_2 |= bit9 << 1;
  binaryData_2 |= bit10 << 2;
  binaryData_2 |= bit11 << 3;
  binaryData_2 |= bit12 << 4;
  binaryData_2 |= bit13 << 5;
  binaryData_2 |= bit14 << 6;
  binaryData_2 |= bit15 << 7;

  if (bit0 == true) //* Otomatik ON

  {
    otomatik_aktif = true;
  }
  else
  {
    otomatik_aktif = false;
  }

  if (bit1 == true) //* Sökücü Sol
  {
    digitalWrite(ok_sol_valf, HIGH);
  }
  else
  {
    digitalWrite(ok_sol_valf, LOW);
  }

  if (bit2 == true) //* Sökücü Sağ
  {
    digitalWrite(ok_sag_valf, HIGH);
  }
  else
  {
    digitalWrite(ok_sag_valf, LOW);
  }

  if (bit3 == true) //* Sökücü Aşağı
  {
    digitalWrite(sokum_asagi_valf, HIGH);
  }
  else
  {
    digitalWrite(sokum_asagi_valf, LOW);
  }

  if (bit4 == true) //* Sökücü Yukarı
  {
    digitalWrite(sokum_yukari_valf, HIGH);
  }
  else
  {

    digitalWrite(sokum_yukari_valf, LOW);
  }

  if (bit5 == true) //* Depo ON
  {
    if ((digitalRead(depo_sensor) == HIGH) && (depo_aktif == false))
      digitalWrite(bosalt_valf, HIGH);
  }
  if (bit6 == true) //* Boşta || Depo OFF
  {
    digitalWrite(bosalt_valf, LOW);
  }

  if (bit7 == true) //* Kırma Kapalı
  {
    digitalWrite(sepet_yukari_valf, HIGH);
  }
  else
  {
    digitalWrite(sepet_yukari_valf, LOW);
  }

  if (bit8 == true) //* Kırma Açık
  {
    digitalWrite(sepet_asagi_valf, HIGH);
  }
  else
  {
    digitalWrite(sepet_asagi_valf, LOW);
  }

  if (bit9 == true) //* Hazırlayıcı Yukarı
  {
    digitalWrite(hazirlayici_yukari_valf, HIGH);
  }
  else
  {
    digitalWrite(hazirlayici_yukari_valf, LOW);
  }

  if (bit10 == true) //* Hazırlayıcı Aşağı
  {
    digitalWrite(hazirlayici_asagi_valf, HIGH);
  }
  else
  {

    digitalWrite(hazirlayici_asagi_valf, LOW);
  }

  if (bit13 == true) //* Makina ON
  {
    digitalWrite(sokum_valf, HIGH);
  }
  else
  {
    digitalWrite(sokum_valf, LOW);
  }

  if (bit15 == true) //* Makina Ters
  {
    digitalWrite(geri_valf, HIGH);
  }
  else
  {
    digitalWrite(geri_valf, LOW);
  }

  if (bit1 == true || bit2 == true || bit3 == true || bit4 == true || bit7 == true || bit8 == true || bit9 == true || bit10 == true) //* Ortak Valf'de Çalışacak Komutlar
  {
    otomatik_flag = false;
    digitalWrite(ortak_valf, HIGH);
  }
  else
  {
    otomatik_flag = true;
    digitalWrite(ortak_valf, LOW);
  }
}

void setup()
{
  /**************************************************************/
  //* Pin Out
  pinMode(RX_data_led, OUTPUT); // Pin'i Çıkış Olarak Belirle

  pinMode(ortak_valf, OUTPUT);
  pinMode(ok_sol_valf, OUTPUT);
  pinMode(ok_sag_valf, OUTPUT);
  pinMode(bosalt_valf, OUTPUT);
  pinMode(bosalt_dur_valf, OUTPUT);
  pinMode(sepet_yukari_valf, OUTPUT);
  pinMode(sepet_asagi_valf, OUTPUT);
  pinMode(sokum_yukari_valf, OUTPUT);
  pinMode(sokum_asagi_valf, OUTPUT);
  pinMode(hazirlayici_asagi_valf, OUTPUT);
  pinMode(hazirlayici_yukari_valf, OUTPUT);
  pinMode(sokum_valf, OUTPUT);
  pinMode(geri_valf, OUTPUT);
  pinMode(ortak_2_valf, OUTPUT);
  pinMode(lamba_valf, OUTPUT);
  pinMode(yedek_1_valf, OUTPUT);

  digitalWrite(ok_sol_valf, LOW);
  digitalWrite(ok_sag_valf, LOW);
  digitalWrite(bosalt_valf, LOW);
  digitalWrite(bosalt_dur_valf, LOW);
  digitalWrite(sepet_yukari_valf, LOW);
  digitalWrite(sepet_asagi_valf, LOW);
  digitalWrite(sokum_yukari_valf, LOW);
  digitalWrite(sokum_asagi_valf, LOW);
  digitalWrite(hazirlayici_asagi_valf, LOW);
  digitalWrite(hazirlayici_yukari_valf, LOW);
  digitalWrite(sokum_valf, LOW);
  digitalWrite(geri_valf, LOW);
  digitalWrite(ortak_2_valf, LOW);
  digitalWrite(lamba_valf, LOW);
  digitalWrite(yedek_1_valf, LOW);
  //* Pin IN
  pinMode(depo_sensor, INPUT);
  pinMode(sokum_sensor, INPUT);
  pinMode(asagi_sensor, INPUT);
  pinMode(yukari_sensor, INPUT);
  pinMode(saga_sensor, INPUT);
  pinMode(sola_sensor, INPUT);
  /**************************************************************/
  wdt_disable(); // Watchdog'u devre dışı bırakın ve 2 saniyeden fazla bekleyin

  // İlek Beslendiğinde Sökğm ve hazırlayıcıyı yukarı çeker
  digitalWrite(ortak_valf, HIGH);
  digitalWrite(sokum_yukari_valf, HIGH);
  delay(3000);
  digitalWrite(sokum_yukari_valf, LOW);
  digitalWrite(hazirlayici_yukari_valf, HIGH);
  delay(3000);
  digitalWrite(ortak_valf, LOW);
  digitalWrite(hazirlayici_yukari_valf, LOW);
  /**************************************************************/
  //* watchdog
  // delay(3000);         // Yanlış yapılandırma durumunda Arduino'nun sonsuza kadar sıfırlanmaya devam etmemesi için yapıldı
  wdt_enable(WDTO_2S); // 2 saniyelik bir zaman aşımı ile bekçi köpeğini etkinleştir
  /**************************************************************/
}

void loop()
{
  unsigned long currentMillis = millis(); // zamanlayıcıyı oku

  if (currentMillis - ISR1_evvelkiMILLIS >= ISR1_Zaman)
  {
    Serial.begin(1200);
    ISR1_evvelkiMILLIS = currentMillis;

    if (Serial.available() >= 5)
    { // 5 baytlık veri gelene kadar bekle
      digitalWrite(RX_data_led, HIGH);
      // İlk üç sabit baytı kontrol et
      byte byte1 = Serial.read();
      byte byte2 = Serial.read();
      byte byte3 = Serial.read();

      // İlk üç sabit bayt 68, 84 ve 89 mu diye kontrol et
      if (byte1 == 68 && byte2 == 84 && byte3 == 89)
      {
        // Eğer doğruysa, son iki değişken baytı al
        byte byte4 = Serial.read();
        byte byte5 = Serial.read();
        rf_data(byte4, byte5);
        // Serial.print(byte4, DEC);
        // Serial.print(", ");
        // Serial.println(byte5, DEC);
        Serial.end();
        // delay(1);
      }
    }
    digitalWrite(RX_data_led, LOW);
  }
  /**************************************************************/
  if ((otomatik_aktif == true) && (otomatik_flag == true)) //* Otomatik yönlendirme
  {
    int asagi_sensor_val = digitalRead(asagi_sensor);
    int yukari_sensor_val = digitalRead(yukari_sensor);
    int saga_sensor_val = digitalRead(saga_sensor);
    int sola_sensor_val = digitalRead(sola_sensor);

    if (asagi_sensor_val == HIGH)
    {
      digitalWrite(ortak_valf, HIGH);
      digitalWrite(sokum_asagi_valf, HIGH);
    }

    if (asagi_sensor_val == LOW)
      digitalWrite(sokum_asagi_valf, LOW);

    if (yukari_sensor_val == HIGH)
    {
      digitalWrite(ortak_valf, HIGH);
      digitalWrite(sokum_yukari_valf, HIGH);
    }
    else
      digitalWrite(sokum_yukari_valf, LOW);

    if (saga_sensor_val == HIGH)
    {
      digitalWrite(ortak_valf, HIGH);
      digitalWrite(ok_sag_valf, HIGH);
    }
    else
      digitalWrite(ok_sag_valf, LOW);

    if (sola_sensor_val == HIGH)
    {
      digitalWrite(ortak_valf, HIGH);
      digitalWrite(ok_sol_valf, HIGH);
    }
    else
      digitalWrite(ok_sol_valf, LOW);

    if ((asagi_sensor_val == LOW) && (yukari_sensor_val == LOW) && (saga_sensor_val == LOW) && (sola_sensor_val == LOW) && (otomatik_flag == true))
      digitalWrite(ortak_valf, LOW);
  }

  wdt_reset(); // watchdog Yenile (Aksi Halde 2sn içinye resset çeker)
}
