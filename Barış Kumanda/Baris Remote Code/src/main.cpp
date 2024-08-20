//*---- BRS-DRK01 V1.0  ----*//

#include <Arduino.h>
#include <avr/sleep.h>
#include <avr/power.h>
#include <util/delay.h>
#include <Keypad2.h> //matrix button library

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
const int rf_power = A2;
const int led = A5;
char key_degisken = 0;

//* Değişkenler
uint8_t press_button;     // Buttona basılınca gönderilecek kodun değişkeni
bool button_flag = false; // buttonun basılıp bırakıldığını algılacak değişken

// number of items in an array
#define NUMITEMS(arg) ((unsigned int)(sizeof(arg) / sizeof(arg[0])))

// Tuş Takımı Oluşturuluyor
Keypad kpd = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

ISR(PCINT0_vect) // interrupts iptal et
{
  PCICR = 0; // cancel pin change interrupts
} // end of ISR (PCINT0_vect)

ISR(PCINT1_vect) // interrupts iptal et
{
  PCICR = 0; // cancel pin change interrupts
} // end of ISR (PCINT1_vect)

ISR(PCINT2_vect) // interrupts iptal et
{
  PCICR = 0; // cancel pin change interrupts
} // end of ISR (PCINT2_vect)

void reconfigurePins()
{
  byte i;

  // go back to all pins as per the keypad library

  for (i = 0; i < NUMITEMS(colPins); i++)
  {
    pinMode(colPins[i], OUTPUT);
    digitalWrite(colPins[i], HIGH);
  } // end of for each column

  for (i = 0; i < NUMITEMS(rowPins); i++)
  {
    pinMode(rowPins[i], INPUT_PULLUP);
  } // end of for each row

} // end of reconfigurePins

void goToSleep()
{
  byte i;

  // set up to detect a keypress
  for (i = 0; i < NUMITEMS(colPins); i++)
  {
    pinMode(colPins[i], OUTPUT);
    digitalWrite(colPins[i], LOW); // columns low
  } // end of for each column

  for (i = 0; i < NUMITEMS(rowPins); i++)
  {
    pinMode(rowPins[i], INPUT_PULLUP);
  } // end of for each row

  // now check no pins pressed (otherwise we wake on a key release)
  for (i = 0; i < NUMITEMS(rowPins); i++)
  {
    if (digitalRead(rowPins[i]) == LOW)
    {
      reconfigurePins();
      return;
    } // end of a pin pressed
  } // end of for each row

  // overcome any debounce delays built into the keypad library
  _delay_ms(50);

  // at this point, pressing a key should connect the high in the row to the
  // to the low in the column and trigger a pin change

  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  sleep_enable();

  byte old_ADCSRA = ADCSRA;
  // disable ADC to save power
  ADCSRA = 0;

  power_all_disable(); // turn off various modules

  PCIFR |= bit(PCIF0) | bit(PCIF1) | bit(PCIF2); // clear any outstanding interrupts
  PCICR |= bit(PCIE0) | bit(PCIE1) | bit(PCIE2); // enable pin change interrupts

  // turn off brown-out enable in software
  MCUCR = bit(BODS) | bit(BODSE);
  MCUCR = bit(BODS);
  sleep_cpu();

  // cancel sleep as a precaution
  sleep_disable();
  power_all_enable();  // enable modules again
  ADCSRA = old_ADCSRA; // re-enable ADC conversion

  // put keypad pins back how they are expected to be
  reconfigurePins();

} // end of goToSleep

void setup()
{
  pinMode(led, OUTPUT);      // Pin'i Çıkış Olarak Belirle
  pinMode(rf_power, OUTPUT); // Pin'i Çıkış Olarak Belirle
  digitalWrite(led, LOW);
  digitalWrite(rf_power, LOW);

  /**************************************************************/
  // pin change interrupt
  PCMSK2 |= bit(PCINT22); // pin 6
  PCMSK2 |= bit(PCINT23); // pin 7
  PCMSK0 |= bit(PCINT0);  // pin 8
  PCMSK0 |= bit(PCINT1);  // pin 9

  goToSleep();
}

