//*----PIC to ATmega Adapter V1.0  ----*//
//*---- Uyumlu Anakart: mainboard ver6.3 model  ----*//
//*---- Model_1 Kablosuz Kumandaya Uygundur----*//

#include <Arduino.h>
#include <VirtualWire.h> //433RF library
#include <avr/wdt.h>     // Watchdog için gerekli kütüphane
/**************************************************************/
//* Şifreleme
uint8_t kimlik_dogrulama_key = 3; // todo Bu Şifre Alıcı Ve Verici Eşleşmesi İçin Aynı Olmalıdır (0-255 Arasında değer olmalı)
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
const int RF_module_pin = 8; // 433mhz Alıcının Bağlı Olduğu Pin (interrupt olsa iyi olur)
const int RX_data_led = 10;  // RF'den Gelen Veri Olduğunda Bildirilecek led

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

void rf_data(String data) //* Gelen Datanın Ayıklanması
{
  /**************************************************************/
  if (data == "0") //* Tuş Bırakıldığında
  {

    digitalWrite(ok_sol_valf, LOW);
    digitalWrite(ok_sag_valf, LOW);
    digitalWrite(bosalt_dur_valf, LOW);
    digitalWrite(sepet_yukari_valf, LOW);
    digitalWrite(sepet_asagi_valf, LOW);
    digitalWrite(sokum_yukari_valf, LOW);
    digitalWrite(sokum_asagi_valf, LOW);
    digitalWrite(hazirlayici_asagi_valf, LOW);
    digitalWrite(hazirlayici_yukari_valf, LOW);
    digitalWrite(geri_valf, LOW);
    digitalWrite(ortak_valf, LOW);
    digitalWrite(ortak_2_valf, LOW);
    digitalWrite(lamba_valf, LOW);
    digitalWrite(yedek_1_valf, LOW);

    if (sokum_aktif == true)
    {
      digitalWrite(lamba_valf, HIGH);
      digitalWrite(sokum_valf, HIGH);
    }
    else
    {
      digitalWrite(lamba_valf, LOW);
      digitalWrite(sokum_valf, LOW);
    }

    if (depo_aktif == true)
      digitalWrite(bosalt_valf, HIGH);
    else
      digitalWrite(bosalt_valf, LOW);

    if (lamba_aktif == true)
      digitalWrite(yedek_1_valf, HIGH);
    else
      digitalWrite(yedek_1_valf, LOW);

    flag = false; // Flag'ı Pasifleştir

    otomatik_flag = true; // Otomatik Sistem Devreye Girebilir duruma getir
  }
  /**************************************************************/
  if ((flag == false) && (data != "0")) // işlemler birdan fazla yapılmaması için
  {
    if (data == "1") //* Otomatik
    {
      otomatik_aktif = !otomatik_aktif;
    }
    if (data == "13") //* Boşalt Çalıştır
    {
      if ((digitalRead(depo_sensor) == HIGH) && (depo_aktif == false))
        depo_aktif = true;
    }
    if (data == "14") //* Boşalt Durdur
    {
      depo_aktif = false;
    }
    if (data == "2") //* Söküm ON/OFF
    {
      sokum_aktif = !sokum_aktif;
    }
    if (data == "3") //* Sökücü Yukarı
    {
      digitalWrite(ortak_valf, HIGH);
      digitalWrite(sokum_yukari_valf, HIGH);
    }
    if (data == "4") //* Sökücü Aşağı
    {
      digitalWrite(ortak_valf, HIGH);
      digitalWrite(sokum_asagi_valf, HIGH);
    }
    if (data == "17") //* Sökücü Sağ
    {
      digitalWrite(ortak_valf, HIGH);
      digitalWrite(ok_sag_valf, HIGH);
    }
    if (data == "6") //* Sökücü Sol
    {
      digitalWrite(ortak_valf, HIGH);
      digitalWrite(ok_sol_valf, HIGH);
    }
    if (data == "16") //* Makina Ters
    {
      if (digitalRead(sokum_valf) == HIGH) //* Makina Geri
      {
        digitalWrite(ortak_2_valf, HIGH);
        digitalWrite(sokum_valf, LOW);
        digitalWrite(geri_valf, HIGH);
      }
      if (digitalRead(bosalt_valf) == HIGH) //*boşalt Geri
      {
        digitalWrite(geri_valf, LOW);
        digitalWrite(bosalt_valf, LOW);
        depo_aktif = false;
        digitalWrite(bosalt_dur_valf, HIGH);
      }
      else
      {
        digitalWrite(ortak_2_valf, HIGH);
        digitalWrite(geri_valf, HIGH);
      }
    }
    if (data == "9") //* Hazırlayıcı Yukarı
    {
      digitalWrite(hazirlayici_yukari_valf, HIGH);
      digitalWrite(ortak_valf, HIGH);
    }
    if (data == "10") //* Hazırlayıcı Aşağı
    {
      digitalWrite(hazirlayici_asagi_valf, HIGH);
      digitalWrite(ortak_valf, HIGH);
    }
    if (data == "11") //* Kırma Kapalı
    {
      digitalWrite(sepet_yukari_valf, HIGH);
      digitalWrite(ortak_valf, HIGH);
    }
    if (data == "12") //* Kırma Açık
    {
      digitalWrite(sepet_asagi_valf, HIGH);
      digitalWrite(ortak_valf, HIGH);
    }
    if (data == "7") //* Depo Yukarı
    {
    }
    if (data == "8") //* Depo Aşağı
    {
    }
    if (data == "15") //* Lamba ON/OFF
    {
      lamba_aktif = !lamba_aktif;
    }
    flag = true; // Flag'ı Etkinleştir
  }
}

void setup()
{
  Serial.begin(1200);           // Seri Monitör
  vw_set_rx_pin(RF_module_pin); // 433mhz Modül Pin'i Belirlenmesi
  vw_setup(1000);               // Saniye/Bit
  vw_rx_start();                // Start the receiver PLL running
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
  uint8_t buf[VW_MAX_MESSAGE_LEN];
  uint8_t buflen = VW_MAX_MESSAGE_LEN;

  if (vw_get_message(buf, &buflen)) //* RF Veri Gelirse
  {
    digitalWrite(RX_data_led, HIGH); // Veri Geldiğini Belirten Led
    for (int i = 0; i < 2; i++)      // Gelen Verinin 2 Satırını Al
    {
      if (i == 0)                           // İlk Stır
      {                                     // İlk Satır Kumanda Yetkisi İçin
        String sifre = String(buf[i], DEC); // Gelen Veriyi Oku
        int sifre_int = sifre.toInt();      // string'i int çevir

        if (sifre_int == kimlik_dogrulama_key) // Yetkiyi Sorgula
        {
          yetki = true;          // Yetki Verildi
          otomatik_flag = false; // tuşa basılınca Otomatik Sistemi anlık olarak devredışı bırak
        }
        else
          yetki = false; // Yetkisiz Kumanda
      }
      if ((i == 1) && (yetki == true)) // 2. Satır
        rf_data(String(buf[i], DEC));  // Gelen Veriyi Void'e Aktar
    }
    digitalWrite(RX_data_led, LOW); // Verinin Bittiğini Belirten Led
  }

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