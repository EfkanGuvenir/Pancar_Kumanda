//*---- T-DRK02 V1.0  ----*//

#include <Arduino.h>
#include <avr/sleep.h>
#include <avr/power.h>
#include <util/delay.h>
#include <Keypad2.h> //matrix button library
#include <SPI.h>     //NRF24 modülü için
#include <RF24.h>    //NRF24 modülü için

//* NRF Şifrelemesi
const byte address[6] = "00001"; // Gönderici ve alıcı arasındaki aynı adres

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
RF24 radio(A1, A0);                // CE, CSN pins
const int led = 1;

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
  radio.powerDown(); // NRF Modülünü Uyut

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
  pinMode(led, OUTPUT); // Pin Çıkış Olarak Ayarlanır

  radio.begin();
  radio.openWritingPipe(address);
  radio.setPALevel(RF24_PA_MAX);   // Güç Seviyesi Maksimum'a Ayarlı
  radio.setDataRate(RF24_250KBPS); // Veri Hızı Ayarı
  radio.stopListening();
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
    digitalWrite(led, LOW);
    radio.powerUp(); // NRF Modülünü Uyandır
    _delay_ms(2);    // NRF Modülünün Açılması için gereken Süre
    const char data[] = "0";
    radio.write(&data, sizeof(data));
    _delay_ms(2);
    goToSleep();
  }
  /**************************************************************/
  if (key)
  {
    digitalWrite(led, HIGH);
    radio.powerUp(); // NRF Modülünü Uyandır
    _delay_ms(2);    // NRF Modülünün Açılması için gereken Süre
    bool success = radio.write(&key, sizeof(key));
    if (success)
    {
      radio.powerDown(); // NRF Modülünü Uyut
    }
  }
}