//*----V1.0 (Beta_1)----*//

#include <Arduino.h>
#include <VirtualWire.h> //433RF library

//* Pin Out
const int RF_module_pin = PD2;        // 433mhz Alıcının Bağlı Olduğu Pin (interrupt olsa iyi olur)
const int otomatik_led = PD3;         // Otomatik Söküm Sisteme Girdiğinde bildirim ışığı
const int sokum_led = PD4;            // Söküm Sisteme Girdiğinde bildirim ışığı
const int RX_data_otomatik_led = PD5; // RF'den Gelen Veri Olduğunda Bildirilecek led

//* değişken
uint8_t rf_digit_key = 0; // 4 Bit Şifreleme için değişken değeri
bool yetki = false;       // Kumanda Şifrelemede Yetkilendirme Değişkeni
bool flag = false;        // Tuşa basılı tuttuğu sürece saçmalamaması için
int bit_1 = 0;            // Gönderilen Değişken Bitlerden 1.
int bit_2 = 0;            // Gönderilen Değişken Bitlerden 2.
int bit_3 = 0;            // Gönderilen Değişken Bitlerden 3.

//* ON OFF Sistem için
bool otomatik_aktif = false; // On-Off İçin Değişlen
bool sokum_aktif = false;    // On-Off İçin Değişlen
bool lamba_aktif = false;    // On-Off İçin Değişlen

//* MainBoard'a Gönderilecek DEC Kumanda Kodlar
const int otomatik_send_DEC_cod = 1;           // Otomatik Tuşunun DEC Kodu
const int sokum_send_DEC_cod = 32;             // Söküm Tuşunun DEC Kodu
const int sokucu_yukari_send_DEC_cod = 4;      // Sökücü Yukarı Tuşunun DEC Kodu
const int sokucu_asagi_send_DEC_cod = 2;       // Sökücü Aşağı Tuşunun DEC Kodu
const int sokucu_sag_send_DEC_cod = 4;         // Sökücü Sağa Tuşunun DEC Kodu
const int sokucu_sol_send_DEC_cod = 2;         // Sökücü Sola Tuşunun DEC Kodu
const int depo_yukari_send_DEC_cod = 8;        // Depo Yukarı Tuşunun DEC Kodu
const int depo_asagi_send_DEC_cod = 16;        // Depo Aşağı Tuşunun DEC Kodu
const int hazirlayici_yukari_send_DEC_cod = 8; // Hazırlayıcı Yukarı Tuşunun DEC Kodu
const int hazirlayici_asagi_send_DEC_cod = 16; // Hazırlayıcı Aşağı Tuşunun DEC Kodu
const int ilave_ac_send_DEC_cod = 1;           // İlave Aç Tuşunun DEC Kodu
const int ilave_kapat_send_DEC_cod = 128;      // İlave Kapat Tuşunun DEC Kodu
const int bosalt_calis_send_DEC_cod = 32;      // Boşaltma Çalıştır Tuşunun DEC Kodu
const int bosalt_dur_send_DEC_cod = 64;        // Boşaltma Durdur Tuşunun DEC Kodu
const int lamba_send_DEC_cod = 64;             // Lamba Tuşunun DEC Kodu
const int sistem_geri_send_DEC_cod = 128;      // Sistem Geri Tuşunun DEC Kodu

