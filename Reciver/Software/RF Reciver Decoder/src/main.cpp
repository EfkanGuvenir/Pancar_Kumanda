#include <Arduino.h>
#include <VirtualWire.h>

const int led_pin = 13;
const int receive_pin = 2;

void setup()
{
  delay(1000);
  Serial.begin(9600); // Debugging only
  Serial.println("setup");

  // Initialise the IO and ISR
  vw_set_rx_pin(receive_pin);
  vw_set_ptt_inverted(true); // Required for DR3100
  vw_setup(1000);            // Bits per sec
  vw_rx_start();             // Start the receiver PLL running
  pinMode(led_pin, OUTPUT);
}

void loop()
{
  uint8_t buf[VW_MAX_MESSAGE_LEN];
  uint8_t buflen = VW_MAX_MESSAGE_LEN;
  if (vw_get_message(buf, &buflen))
  { // RF Veri Gelirse
    digitalWrite(led_pin, HIGH);
    int i;
    for (i = 0; i < buflen; i++)
    {
      Serial.print(buf[i], DEC);
      Serial.print(' ');
    }
    Serial.println();
    digitalWrite(led_pin, LOW);
  }
}