void loop()
{
  char key = kpd.getKey(); // Basılı olan tuşu al
  /**************************************************************/
  if (kpd.getState() == RELEASED) // Button Bırakıldığını Algılar
  {
    digitalWrite(rf_power, HIGH);
    Serial.begin(1200);
    for (int i = 0; i < 2; i++)
    {
      // todo tuş Bırakıldığında Giden Kod
      Serial.write(245);
      Serial.write(166);
      Serial.write(170);
      Serial.write(106);
      Serial.write(153);

      Serial.flush(); // seri port üzerinden gönderilen verilerin tamamlanmasını bekler.
    }
    if (Serial)
    {
      Serial.end(); // Seri Haberleşmeyi Sonlandırır
    }
    button_flag = false;
    key_degisken = 0;
    digitalWrite(rf_power, LOW);
    digitalWrite(led, LOW);
    goToSleep();
    return;
  }

  if (key) // Buttona ilk Basildiğinda bir defa çalışır
  {
    key_degisken = key;
    button_flag = true;
    digitalWrite(rf_power, HIGH);
    digitalWrite(led, HIGH);
    delayMicroseconds(50);
  }

  if (button_flag == true) // buttona basılı tutulmuşsa
  {
    Serial.begin(1200);
    switch (key_degisken) // Basılan tuş için ayarlama yap
    {
    case 'E': // Hazırlayıcı Yukarı
      Serial.write(245);
      Serial.write(166);
      Serial.write(89);
      Serial.write(85);
      Serial.write(85);
      Serial.write(85);
      Serial.write(89);
      Serial.write(153);
      break;
    case 'F': // Depo Kaldır
      Serial.write(245);
      Serial.write(166);
      Serial.write(85);
      Serial.write(85);
      Serial.write(89);
      Serial.write(85);
      Serial.write(89);
      Serial.write(153);
      break;
    case 'G': // OK Sol
      Serial.write(245);
      Serial.write(166);
      Serial.write(85);
      Serial.write(85);
      Serial.write(85);
      Serial.write(86);
      Serial.write(86);
      Serial.write(153);
      break;
    case 'H': // Boşalt
      Serial.write(245);
      Serial.write(166);
      Serial.write(85);
      Serial.write(101);
      Serial.write(85);
      Serial.write(85);
      Serial.write(101);
      Serial.write(153);
      break;
    case 'I': // Hazırlayıcı Aşağı
      Serial.write(245);
      Serial.write(166);
      Serial.write(101);
      Serial.write(85);
      Serial.write(85);
      Serial.write(85);
      Serial.write(101);
      Serial.write(153);
      break;
    case 'J': // Kapak Aç
      Serial.write(245);
      Serial.write(166);
      Serial.write(85);
      Serial.write(86);
      Serial.write(85);
      Serial.write(85);
      Serial.write(86);
      Serial.write(153);
      break;
    case 'K': // OK Kaldır
      Serial.write(245);
      Serial.write(166);
      Serial.write(85);
      Serial.write(85);
      Serial.write(85);
      Serial.write(101);
      Serial.write(101);
      Serial.write(153);
      break;
    case 'L': //  Otomatik
      Serial.write(245);
      Serial.write(166);
      Serial.write(86);
      Serial.write(85);
      Serial.write(85);
      Serial.write(85);
      Serial.write(86);
      Serial.write(153);
      break;
    case 'M': // Depo İndir
      Serial.write(245);
      Serial.write(166);
      Serial.write(85);
      Serial.write(85);
      Serial.write(101);
      Serial.write(85);
      Serial.write(101);
      Serial.write(153);
      break;
    case 'N': // Kapak Kapat
      Serial.write(245);
      Serial.write(166);
      Serial.write(85);
      Serial.write(89);
      Serial.write(85);
      Serial.write(85);
      Serial.write(89);
      Serial.write(153);
      break;
    case 'O': // OK Sağ
      Serial.write(245);
      Serial.write(166);
      Serial.write(85);
      Serial.write(85);
      Serial.write(85);
      Serial.write(89);
      Serial.write(89);
      Serial.write(153);
      break;
    case 'P': // Yedek
      Serial.write(0);
      break;
    }
    if (Serial)
      Serial.flush(); // seri port üzerinden gönderilen verilerin tamamlanmasını bekler.
    delay(10);
  }
  /**************************************************************/
}