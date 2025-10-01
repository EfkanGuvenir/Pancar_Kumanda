//*---- T-DRK03 V25.0.5  ----*//

#include <Arduino.h>
#include <avr/sleep.h>
#include <avr/power.h>
#include <util/delay.h>
#include <Keypad2.h>     //matrix button library
#include <VirtualWire.h> //433RF library

// todo Bu Şifre Alıcı Ve Verici Eşleşmesi İçin Aynı Olmalıdır (100-255 Arasında değer olmalı)
uint8_t sifreleme = 100;

const byte ROWS = 4;
const byte COLS = 4;

char keys[ROWS][COLS] =
    {
        // 5   4    3    2
        {'A', 'B', 'C', 'D'}, // 9
        {'E', 'F', 'G', 'H'}, // 8
        {'I', 'J', 'K', 'L'}, // 7
        {'M', 'N', 'O', 'P'}, // 6
};

byte rowPins[ROWS] = {6, 7, 8, 9}; // connect to the row pinouts of the keypad
byte colPins[COLS] = {2, 3, 4, 5}; // connect to the column pinouts of the keypad
const int led = A5;
const int rf_data = 10; // Transmitter Veri Pin'i

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
  // Donma Olmasın Diye  for döngüsü ile delay yaptım.
  // olası bir tuş basımını algılamak için
  for (int i = 0; i < 15; i++)
  {
    if (kpd.getState() == PRESSED)
      return;
    delayMicroseconds(500);
  }
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
  /**************************************************************/
  pinMode(led, OUTPUT); // Pin Çıkış Olarak Ayarlanır
  digitalWrite(led, LOW);
  /**************************************************************/
  vw_set_tx_pin(rf_data); // Vericinin Pin'i Ayrlanıyor
  vw_setup(2000);         // bps - Vericinin hızı (Önerilen Değer Bu)
  /**************************************************************/
  // pin change interrupt
  PCMSK2 |= bit(PCINT22); // pin 6
  PCMSK2 |= bit(PCINT23); // pin 7
  PCMSK0 |= bit(PCINT0);  // pin 8
  PCMSK0 |= bit(PCINT1);  // pin 9
  /**************************************************************/

  for (int i = 0; i < 3; i++)
  {
    uint8_t hexData[] = {sifreleme, 0}; // Gönderilecek hex kodunu temsil eden bayt dizisi
    vw_send(hexData, sizeof(hexData));
    vw_wait_tx(); // Wait until the whole message is gone
    delayMicroseconds(500);
  }
  /**************************************************************/
  goToSleep();
}

// Mevcut global değişkenlerden sonra (yaklaşık 11. satır civarı) bu değişkenleri ekle
unsigned long sonGonderimZamani = 0;
const unsigned long gonderimAraligi = 50; // ms aralığında veri gönderimi
bool tusBasili = false;
char mevcutTus = 0;

void loop()
{
  char key = kpd.getKey(); // Basılı olan tuşu al
  /**************************************************************/
  if (kpd.getState() == RELEASED) // Button Bırakıldığını Algılar
  {
    digitalWrite(led, LOW);
    tusBasili = false;
    mevcutTus = 0;
    for (int i = 0; i < 3; i++) // 3 Kez Veri Gönderir
    {
      uint8_t hexData[] = {sifreleme, 0}; // Gönderilecek hex kodunu temsil eden bayt dizisi
      vw_send(hexData, sizeof(hexData));
      vw_wait_tx(); // Wait until the whole message is gone
      delayMicroseconds(500);
    }
    goToSleep();
  }
  /**************************************************************/
  if (key)
  {
    digitalWrite(led, HIGH);

    // Tuşa Bastığı ilk an 2 defa Veri Gönderir
    uint8_t hexData[] = {sifreleme, static_cast<uint8_t>(key)};
    vw_send(hexData, sizeof(hexData));
    vw_wait_tx(); // Wait until the whole message is gone
    delayMicroseconds(500);
    tusBasili = true;
    mevcutTus = key;
    sonGonderimZamani = millis(); // Tuş ilk basıldığında zamanlayıcıyı sıfırla
  }

  // Tuş basılıyken her ms'de bir donma olmadan veri gönder
  if (tusBasili && (millis() - sonGonderimZamani >= gonderimAraligi))
  {
    uint8_t hexData[] = {sifreleme, static_cast<uint8_t>(mevcutTus)};
    vw_send(hexData, sizeof(hexData));
    vw_wait_tx(); // Wait until the whole message is gone
    delayMicroseconds(500);
    sonGonderimZamani = millis(); // Son gönderim zamanını güncelle
  }

  /**************************************************************/
}
