//*---- T(kablolu) Kumanda MCU Atmega8 V1.0----*//
//*---- Model_3 Kumandalara Uygundur----*//
//*---- Harmak Makinalara Uygundur ----*//

/**************************************************************/
#include <Arduino.h>
#include <Keypad2.h> // Matrix button library
#include <avr/wdt.h> // Watchdog için gerekli kütüphane
/**************************************************************/
//* Tuş Takımı Oluşturuluyor
const byte ROWS = 4;
const byte COLS = 4;

char keys[ROWS][COLS] =
    {
        // 2   3    4    5
        {'A', 'B', 'C', 'D'}, // 6
        {'E', 'F', 'G', 'H'}, // 7
        {'I', 'J', 'K', 'L'}, // 8
        {'M', 'N', 'O', 'P'}, // 9
};

byte rowPins[ROWS] = {6, 7, 8, 9}; // connect to the row pinouts of the keypad
byte colPins[COLS] = {2, 3, 4, 5}; // connect to the column pinouts of the keypad

Keypad kpd = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);
/**************************************************************/
//* Pin Out
const int led_1 = A4; // RF'den Gelen Veri Olduğunda Bildirilecek led
const int led_2 = A5; // Söküm Sisteme Girdiğinde bildirim  ışığı
const int led_3 = A3; // Otomatik Söküm Sisteme Girdiğinde bildirim ışığı
/**************************************************************/
//* ON OFF Sistem için
bool otomatik_aktif = false; // On-Off İçin Değişlen
bool makina_aktif = false;   // On-Off İçin Değişlen
bool depo_aktif = false;     // On-Off İçin Değişlen
/**************************************************************/
//* Değişkenler
bool button_flag; // Button yanlış basılmaların önüne geçer

bool bit7, bit6 = true, bit5, bit4, bit3, bit2, bit1, bit0; // Mainboard'a Gidecek Birinci Veri
bool bit15, bit14, bit13, bit12, bit11, bit10, bit9, bit8;  // Mainboard'a Gidecek  İkinci Veri
/**************************************************************/
//* Zamanlama
unsigned long ISR1_Zaman = 50;    // Veriyi Gönderecek Süre
unsigned long ISR1_evvelkiMILLIS; // Veriyi Gönderecek Süre

unsigned long lastDebounceTime;    // On Off Tuşlar için Yanlış Basımın Önüne Geçmek için
unsigned long debounceDelay = 100; // On Off Tuşlar için Yanlış Basımın Önüne Geçmek için
/**************************************************************/
void setup()
{
  Serial.begin(1200); // Seri Monitör
  wdt_disable();      // Watchdog'u devre dışı bırakın ve 2 saniyeden fazla bekleyin
  /**************************************************************/
  pinMode(led_1, OUTPUT); // Pin'i Çıkış Olarak Belirle
  pinMode(led_2, OUTPUT); // Valf İçin Kullanılan Led
  pinMode(led_3, OUTPUT); // Pin'i Çıkış Olarak Belirle

  digitalWrite(led_1, HIGH);
  delay(1000);
  digitalWrite(led_2, HIGH);
  delay(1000);
  digitalWrite(led_3, HIGH);
  delay(1000);
  digitalWrite(led_1, LOW);
  digitalWrite(led_2, LOW);
  digitalWrite(led_3, LOW);
  /**************************************************************/
  //* watchdog
  wdt_enable(WDTO_2S); // 2 saniyelik bir zaman aşımı ile bekçi köpeğini etkinleştir
                       /**************************************************************/
}

void loop()
{
  /**************************************************************/
  // todo ON OFF Tuş Yanlış Basılmalarına karşı bir önlem
  if (((millis() - lastDebounceTime) > debounceDelay) && (button_flag == true))
  {
    button_flag = false;
  }
  /**************************************************************/
  unsigned long currentMillis = millis(); // zamanlayıcıyı oku
  /**************************************************************/
  char key = kpd.getKey(); // Basılan Tuşu Oku
  /**************************************************************/
  if (kpd.getState() == RELEASED) // Tuş Bırakıldığında
  {
    digitalWrite(led_2, LOW);
    bit1 = false, bit2 = false, bit3 = false, bit4 = false, bit7 = false;
    bit8 = false, bit9 = false, bit10 = false, bit11 = false, bit12 = false, bit14 = false, bit15 = false;
  }
  /**************************************************************/
  if (key != NO_KEY) // Tuşa Basıldığında
  {
    digitalWrite(led_2, HIGH);
    switch (key)
    {
    case 'A': // OK Sağ
      bit2 = true;
      break;
    case 'B': // Makina ON/OFF
      if (button_flag == false)
      {
        button_flag = true;
        makina_aktif = !makina_aktif;
        delay(50);
        if (makina_aktif == true)
        {
          digitalWrite(led_3, HIGH);
          bit13 = true;
        }
        if (makina_aktif == false)
        {
          digitalWrite(led_3, LOW);
          bit13 = false;
        }
      }
      break;
    case 'C': // OK Yukarı
      bit4 = true;
      break;
    case 'D': // Otomatik ON/OFF
      if (button_flag == false)
      {
        button_flag = true;
        otomatik_aktif = !otomatik_aktif;
        delay(50);
        if (otomatik_aktif == true)
        {
          digitalWrite(led_1, HIGH);
          bit0 = true;
        }
        else
        {
          digitalWrite(led_1, LOW);
          bit0 = false;
        }
      }
      break;
    case 'E': // Kırma Açık
      bit8 = true;
      break;
    case 'F': // Kırma Kapalı
      bit7 = true;
      break;
    case 'H': // Makina Ters
      bit15 = true;
      break;
    case 'K': // OK Aşağı
      bit3 = true;
      break;
    case 'L': // Ok Sol
      bit1 = true;
      break;
    case 'M': // Hazırlayıcı Aşağı
      bit10 = true;
      break;
    case 'N': // Hazırlayıcı yukarı
      bit9 = true;
      break;
    case 'P': // Depo ON/OFF
      if (button_flag == false)
      {
        button_flag = true;
        depo_aktif = !depo_aktif;
        delay(50);
        if (depo_aktif == true)
        {
          bit6 = false;
          bit5 = true;
        }
        else
        {
          bit6 = true;
          bit5 = false;
        }
        break;
      }
    }
  }

  /**************************************************************/
  // todo Mainboard'a Gönderilecek Veriler
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

    wdt_reset(); // watchdog Yenile (Aksi Halde 2sn içinye resset çeker)
  }
  /**************************************************************/
}