//*---- T-DRK01 V1.3  ----*//

#include <Arduino.h>
#include <Keypad2.h> //matrix button library
#include <avr/sleep.h>
#include <avr/power.h>
#include <VirtualWire.h> //433RF library

//* Şifreleme
uint8_t kimlik_dogrulama_key = 20; // todo Bu Şifre Alıcı Ve Verici Eşleşmesi İçin Aynı Olmalıdır (20-255 Arasında değer olmalı)

const byte ROWS = 4;
const byte COLS = 4;

char keys[ROWS][COLS] =
    {
        {'1', '2', '3', 'A'},
        {'4', '5', '6', 'B'},
        {'7', '8', '9', 'C'},
        {'*', '0', '#', 'D'},
};

byte rowPins[ROWS] = {6, 7, 8, 9}; // connect to the row pinouts of the keypad
byte colPins[COLS] = {2, 3, 4, 5}; // connect to the column pinouts of the keypad

// number of items in an array
#define NUMITEMS(arg) ((unsigned int)(sizeof(arg) / sizeof(arg[0])))

// Pin Giriş/Çıkışları
const int rf_data = 10;  // Transmitter Veri Pin'i
const int rf_power = 13; // Transmitter Güç Pin'i

//* Değişkenler
uint8_t press_button;     // Buttona basılınca gönderilecek kodun değişkeni
bool button_flag = false; // buttonun basılıp bırakıldığını algılacak değişken

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
  }                                // end of for each column

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
  }   // end of for each row

  // overcome any debounce delays built into the keypad library
  delay(50);

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
  vw_set_tx_pin(rf_data);    // Vericinin Pin'i Ayrlanıyor
  vw_setup(2000);            // bps - Vericinin hızı
  pinMode(rf_power, OUTPUT); // Pin Çıkış Olarak Ayarlanır
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
    button_flag = false; // Buttona
    delayMicroseconds(50);
    for (int i = 0; i < 2; i++)
    {
      uint8_t shexData[] = {kimlik_dogrulama_key, 0}; // Gönderilecek hex kodunu temsil eden bayt dizisi
      vw_send(shexData, sizeof(shexData));
      vw_wait_tx(); // Wait until the whole message is gone
      delayMicroseconds(500);
    }
    digitalWrite(rf_power, LOW);
    goToSleep();
    return;
  }
  /**************************************************************/
  switch (key) // Basılan tuş için ayarlama yap
  {
  case '0': // sayaca 0 ise
    press_button = 16;
    break; // swtich den çık
  case '1':
    press_button = 1;
    break;
  case '2':
    press_button = 6;
    break;
  case '3':
    press_button = 7;
    break;
  case '4':
    press_button = 3;
    break;
  case '5':
    press_button = 4;
    break;
  case '6':
    press_button = 11;
    break;
  case '7':
    press_button = 2;
    break;
  case '8':
    press_button = 17;
    break;
  case '9':
    press_button = 8;
    break;
  case 'A':
    press_button = 9;
    break;
  case 'B':
    press_button = 12;
    break;
  case 'C':
    press_button = 13;
    break;
  case 'D':
    press_button = 10;
    break;
  case '*':
    press_button = 14;
    break;
  case '#':
    press_button = 15;
    break;
  }
  /**************************************************************/
  if (key)
  {
    digitalWrite(rf_power, HIGH);
    button_flag = true;
    delayMicroseconds(50);
  }
  /**************************************************************/
  if (button_flag == true) // buttona basılmışsa
  {
    uint8_t hexData[] = {kimlik_dogrulama_key, press_button}; // Gönderilecek hex kodunu temsil eden bayt dizisi
    vw_send(hexData, sizeof(hexData));
    vw_wait_tx(); // Wait until the whole message is gone
    delayMicroseconds(500);
  }
}