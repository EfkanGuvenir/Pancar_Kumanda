//*---- T(kablolu)-DRK01 V1.0  ----*//
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
bool sokum_aktif = false;    // On-Off İçin Değişlen
bool depo_aktif = false;     // On-Off İçin Değişlen
/**************************************************************/
//* Değişkenler
bool bit6 = true; // Kumanda Sabit 64 Gönderiyorsa Bu onun için

bool bit7, bit5, bit4, bit3, bit2, bit1, bit0;               // Mainboard'a Gidecek Birinci Veri
bool bit15, bit14, bit13, bit12, bit11, bit10, bit9, bit8;   // Mainboard'a Gidecek  İkinci Veri
bool bit23, bit22, bit21, bit20, bit19, bit18, bit17, bit16; // Mainboard'a Gidecek  Üçüncü Veri

/**************************************************************/
//* Zamanlama
unsigned long ISR1_Zaman = 50; // ms
unsigned long ISR1_evvelkiMILLIS;
/**************************************************************/
void setup()
{
    Serial.begin(1200); // Seri Monitör
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
    unsigned long currentMillis = millis(); // zamanlayıcıyı oku
    /**************************************************************/
    char key = kpd.getKey(); // Basılan Tuşu Oku
    /**************************************************************/
    if (kpd.getState() == RELEASED) // Tuş Bırakıldığında
    {
        bit6 = true; // Kumanda Sabit 64 Gönderiyorsa Bu onun için
        bit0 = false, bit1 = false, bit2 = false, bit3 = false, bit4 = false, bit5 = false, bit7 = false;
        bit8 = false, bit9 = false, bit10 = false, bit11 = false, bit12 = false, bit13 = false, bit14 = false, bit15 = false;
        bit16 = false, bit17 = false, bit18 = false, bit19 = false, bit20 = false, bit21 = false, bit22 = false, bit23 = false;

        if (otomatik_aktif == true)
        {
            bit0 = true;
        }
        if (sokum_aktif == true)
        {
            bit13 = true;
        }
        if (depo_aktif == true)
        {
            bit6 = false;
            bit5 = true;
        }
        digitalWrite(led_2, LOW);
    }
    /**************************************************************/
    if (key != NO_KEY) // Tuşa Basıldığında
    {
        switch (key)
        {
        case 'A': //* Otomatik
            digitalWrite(led_2, HIGH);
            otomatik_aktif = true; // Otomatiği Kapat
            digitalWrite(led_1, LOW);
            digitalWrite(led_3, HIGH);
            break; // swtich den çık
        case '2':  //* Manuel
            digitalWrite(led_2, HIGH);
            otomatik_aktif = false;
            digitalWrite(led_3, LOW);
            digitalWrite(led_1, HIGH);
            break;
        case 'D': //* Söküm
            digitalWrite(led_2, HIGH);
            if (sokum_aktif)
            {
                sokum_aktif = false;
            }
            else
            {
                sokum_aktif = true;
            }
            break;
        case '0':
            bit10 = true;
            digitalWrite(led_2, HIGH);
            break;
        case '1':
            bit2 = true;
            digitalWrite(led_2, HIGH);
            break;
        case '3':
            bit4 = true;
            digitalWrite(led_2, HIGH);
            break;
        case '4':
            digitalWrite(led_2, HIGH);
            break;
        case '5':
            digitalWrite(led_2, HIGH);
            break;
        case '6':
            digitalWrite(led_2, HIGH);
            break;
        case '7':
            bit8 = true;
            digitalWrite(led_2, HIGH);
            break;
        case '8':
            bit7 = true;
            digitalWrite(led_2, HIGH);
            break;
        case '9':
            bit3 = true;
            digitalWrite(led_2, HIGH);
            break;
        case 'B':
            bit15 = true;
            digitalWrite(led_2, HIGH);
            break;
        case 'C':
            bit1 = true;
            digitalWrite(led_2, HIGH);
            break;
        case '#':
            bit9 = true;
            digitalWrite(led_2, HIGH);
            break;
        case '*':
            if (depo_aktif == false)
            {
                depo_aktif = true;
                digitalWrite(led_2, HIGH);
            }
            else
            {
                depo_aktif = false;
                digitalWrite(led_2, HIGH);
            }
            break;
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
    }
    /**************************************************************/
}
