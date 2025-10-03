//* Bu kod, farklı tuş takımları tarafından gönderilen kodları okumak için yazılmıştır.

/**************************************************************/
#include <Arduino.h>     //Arduino Library
#include <VirtualWire.h> //433RF library
/**************************************************************/
//* Pin Out
const int RF_module_pin = 2; // 433mhz Alıcının Bağlı Olduğu Pin (interrupt olsa iyi olur)
const int RX_data_led = 10;  // RF'den Gelen Veri Olduğunda Bildirilecek led
/**************************************************************/

void setup()
{
  Serial.begin(1200);           // Seri Monitör
  vw_set_rx_pin(RF_module_pin); // 433mhz Modül Pin'i Belirlenmesi
  vw_setup(1200);               // Saniye/Bit
  vw_rx_start();                // Start the receiver PLL running

  //* Pin Output
  pinMode(RX_data_led, OUTPUT);   // Pin'i Çıkış Olarak Belirle
  digitalWrite(RX_data_led, LOW); // Veri Geldiğini Belirten Led
}

void loop()
{
  // RF Gelen Data
  uint8_t buf[VW_MAX_MESSAGE_LEN];
  uint8_t buflen = VW_MAX_MESSAGE_LEN;

  if (vw_get_message(buf, &buflen)) // Non-blocking
  {
    int i;
    digitalWrite(RX_data_led, HIGH); // Veri Geldiğini Belirten Led
    for (i = 0; i < buflen; i++)
    {
      Serial.write(buf[i]);
    }
    digitalWrite(RX_data_led, LOW); // Verinin Bittiğini Belirten Led
  }
}