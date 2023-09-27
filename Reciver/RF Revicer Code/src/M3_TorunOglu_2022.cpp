//*---- V1.2  ----*//
//*----	Model_3 ----*//

#include <Arduino.h>     //Arduino Library
#include <VirtualWire.h> //433RF library
#include <avr/wdt.h>     // Watchdog için gerekli kütüphane
/**************************************************************/
//*Şifreleme
uint8_t kimlik_dogrulama_key = 25; // todo Bu Şifre Alıcı Ve Verici Eşleşmesi İçin Aynı Olmalıdır (20-255 Arasında değer olmalı)
/**************************************************************/
//* Pin Out
const int RF_module_pin = 11; // 433mhz Alıcının Bağlı Olduğu Pin (interrupt olsa iyi olur)
const int RX_data_led = 14;   // RF'den Gelen Veri Olduğunda Bildirilecek led
/**************************************************************/
//* değişken
bool yetki = false; // Kumanda Şifrelemede Yetkilendirme Değişkeni
bool flag = false;  // Tuşa basılı tuttuğu sürece saçmalamaması için

bool bit7, bit6, bit5, bit4, bit3, bit2, bit1, bit0;       // Mainboard'a Gidecek Birinci Veri
bool bit15, bit14, bit13, bit12, bit11, bit10, bit9, bit8; // Mainboard'a Gidecek  İkinci Veri
/**************************************************************/
//* ON OFF Sistem için
bool otomatik_aktif = false; // On-Off İçin Değişlen
bool makina_aktif = false;   // On-Off İçin Değişlen
bool depo_aktif = false;     // On-Off İçin Değişlen
/**************************************************************/
//* Zamanlama
unsigned long ISR1_Zaman = 50;    // 1.Veriyi Gönderecek Süre
unsigned long ISR1_evvelkiMILLIS; // 1.Veriyi Gönderecek Süre

//* Milis Fonksiyonu
unsigned long ISR2_Zaman = 1000;  // Kumandadan Veri Kesilince Sistemin Sıfırlanacağı Süre
unsigned long ISR2_evvelkiMILLIS; // Kumandadan Veri Kesilince Sistemin Sıfırlanacağı Süre
/**************************************************************/

void rf_data(String data) //* Gelen Datanın Ayıklanması
{
  if (data == "0") //* Tuş Bırakıldığında
  {
    bit0 = false, bit1 = false, bit2 = false, bit3 = false, bit4 = false, bit5 = false, bit6 = false, bit7 = false;
    bit8 = false, bit9 = false, bit10 = false, bit11 = false, bit12 = false, bit13 = false, bit14 = false, bit15 = false;
    flag = false; // Flag'ı Pasifleştir
    if (otomatik_aktif == true)
    {
      bit0 = true;
    }
    if (makina_aktif == true)
    {
      bit13 = true;
    }
    if (depo_aktif == true)
    {
      bit5 = true;
    }
  }

  if ((flag == false) && (data != "0")) // işlemler birdan fazla yapılmaması için
  {
    if (data == "1") //* Otomatik ON/OFF
    {
      if (otomatik_aktif) // Otomatik Aktif ise
      {
        otomatik_aktif = false; // Otomatiği Kapat
        bit0 = false;           // Otomatiği Kapat
      }
      else // Otomatik Aktif Değilse
      {
        otomatik_aktif = true;
        bit0 = true;
      }
    }
    if (data == "2") //* Makina ON/OFF
    {
      if (makina_aktif) // Makina Aktif ise
      {
        makina_aktif = false; // Makina Kapat
        bit6 = false;         // Makina Kapat
      }
      else // Makina Aktif Değilse
      {
        makina_aktif = true;
        bit6 = true;
      }
    }
    if (data == "13") //* Depo ON/OFF
    {
      if (depo_aktif) // depo Aktif ise
      {
        depo_aktif = false; // depo Kapat
        bit5 = false;       // depo Kapat
        bit6 = true;        // Makina Ters
        bit4 = true;        // Boşalt Durdur
      }
      else // depo Aktif Değilse
      {
        depo_aktif = true;
        bit5 = true;
      }
    }

    if (data == "3") //* Sökücü Yukarı
      bit4 = true;

    if (data == "4") //* Sökücü Aşağı
      bit3 = true;

    if (data == "17") //* Sökücü Sağa
      bit2 = true;

    if (data == "6") //* Sökücü Sola
      bit1 = true;

    if (data == "11") //* Kırma Kapalı
      bit7 = true;

    if (data == "12") //* Kırma Açık
      bit8 = true;

    if (data == "9") //* Hazırlayıcı Yukarı
      bit9 = true;

    if (data == "10") //* Hazırlayıcı Aşağı
      bit10 = true;

    if (data == "13") //* Boşalt Çalıştır
      bit5 = true;

    if (data == "16") //* Makina Ters
      bit15 = true;

    flag = true; // Flag'ı Etkinleştir
  }
}

