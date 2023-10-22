//*---- T(kablolu) Kumanda MCU V1.2----*//
//*---- Model_2 Kumandalara Uygundur----*//
//*---- ÖzenİŞ ----*//

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
        {'1', '2', '3', 'A'},
        {'4', '5', '6', 'B'},
        {'7', '8', '9', 'C'},
        {'*', '0', '#', 'D'},
};

byte rowPins[ROWS] = {15, 16, 17, 18}; // connect to the row pinouts of the keypad
byte colPins[COLS] = {11, 12, 13, 14}; // connect to the column pinouts of the keypad

Keypad kpd = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);
/**************************************************************/
//* Pin Out
const int led_1 = 22; // RF'den Gelen Veri Olduğunda Bildirilecek led
const int led_2 = 20; // Söküm Sisteme Girdiğinde bildirim  ışığı
const int led_3 = 21; // Otomatik Söküm Sisteme Girdiğinde bildirim ışığı
/**************************************************************/
//* ON OFF Sistem için
bool otomatik_aktif = false; // On-Off İçin Değişlen
bool bosalt_aktif = false;   // On-Off İçin Değişlen
/**************************************************************/
//* Değişkenler
bool button_flag; // Button yanlış basılmaların önüne geçer

bool bit7, bit6, bit5, bit4, bit3, bit2, bit1, bit0 = true; // Mainboard'a Gidecek Birinci Veri
bool bit15, bit14, bit13, bit12, bit11, bit10, bit9, bit8;  // Mainboard'a Gidecek  İkinci Veri
/**************************************************************/
//* Zamanlama
unsigned long ISR1_Zaman = 50;    // Veriyi Gönderecek Süre
unsigned long ISR1_evvelkiMILLIS; // Veriyi Gönderecek Süre
bool data_send_flag;              // İki Ayrı data göndereceği süre için

unsigned long lastDebounceTime;    // On Off Tuşlar için Yanlış Basımın Önüne Geçmek için
unsigned long debounceDelay = 100; // On Off Tuşlar için Yanlış Basımın Önüne Geçmek için
/**************************************************************/
void setup()
{
    Serial.begin(2400); // Seri Monitör
    /**************************************************************/
    pinMode(led_1, OUTPUT); // Pin'i Çıkış Olarak Belirle
    pinMode(led_2, OUTPUT); // Valf İçin Kullanılan Led
    pinMode(led_3, OUTPUT); // Pin'i Çıkış Olarak Belirle

    digitalWrite(led_1, HIGH);
    digitalWrite(led_2, LOW);
    digitalWrite(led_3, LOW);
    /**************************************************************/
    //* watchdog
    wdt_disable();       // Watchdog'u devre dışı bırakın ve 2 saniyeden fazla bekleyin
    delay(3000);         // Yanlış yapılandırma durumunda Arduino'nun sonsuza kadar sıfırlanmaya devam etmemesi için yapıldı
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

        bit0 = true, bit1 = false, bit2 = false, bit3 = false, bit4 = false, bit5 = false, bit6 = false, bit7 = false;
        bit8 = false, bit9 = false, bit10 = false, bit11 = false, bit12 = false, bit13 = false, bit14 = false, bit15 = false;

        if (otomatik_aktif == true)
            bit9 = true;
        if (bosalt_aktif == true)
        {
            bit6 = true;
            bit14 = true;
        }
        digitalWrite(led_2, LOW);
    }
    /**************************************************************/
    if (key != NO_KEY) // Tuşa Basıldığında
    {
        switch (key)
        {
        case 'A': //* Otomatik On
            digitalWrite(led_2, HIGH);
            otomatik_aktif = true;
            bit9 = true;
            digitalWrite(led_1, LOW);
            digitalWrite(led_3, HIGH);
            break; // swtich den çık

        case '2': //* Manuel
            digitalWrite(led_2, HIGH);
            otomatik_aktif = false;
            bit9 = false;
            digitalWrite(led_1, HIGH);
            digitalWrite(led_3, LOW);
            break; // swtich den çık

        case 'D': //* söküm On/Off
            digitalWrite(led_2, HIGH);
            break;

        case '6': //* Boşalt Çalıştır
            bosalt_aktif = true;
            bit6 = true;
            bit14 = true;
            digitalWrite(led_2, HIGH);
            break;

        case '5': //* Boşalt Durdur
            bosalt_aktif = false;
            bit7 = true;
            bit15 = true;
            digitalWrite(led_2, HIGH);
            break;

        case '9': //*Söküm Aşağı
            bit3 = true;
            digitalWrite(led_2, HIGH);
            break;

        case '3': //*Söküm Yukarı
            bit4 = true;
            digitalWrite(led_2, HIGH);
            break;

        case '1': //* OK Sağa
            bit11 = true;
            digitalWrite(led_2, HIGH);
            break;

        case 'C': //* OK Sola
            bit10 = true;
            digitalWrite(led_2, HIGH);
            break;

        case 'B': //* Makina Ters
            bit5 = true;
            digitalWrite(led_2, HIGH);
            break;

        case '0': //* Hazırlayıcı Aşağı
            bit13 = true;
            digitalWrite(led_2, HIGH);
            break;

        case '#': //* Hazırlayıcı Yukarı
            bit12 = true;
            digitalWrite(led_2, HIGH);
            break;

        case '8': //* Depo Kapatma
            bit1 = true;
            digitalWrite(led_2, HIGH);
            break;

        case '7': //* Depo Açma
            bit2 = true;
            digitalWrite(led_2, HIGH);
            break;
        case '*': //* Lamba on/off
            digitalWrite(led_2, HIGH);
            break;
        case '4': //* Flaş
            bit6 = true;
            digitalWrite(led_2, HIGH);
            break;
        }
        lastDebounceTime = millis(); // Tuşa Basım Zamanlayıcısını Sıfırla
    }

    /**************************************************************/
    // todo Mainboard'a Gönderilecek Veriler
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
    wdt_reset(); // watchdog Yenile (Aksi Halde 2sn içinye resset çeker)
    /**************************************************************/
}
