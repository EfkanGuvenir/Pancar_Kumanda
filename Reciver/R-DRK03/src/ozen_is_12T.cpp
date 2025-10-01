//*---- R-DRK03 V25.0.3 ----*//
//*----	Model_3 12T ----*//
//* ÖzenİŞ 2016 Model'de test Edildi. *//
//* ÖzenİŞ 2011 Model'de test Edildi. *//

#include <Arduino.h>     //Arduino Library
#include <VirtualWire.h> //433RF library
#include <avr/wdt.h>     // Watchdog için gerekli kütüphane
/**************************************************************/
//*Şifreleme
uint8_t kimlik_dogrulama_key = 100; // todo Bu Şifre Alıcı Ve Verici Eşleşmesi İçin Aynı Olmalıdır (100-255 Arasında değer olmalı)
/**************************************************************/
//* değişken
bool yetki = false;         // Kumanda Şifrelemede Yetkilendirme Değişkeni
bool flag = false;          // Tuşa basılı tuttuğu sürece saçmalamaması için
uint8_t onceki_komut = 255; // Önceki komutu saklamak için (255 = başlangıç değeri)

constexpr bool bit0 = true;                          // Boşta Sürekli bit pasif olduğu için
constexpr bool bit8 = false;                         // Boşta Sürekli bit aktif olduğu için
bool bit7, bit6, bit5, bit4, bit3, bit2, bit1;       // Mainboard'a Gidecek Birinci Veri
bool bit15, bit14, bit13, bit12, bit11, bit10, bit9; // Mainboard'a Gidecek  İkinci Veri
/**************************************************************/
//* ON OFF Sistem için
bool otomatik_aktif = false; // On-Off İçin Değişlen
bool makina_aktif = false;   // On-Off İçin Değişlen
bool depo_aktif = false;     // On-Off İçin Değişlen
/**************************************************************/
//* Zamanlama
unsigned long ISR1_Zaman = 50;    // 1.Veriyi Gönderecek Süre
unsigned long ISR1_evvelkiMILLIS; // 1.Veriyi Gönderecek Süre

unsigned long ISR2_Zaman = 200;   // Gelen Veri Olmadığında Tuş Bırakılmış Sayılacak Süre
unsigned long ISR2_evvelkiMILLIS; // Gelen Veri Olmadığında Tuş Bırakılmış Sayılacak Süre

bool data_send_flag;              // İki Ayrı data göndereceği süre için
/**************************************************************/
//* Pin Out
const int RF_module_pin = 2; // 433mhz Alıcının Bağlı Olduğu Pin (interrupt olsa iyi olur)
const int RX_data_led = 10;  // RF'den Gelen Veri Olduğunda Bildirilecek led
/**************************************************************/

void rf_data(uint8_t komut) //* Gelen Komutun Ayıklanması
{
  if (komut == 0) //* Tuş Bırakıldığında
  {
    bit1 = false, bit2 = false, bit3 = false, bit4 = false, bit5 = false, bit6 = false, bit7 = false;
    bit9 = false, bit10 = false, bit11 = false, bit12 = false, bit13 = false, bit14 = false, bit15 = false;
    flag = false; // Flag'ı Pasifleştir

    if (otomatik_aktif == true)
      bit9 = true;
  }

  if ((flag == false) && (komut != 0)) // işlemler birdan fazla yapılmaması için
  {
    if (komut == 72) //* Otomatik ON/OFF
    {
      otomatik_aktif = true;
    }

    if (komut == 80) //* Makina ON/OFF (Yedek)
    {
      otomatik_aktif = false;
    }

    if (komut == 70) //* Depo ON/OFF
    {
      if (depo_aktif) // depo Aktif ise
      {
        bit7 = true;
        bit15 = true;
        depo_aktif = false; // depo Kapat
      }
      else // depo Aktif Değilse
      {
        bit6 = true;
        bit14 = true;
        depo_aktif = true;
      }
    }

    if (komut == 76) //* Sökücü Yukarı
      bit4 = true;

    if (komut == 75) //* Sökücü Aşağı
      bit3 = true;

    if (komut == 79) //* Sökücü Sağa
      bit11 = true;

    if (komut == 71) //* Sökücü Sola
      bit10 = true;

    if (komut == 78) //* Kırma Kapalı
      bit1 = true;

    if (komut == 74) //* Kırma Açık
      bit2 = true;

    if (komut == 69) //* Hazırlayıcı Yukarı
      bit12 = true;

    if (komut == 73) //* Hazırlayıcı Aşağı
      bit13 = true;

    if (komut == 77) //* Makina Ters
      bit5 = true;

    flag = true; // Flag'ı Etkinleştir
  }
}