void rf_data(String data) //* Gelen Datanın Ayıklanması
{
  if (data == "0") //* Tuş Bırakıldığında
  {
    flag = false; // Flag'ı Pasifleştir
    bit_1 = 0;
    bit_2 = 0;
    bit_3 = 0;
    if (otomatik_aktif == true)
      bit_1 = bit_1 + otomatik_send_DEC_cod;
    if (sokum_aktif == true)
      bit_2 = bit_2 + sokum_send_DEC_cod;
    if (lamba_aktif == true)
      bit_2 = bit_2 + lamba_send_DEC_cod;
  }

  if ((flag == false) && (data != "0")) // işlemler birdan fazla yapılmaması için
  {
    if (data == "1") //* Otomatik
    {
      if (otomatik_aktif) // Otomatik Aktif ise
      {
        otomatik_aktif = false; // Otomatiği Kapat
        digitalWrite(otomatik_led, LOW);
      }
      else // Otomatik Aktif Değilse
      {
        otomatik_aktif = true;
        digitalWrite(otomatik_led, HIGH);
      }
    }

    if (data == "9") //* Söküm
    {
      if (sokum_aktif)
      {
        sokum_aktif = false;
        digitalWrite(sokum_led, LOW);
      }
      else
      {
        sokum_aktif = true;
        bit_2 = bit_2 + sokum_send_DEC_cod;
        digitalWrite(sokum_led, HIGH);
      }
    }

    if (data == "15") //* Lamba
    {
      if (lamba_aktif)
        lamba_aktif = false;
      else
      {
        lamba_aktif = true;
        bit_2 = bit_2 + lamba_send_DEC_cod;
      }
    }

    if (data == "5") //* Sökücü Yukarı
      bit_2 = bit_2 + sokucu_yukari_send_DEC_cod;

    if (data == "6") //* Sökücü Aşağı
      bit_2 = bit_2 + sokucu_asagi_send_DEC_cod;

    if (data == "10") //* Sökücü Sağa
      bit_1 = bit_1 + sokucu_sag_send_DEC_cod;

    if (data == "2") //* Sökücü Sola
      bit_1 = bit_1 + sokucu_sol_send_DEC_cod;

    if (data == "3") //* Depo Yukarı
      bit_1 = bit_1 + depo_yukari_send_DEC_cod;

    if (data == "11") //* Depo Aşağı
      bit_1 = bit_1 + depo_asagi_send_DEC_cod;

    if (data == "4") //* Hazırlayıcı Yukarı
      bit_2 = bit_2 + hazirlayici_yukari_send_DEC_cod;

    if (data == "16") //* Hazırlayıcı Aşağı
      bit_2 = bit_2 + hazirlayici_asagi_send_DEC_cod;

    if (data == "7") //* İlave Kapatma
      bit_1 = bit_1 + ilave_kapat_send_DEC_cod;

    if (data == "8") //* İlave açma
      bit_2 = bit_2 + ilave_ac_send_DEC_cod;

    if (data == "12") //* Boşalt Çalıştır
      bit_1 = bit_1 + bosalt_calis_send_DEC_cod;

    if (data == "13") //* Boşalt Durdur
      bit_1 = bit_1 + bosalt_dur_send_DEC_cod;

    if (data == "14") //* Sistem Geri
      bit_2 = bit_2 + sistem_geri_send_DEC_cod;

    flag = true; // Flag'ı Etkinleştir
  }
}

void setup()
{
  Serial.begin(1200);                    // Seri Monitör
  vw_set_rx_pin(RF_module_pin);          // 433mhz Modül Pin'i Belirlenmesi
  vw_setup(1000);                        // Saniye/Bit
  vw_rx_start();                         // Start the receiver PLL running
  pinMode(RX_data_otomatik_led, OUTPUT); // Pin'i Çıkış Olarak Belirle
  pinMode(otomatik_led, OUTPUT);         // Pin'i Çıkış Olarak Belirle
  pinMode(sokum_led, OUTPUT);            // Pin'i Çıkış Olarak Belirle
}

void loop()
{
  // RF Gelen Data
  uint8_t buf[VW_MAX_MESSAGE_LEN];
  uint8_t buflen = VW_MAX_MESSAGE_LEN;

  if (vw_get_message(buf, &buflen)) //* RF Veri Gelirse;
  {
    digitalWrite(RX_data_otomatik_led, HIGH); // Veri Geldiğini Belirten Led
    for (int i = 0; i < 2; i++)               // Gelen Verinin 2 Satırını Al
    {
      if (i == 0)                           // İlk Stır
      {                                     // İlk Satır Kumanda Yetkisi İçin
        String sifre = String(buf[i], DEC); // Gelen Veriyi Oku
        int sifre_int = sifre.toInt();      // string'i int çevir

        if (sifre_int == rf_digit_key) // Yetkiyi Sorgula
          yetki = true;                // Yetki Verildi
        else
          yetki = false; // Yetkisiz Kumanda
      }
      if ((i == 1) && (yetki == true)) // 2. Satır
        rf_data(String(buf[i], DEC));  // Gelen Veriyi Void'e Aktar
    }
    digitalWrite(RX_data_otomatik_led, LOW); // Verinin Bittiğini Belirten Led
  }

  //* Mainboard'a Gönderilen Veri
  Serial.write(68);
  Serial.write(84);
  Serial.write(89);
  Serial.write(bit_1);
  Serial.write(bit_2);
  Serial.write(bit_3);

  delay(59); // Bekleme Süresi (Değiştirilebilir)
}