//*---- T(kablolu) V1.0  ----*//
//*---- Kumanda Tuş Takımını Test Etmek İçin Kullanılır----*//
/**************************************************************/
#include <Arduino.h>
#include <Keypad2.h> // Matrix button library
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
void setup()
{
    Serial.begin(1200); // Seri Monitör
    /**************************************************************/
    pinMode(led_1, OUTPUT); // Pin'i Çıkış Olarak Belirle
    pinMode(led_2, OUTPUT); // Valf İçin Kullanılan Led
    pinMode(led_3, OUTPUT); // Pin'i Çıkış Olarak Belirle

    digitalWrite(led_1, HIGH);
    delay(2000);
    digitalWrite(led_1, LOW);
    digitalWrite(led_2, HIGH);
    delay(2000);
    digitalWrite(led_2, LOW);
    digitalWrite(led_3, HIGH);
    delay(2000);
    digitalWrite(led_3, LOW);
    /**************************************************************/
}

void loop()
{
    /**************************************************************/
    char key = kpd.getKey(); // Basılan Tuşu Oku
    /**************************************************************/
    if (key != NO_KEY) // Tuşa Basıldığında
    {
        switch (key)
        {
        case 'A': //* Otomatik
            Serial.println("A");
            break; // swtich den çık
        case '2':  //* Manuel
            Serial.println("2");
            break;
        case 'D': //* Söküm
            Serial.println("D");
            break;
        case '0':
            Serial.println("0");
            break;
        case '1':
            Serial.println("1");
            break;
        case '3':
            Serial.println("3");
            break;
        case '4':
            Serial.println("4");
            break;
        case '5':
            Serial.println("5");
            break;
        case '6':
            Serial.println("6");
            break;
        case '7':
            Serial.println("7");
            break;
        case '8':
            Serial.println("8");
            break;
        case '9':
            Serial.println("9");
            break;
        case 'B':
            Serial.println("B");
            break;
        case 'C':
            Serial.println("C");
            break;
        case '#':
            Serial.println("#");
            break;
        case '*':
            Serial.println("*");
            break;
        }
    }
    /**************************************************************/
}
