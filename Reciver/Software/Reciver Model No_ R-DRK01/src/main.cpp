//*---- DRK01 V1.1  ----*//
//*----	16-Tuş Model A'ya Uygundur.----*//

#include <Arduino.h>
#include <VirtualWire.h> //433RF library
#include <avr/wdt.h>     // Watchdog için gerekli kütüphane

//* Şifreleme
uint8_t kimlik_dogrulama_key = 0; // todo Bu Şifre Alıcı Ve Verici Eşleşmesi İçin Aynı Olmalıdır (0-255 Arasında değer olmalı)

//* Pin Out
const int RF_module_pin = 11; // 433mhz Alıcının Bağlı Olduğu Pin (interrupt olsa iyi olur)
const int otomatik_led = 12;  // Otomatik Söküm Sisteme Girdiğinde bildirim ışığı
const int sokum_led = 13;     // Söküm Sisteme Girdiğinde bildirim ışığı
const int RX_data_led = 14;   // RF'den Gelen Veri Olduğunda Bildirilecek led
const int Switch1 = 27;       // Şifreleme Anahtarının 1. biti
const int Switch2 = 26;       // Şifreleme Anahtarının 2. biti
const int Switch3 = 25;       // Şifreleme Anahtarının 3. biti
const int Switch4 = 34;       // Şifreleme Anahtarının 4. biti

//* değişken
bool yetki = false; // Kumanda Şifrelemede Yetkilendirme Değişkeni
bool flag = false;  // Tuşa basılı tuttuğu sürece saçmalamaması için

bool bit7, bit6, bit5, bit4, bit3, bit2, bit1, bit0;         // Mainboard'a Gidecek Birinci Veri
bool bit15, bit14, bit13, bit12, bit11, bit10, bit9, bit8;   // Mainboard'a Gidecek  İkinci Veri
bool bit23, bit22, bit21, bit20, bit19, bit18, bit17, bit16; // Mainboard'a Gidecek  Üçüncü Veri

//* ON OFF Sistem için
bool otomatik_aktif = false; // On-Off İçin Değişlen
bool sokum_aktif = false;    // On-Off İçin Değişlen
bool lamba_aktif = false;    // On-Off İçin Değişlen

void rf_data(String data) //* Gelen Datanın Ayıklanması
{
  if (data == "0") //* Tuş Bırakıldığında
  {
    bit0 = false, bit1 = false, bit2 = false, bit3 = false, bit4 = false, bit5 = false, bit6 = false, bit7 = false;
    bit8 = false, bit9 = false, bit10 = false, bit11 = false, bit12 = false, bit13 = false, bit14 = false, bit15 = false;
    bit16 = false, bit17 = false, bit18 = false, bit19 = false, bit20 = false, bit21 = false, bit22 = false, bit23 = false;
    flag = false; // Flag'ı Pasifleştir
    if (otomatik_aktif == true)
    {
      bit0 = true;
    }
    if (sokum_aktif == true)
    {
      bit13 = true;
    }
    if (lamba_aktif == true)
    {
    }
  }

  if ((flag == false) && (data != "0")) // işlemler birdan fazla yapılmaması için
  {
    if (data == "1") //* Otomatik
    {
      if (otomatik_aktif) // Otomatik Aktif ise
      {
        otomatik_aktif = false; // Otomatiği Kapat
        bit0 = false;           // Otomatiği Kapat
        digitalWrite(otomatik_led, LOW);
      }
      else // Otomatik Aktif Değilse
      {
        otomatik_aktif = true;
        bit0 = true;
        digitalWrite(otomatik_led, HIGH);
      }
    }

    if (data == "2") //* Söküm
    {
      if (sokum_aktif)
      {
        sokum_aktif = false;
        bit13 = false;
        digitalWrite(sokum_led, LOW);
      }
      else
      {
        sokum_aktif = true;
        bit13 = true;
        digitalWrite(sokum_led, HIGH);
      }
    }

    if (data == "15") //* Lamba
    {
      if (lamba_aktif)
        lamba_aktif = false;
      else
        lamba_aktif = true;
    }
    if (data == "3") //* Sökücü Yukarı
      bit10 = true;

    if (data == "4") //* Sökücü Aşağı
      bit9 = true;

    if (data == "17") //* Sökücü Sağa
      bit2 = true;

    if (data == "6") //* Sökücü Sola
      bit1 = true;

    if (data == "7") //* Depo Yukarı
      bit8 = true;

    if (data == "8") //* Depo Aşağı
      bit7 = true;

    if (data == "9") //* Hazırlayıcı Yukarı
      bit11 = true;

    if (data == "10") //* Hazırlayıcı Aşağı
      bit12 = true;

    if (data == "11") //* İlave Kapatma
      bit8 = true;

    if (data == "12") //* İlave açma
      bit7 = true;

    if (data == "13") //* Boşalt Çalıştır
      bit5 = true;

    if (data == "14") //* Boşalt Durdur
    {
      // todo BoşaltDurdur bırakıldığında boşaltma devreye tekrar girdiğinden bir kod daha gönderiyor
      bit6 = true; // Sistem Geri
      bit4 = true; // Bosalt DurDur Kodu
    }
    if (data == "16") //* Sistem Geri
      bit6 = true;

    flag = true; // Flag'ı Etkinleştir
  }
}

void setup()
{
  Serial.begin(1200);           // Seri Monitör
  vw_set_rx_pin(RF_module_pin); // 433mhz Modül Pin'i Belirlenmesi
  vw_setup(1000);               // Saniye/Bit
  vw_rx_start();                // Start the receiver PLL running

  //* Pin Output
  pinMode(RX_data_led, OUTPUT);  // Pin'i Çıkış Olarak Belirle
  pinMode(otomatik_led, OUTPUT); // Pin'i Çıkış Olarak Belirle
  pinMode(sokum_led, OUTPUT);    // Pin'i Çıkış Olarak Belirle

  //* watchdog
  wdt_disable();       // Watchdog'u devre dışı bırakın ve 2 saniyeden fazla bekleyin
  delay(3000);         // Yanlış yapılandırma durumunda Arduino'nun sonsuza kadar sıfırlanmaya devam etmemesi için yapıldı
  wdt_enable(WDTO_2S); // 2 saniyelik bir zaman aşımı ile bekçi köpeğini etkinleştir
}

void loop()
{
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
    digitalWrite(RX_data_led, LOW); // Verinin Bittiğini Belirten Led
  }
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
  byte binaryData_3 = 0;
  binaryData_3 |= bit16;
  binaryData_3 |= bit17 << 1;
  binaryData_3 |= bit18 << 2;
  binaryData_3 |= bit19 << 3;
  binaryData_3 |= bit20 << 4;
  binaryData_3 |= bit21 << 5;
  binaryData_3 |= bit22 << 6;
  binaryData_3 |= bit23 << 7;
  //* Mainboard'a Gönderilen Veri
  Serial.write(68);
  Serial.write(84);
  Serial.write(89);
  Serial.write(binaryData_1);
  Serial.write(binaryData_2);
  Serial.write(binaryData_3);

  wdt_reset(); // watchdog Yenile (Aksi Halde 2sn içinye resset çeker)

  delay(59); // Bekleme Süresi (Değiştirilebilir)
}