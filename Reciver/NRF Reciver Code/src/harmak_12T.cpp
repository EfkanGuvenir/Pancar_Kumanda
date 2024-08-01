//*---- NRF V1.0  ----*//
//*----	Model_3 ----*//

#include <Arduino.h>
#include <avr/wdt.h> // Watchdog için gerekli kütüphane
#include <SPI.h>     //NRF24 modülü için
#include <RF24.h>    //NRF24 modülü için

//* NRF Şifrelemesi
const byte address[6] = "00002"; // Gönderici ve alıcı arasındaki aynı adres

const int led = 10;
RF24 radio(A1, A0); // CE, CSN pins

//* ON OFF Sistem için
bool otomatik_aktif; // On-Off İçin Değişlen
bool makina_aktif;   // On-Off İçin Değişlen
bool depo_aktif;     // On-Off İçin Değişlen
bool flag;           // Tuşa basılı tuttuğu sürece saçmalamaması için

bool bit7, bit6 = true, bit5, bit4, bit3, bit2, bit1, bit0; // Mainboard'a Gidecek Birinci Veri
bool bit15, bit14, bit13, bit12, bit11, bit10, bit9, bit8;  // Mainboard'a Gidecek  İkinci Veri

//* Zamanlama
unsigned long ISR1_Zaman = 50;    // 1.Veriyi Gönderecek Süre
unsigned long ISR1_evvelkiMILLIS; // 1.Veriyi Gönderecek Süre

void key(char key_data)
{
  if (key_data == '0') //* Tuş Bırakıldığında
  {
    digitalWrite(led, LOW);
    bit0 = false, bit1 = false, bit2 = false, bit3 = false, bit4 = false, bit5 = false, bit6 = true, bit7 = false;
    bit8 = false, bit9 = false, bit10 = false, bit11 = false, bit12 = false, bit13 = false, bit14 = false, bit15 = false;
    flag = false; // Flag'ı Pasifleştir

    if (otomatik_aktif == true)
      bit0 = true;
    else
      bit0 = false;

    if (makina_aktif == true)
      bit13 = true;
    else
      bit13 = false;

    if (depo_aktif == true)
    {
      bit6 = false;
      bit5 = true;
    }
    else
      bit5 = false;
  }

  if (flag == false) // işlemler birdan fazla yapılmaması için
  {
    switch (key_data)
    {
    case 'A':

      break;
    case 'B':

      break;
    case 'C':

      break;
    case 'D':

      break;
    case 'E': // Hazırlayıcı Yukarı

      break;
    case 'F': // Depo
      depo_aktif = !depo_aktif;
      break;
    case 'G': // OK Sol

      break;
    case 'H': // Otomatik
      otomatik_aktif = !otomatik_aktif;
      break;
    case 'I': // Hazırlayıcı Aşağı

      break;
    case 'J': // Kırma Açık

      break;
    case 'K': // OK Aşağı

      break;
    case 'L': // OK Yukarı

      break;
    case 'M': // Makina Ters

      break;
    case 'N': // Kırma Kapalı

      break;
    case 'O': // Ok Sağ

      break;
    case 'P': // Makina
      makina_aktif = !makina_aktif;
      break;
    }
    flag = true; // Flag'ı Etkinleştir
  }
}

void setup()
{
  Serial.begin(1200);
  pinMode(led, OUTPUT); // Pin Çıkış Olarak Ayarlanır
                        //* NRF Starting
  radio.begin();
  radio.openReadingPipe(1, address);
  radio.setPALevel(RF24_PA_MAX);
  radio.setDataRate(RF24_250KBPS);
  radio.startListening();

  //* watchdog
  wdt_disable();       // Watchdog'u devre dışı bırakın ve 2 saniyeden fazla bekleyin
  delay(3000);         // Yanlış yapılandırma durumunda Arduino'nun sonsuza kadar sıfırlanmaya devam etmemesi için yapıldı
  wdt_enable(WDTO_2S); // 2 saniyelik bir zaman aşımı ile bekçi köpeğini etkinleştir
}

void loop()
{
  unsigned long currentMillis = millis(); // zamanlayıcıyı oku
  /**************************************************************/
  if (radio.available())
  {
    digitalWrite(led, HIGH);         // Veri Geldiğini Belirtir
    char text;                       // Gelen veriyi saklayacak değişken
    radio.read(&text, sizeof(text)); // Gelen Veri Okunur
    key(text);                       // Gelen Veri Void Bloğuna Aktarılır
  }

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