void setup()
{
  Serial.begin(2400);           // Seri Monitör
  vw_set_rx_pin(RF_module_pin); // 433mhz Modül Pin'i Belirlenmesi
  vw_setup(2000);               // Saniye/Bit
  vw_rx_start();                // Start the receiver PLL running

  //* Pin Output
  pinMode(RX_data_led, OUTPUT);    // Pin'i Çıkış Olarak Belirle
  digitalWrite(RX_data_led, HIGH); // Veri Geldiğini Belirten Led

  //* watchdog
  wdt_disable();                  // Watchdog'u devre dışı bırakın ve 2 saniyeden fazla bekleyin
  delay(3000);                    // Yanlış yapılandırma durumunda Arduino'nun sonsuza kadar sıfırlanmaya devam etmemesi için yapıldı
  digitalWrite(RX_data_led, LOW); // Veri Geldiğini Belirten Led
  wdt_enable(WDTO_2S);            // 2 saniyelik bir zaman aşımı ile bekçi köpeğini etkinleştir
}

void loop()
{
  /**************************************************************/
  unsigned long currentMillis = millis(); // zamanlayıcıyı oku
  /**************************************************************/
  // RF Gelen Data
  uint8_t buf[VW_MAX_MESSAGE_LEN];
  uint8_t buflen = VW_MAX_MESSAGE_LEN;

  if (vw_get_message(buf, &buflen)) // Non-blocking
  {

    // İlk byte kimlik doğrulama, ikinci byte komut
    if (buflen >= 2) // En az 2 byte geldiğinden emin ol
    {
      uint8_t sifre_int = buf[0];            // İlk byte: kimlik doğrulama
      if (sifre_int == kimlik_dogrulama_key) // Yetkiyi Sorgula
      {
        digitalWrite(RX_data_led, HIGH); // Veri Geldiğini Belirten Led
        uint8_t komut = buf[1];          // İkinci byte: komut
        ISR2_evvelkiMILLIS = currentMillis;
        // Ard arda gelen aynı komutları filtrele
        if (komut != onceki_komut)
        {
          rf_data(komut);       // Komutu rf_data'ya gönder
          onceki_komut = komut; // Önceki komutu güncelle
          ISR2_evvelkiMILLIS = currentMillis;
        }
        if (komut == 0)
          digitalWrite(RX_data_led, LOW); // Veri Geldiğini Belirten Led
      }
    }
  }
  /**************************************************************/
  if (currentMillis - ISR2_evvelkiMILLIS >= ISR2_Zaman) //
  {
    // Veri kesintisi sürüyorsa ve son durum zaten 'bırakıldı' değilse, bir kez bırakma uygula
    if (onceki_komut != 0 || flag)
    {
      rf_data(0);       // Tuş bırakıldı mantığını uygula
      onceki_komut = 0; // Yinelenmeyi önle
      digitalWrite(RX_data_led, LOW);
    }
    ISR2_evvelkiMILLIS = currentMillis; // Süreyi tekrar başlat
  }
  /**************************************************************/
  if (currentMillis - ISR1_evvelkiMILLIS >= ISR1_Zaman) // Zamanlayıcı
  {
    ISR1_evvelkiMILLIS = currentMillis;
    data_send_flag = !data_send_flag; // Sırayla Veri Göndermek için

    // todo 1. Veri
    if (data_send_flag == true)
    {
      byte binaryData_1 = 0;
      binaryData_1 |= bit0;
      binaryData_1 |= bit1 << 1;
      binaryData_1 |= bit2 << 2;
      binaryData_1 |= bit3 << 3;
      binaryData_1 |= bit4 << 4;
      binaryData_1 |= bit5 << 5;
      binaryData_1 |= bit6 << 6;
      binaryData_1 |= bit7 << 7;
      Serial.write(binaryData_1); // Mainboard'a Gönderilen Veri
    }
    // todo 2. Veri
    if (data_send_flag == false)
    {
      byte binaryData_2 = 0;
      binaryData_2 |= bit8;
      binaryData_2 |= bit9 << 1;
      binaryData_2 |= bit10 << 2;
      binaryData_2 |= bit11 << 3;
      binaryData_2 |= bit12 << 4;
      binaryData_2 |= bit13 << 5;
      binaryData_2 |= bit14 << 6;
      binaryData_2 |= bit15 << 7;
      Serial.write(binaryData_2); // Mainboard'a Gönderilen Veri
    }
  }
  /**************************************************************/
  wdt_reset(); // watchdog Yenile (Aksi Halde 2sn içinye resset çeker)
}