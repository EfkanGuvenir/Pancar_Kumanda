#include <Arduino.h>
#include <VirtualWire.h> //433RF library

//* Pin Out
const int RF_module_pin = 2; // 433mhz Alıcının Bağlı Olduğu Pin (interrupt olsa iyi olur)

void setup()
{
  Serial.begin(9600);           // Seri Monitör
  vw_set_rx_pin(RF_module_pin); // 433mhz Modül Pin'i Belirlenmesi
  vw_setup(1000);               // Saniye/Bit
  vw_rx_start();                // Start the receiver PLL running
}

void loop()
{
  // RF Gelen Data
  uint8_t buf[VW_MAX_MESSAGE_LEN];
  uint8_t buflen = VW_MAX_MESSAGE_LEN;

  if (vw_get_message(buf, &buflen)) // Non-blocking
  {
    int i;
    for (i = 0; i < buflen; i++)
    {
      Serial.print(buf[i], DEC);
    }
    Serial.println();
  }
}