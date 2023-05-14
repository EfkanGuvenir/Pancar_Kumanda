
// Wake from deep sleep with a keypress demonstration

// Author: Nick Gammon
// Date: 18th November 2012

#include <Keypad2.h>
#include <avr/sleep.h>
#include <avr/power.h>

const byte ROWS = 4;
const byte COLS = 4; 

char keys[ROWS][COLS] = 
  {
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'},
  };

byte rowPins[ROWS] = {6, 7, 8, 9}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {2, 3, 4, 5}; //connect to the column pinouts of the keypad
  
// number of items in an array
#define NUMITEMS(arg) ((unsigned int) (sizeof (arg) / sizeof (arg [0])))

const byte ledPin = 13;

  // Create the Keypad
Keypad kpd = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );

/*

Pin change interrupts.

Pin                  Mask / Flag / Enable

D0	  PCINT16 (PCMSK2 / PCIF2 / PCIE2)
D1	  PCINT17 (PCMSK2 / PCIF2 / PCIE2)
D2	  PCINT18 (PCMSK2 / PCIF2 / PCIE2)
D3	  PCINT19 (PCMSK2 / PCIF2 / PCIE2)
D4	  PCINT20 (PCMSK2 / PCIF2 / PCIE2)
D5	  PCINT21 (PCMSK2 / PCIF2 / PCIE2)
D6	  PCINT22 (PCMSK2 / PCIF2 / PCIE2)
D7	  PCINT23 (PCMSK2 / PCIF2 / PCIE2)
D8	  PCINT0 (PCMSK0 / PCIF0 / PCIE0)
D9	  PCINT1 (PCMSK0 / PCIF0 / PCIE0)
D10	  PCINT2 (PCMSK0 / PCIF0 / PCIE0)
D11	  PCINT3 (PCMSK0 / PCIF0 / PCIE0)
D12	  PCINT4 (PCMSK0 / PCIF0 / PCIE0)
D13	  PCINT5 (PCMSK0 / PCIF0 / PCIE0)
A0	  PCINT8 (PCMSK1 / PCIF1 / PCIE1)
A1	  PCINT9 (PCMSK1 / PCIF1 / PCIE1)
A2	  PCINT10 (PCMSK1 / PCIF1 / PCIE1)
A3	  PCINT11 (PCMSK1 / PCIF1 / PCIE1)
A4	  PCINT12 (PCMSK1 / PCIF1 / PCIE1)
A5	  PCINT13 (PCMSK1 / PCIF1 / PCIE1)

*/

// turn off interrupts until we are ready
ISR (PCINT0_vect)
  {
  PCICR = 0;  // cancel pin change interrupts
  } // end of ISR (PCINT0_vect)

ISR (PCINT1_vect)
  {
  PCICR = 0;  // cancel pin change interrupts
  } // end of ISR (PCINT1_vect)

ISR (PCINT2_vect)
  {
  PCICR = 0;  // cancel pin change interrupts
  } // end of ISR (PCINT2_vect)

void setup ()
  {
  pinMode (ledPin, OUTPUT);
  
  // pin change interrupt masks (see above list)
  PCMSK2 |= bit (PCINT22);   // pin 6
  PCMSK2 |= bit (PCINT23);   // pin 7
  PCMSK0 |= bit (PCINT0);    // pin 8
  PCMSK0 |= bit (PCINT1);    // pin 9

  }  // end of setup

// set pins as keypad library expects them
// or call: kpd.initializePins ();
//    however in the library I have that is a private method

void reconfigurePins ()
  {
  byte i;
  
  // go back to all pins as per the keypad library
  
  for (i = 0; i < NUMITEMS (colPins); i++)
    {
    pinMode (colPins [i], OUTPUT);
    digitalWrite (colPins [i], HIGH); 
    }  // end of for each column 

  for (i = 0; i < NUMITEMS (rowPins); i++)
    {
    pinMode (rowPins [i], INPUT_PULLUP);
    }   // end of for each row

  }  // end of reconfigurePins

void goToSleep ()
  {
  byte i;
   
  // set up to detect a keypress
  for (i = 0; i < NUMITEMS (colPins); i++)
    {
    pinMode (colPins [i], OUTPUT);
    digitalWrite (colPins [i], LOW);   // columns low
    }  // end of for each column

  for (i = 0; i < NUMITEMS (rowPins); i++)
    {
    pinMode (rowPins [i], INPUT_PULLUP);
    }  // end of for each row
    
   // now check no pins pressed (otherwise we wake on a key release)
   for (i = 0; i < NUMITEMS (rowPins); i++)
    {
    if (digitalRead (rowPins [i]) == LOW)
       {
       reconfigurePins ();
       return; 
       } // end of a pin pressed
    }  // end of for each row
  
  // overcome any debounce delays built into the keypad library
  delay (50);
  
  // at this point, pressing a key should connect the high in the row to the 
  // to the low in the column and trigger a pin change
  
  set_sleep_mode (SLEEP_MODE_PWR_DOWN);  
  sleep_enable();

  byte old_ADCSRA = ADCSRA;
  // disable ADC to save power
  ADCSRA = 0;  

  power_all_disable ();  // turn off various modules
   
  PCIFR  |= bit (PCIF0) | bit (PCIF1) | bit (PCIF2);   // clear any outstanding interrupts
  PCICR  |= bit (PCIE0) | bit (PCIE1) | bit (PCIE2);   // enable pin change interrupts
   
  // turn off brown-out enable in software
  MCUCR = bit (BODS) | bit (BODSE);
  MCUCR = bit (BODS); 
  sleep_cpu ();  
 
  // cancel sleep as a precaution
  sleep_disable();
  power_all_enable ();   // enable modules again
  ADCSRA = old_ADCSRA;   // re-enable ADC conversion
  
  // put keypad pins back how they are expected to be
  reconfigurePins ();
    
  }  // end of goToSleep
  
void loop () 
  {
   
   byte key =  kpd.getKey();
   if (!key)
     {
     // no key pressed? go to sleep
     goToSleep ();
     return;
     }

  // confirmation we woke - flash LED number of times
  // for the appropriate pin (eg. pin 1: one time) 
  for (byte i = 0; i < (key - '0'); i++)
    {
    digitalWrite (ledPin, HIGH);
    delay (500); 
    digitalWrite (ledPin, LOW);
    delay (500); 
    } // end of for loop
    
  } // end of loop