void setup()
{
  Serial.begin(1200);           // Seri Monitör
  vw_set_rx_pin(RF_module_pin); // 433mhz Modül Pin'i Belirlenmesi
  vw_setup(2000);               // Saniye/Bit
  vw_rx_start();                // Start the receiver PLL running

  //* Pin Output
  pinMode(RX_data_led, OUTPUT); // Pin'i Çıkış Olarak Belirle

  //* watchdog
  wdt_disable();       // Watchdog'u devre dışı bırakın ve 2 saniyeden fazla bekleyin
  delay(3000);         // Yanlış yapılandırma durumunda Arduino'nun sonsuza kadar sıfırlanmaya devam etmemesi için yapıldı
  wdt_enable(WDTO_2S); // 2 saniyelik bir zaman aşımı ile bekçi köpeğini etkinleştir
}

void loop()
{
  /**************************************************************/
  unsigned long currentMillis = millis(); // zamanlayıcıyı oku
  /**************************************************************/
  // RF Gelen Data
  uint8_t buf[VW_MAX_MESSAGE_LEN];
  uint8_t buflen = VW_MAX_MESSAGE_LEN;

  if (vw_get_message(buf, &buflen)) //* RF Veri Gelirse;
  {
    digitalWrite(RX_data_led, HIGH); // Veri Geldiğini Belirten Led
    for (int i = 0; i < 2; i++)      // Gelen Verinin 2 Satırını Al
    {
      if (i == 0)                           // İlk Stır
      {                                     // İlk Satır Kumanda Yetkisi İçin
        String sifre = String(buf[i], DEC); // Gelen Veriyi Oku
        int sifre_int = sifre.toInt();      // string'i int çevir

        if (sifre_int == kimlik_dogrulama_key) // Yetkiyi Sorgula
          yetki = true;                        // Yetki Verildi
        else
          yetki = false; // Yetkisiz Kumanda
      }
      if ((i == 1) && (yetki == true)) // 2. Satır
        rf_data(String(buf[i], DEC));  // Gelen Veriyi Void'e Aktar
    }
    ISR2_evvelkiMILLIS = currentMillis;
    digitalWrite(RX_data_led, LOW); // Verinin Bittiğini Belirten Led
  }
  else
  {
    if (currentMillis - ISR2_evvelkiMILLIS >= ISR2_Zaman)
    {
      rf_data("0"); // Gelen Veriyi Void'e Aktar
    }
  }
  /**************************************************************/
  if (currentMillis - ISR1_evvelkiMILLIS >= ISR1_Zaman) // Zamanlayıcı
  {
    ISR1_evvelkiMILLIS = currentMillis;

    byte binaryData_1 = 0;
    binaryData_1 |= bit0;
    binaryData_1 |= bit1 << 1;
    binaryData_1 |= bit2 << 2;
    binaryData_1 |= bit3 << 3;
    binaryData_1 |= bit4 << 4;
    binaryData_1 |= bit5 << 5;
    binaryData_1 |= bit6 << 6;
    binaryData_1 |= bit7 << 7;
    byte binaryData_2 = 0;
    binaryData_2 |= bit8;
    binaryData_2 |= bit9 << 1;
    binaryData_2 |= bit10 << 2;
    binaryData_2 |= bit11 << 3;
    binaryData_2 |= bit12 << 4;
    binaryData_2 |= bit13 << 5;
    binaryData_2 |= bit14 << 6;
    binaryData_2 |= bit15 << 7;

    //* Mainboard'a Gönderilen Veri
    Serial.write(68);
    Serial.write(84);
    Serial.write(89);
    Serial.write(binaryData_1);
    Serial.write(binaryData_2);
  }

  wdt_reset(); // watchdog Yenile (Aksi Halde 2sn içinye resset çeker)
}