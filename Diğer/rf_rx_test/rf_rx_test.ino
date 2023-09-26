
#include <VirtualWire.h>  //433RF library


//* Pin Out
const int RF_module_pin = 11;  // 433mhz Alıcının Bağlı Olduğu Pin (interrupt olsa iyi olur)
const int RX_data_led = 13;    // RF'den Gelen Veri Olduğunda Bildirilecek led

void setup() {

  // Seri haberleşmeyi başlat
  Serial.begin(9600);    // Arduino'nun seri monitörünü kullanmak için

  vw_set_rx_pin(RF_module_pin);  // 433mhz Modül Pin'i Belirlenmesi
  vw_setup(2000);                // Saniye/Bit
  vw_rx_start();                 // Start the receiver PLL running

  //* Pin Output
  pinMode(RX_data_led, OUTPUT);  // Pin'i Çıkış Olarak Belirle

  Serial.println("Doruk Teknik Hazırız.");
}

void loop() {
  /**************************************************************/
  // RF Gelen Data
  uint8_t buf[VW_MAX_MESSAGE_LEN];
  uint8_t buflen = VW_MAX_MESSAGE_LEN;

  if (vw_get_message(buf, &buflen))  //* RF Veri Gelirse;
  {
    digitalWrite(RX_data_led, HIGH);  // Veri Geldiğini Belirten Led
    for (int i = 0; i < 2; i++)       // Gelen Verinin 2 Satırını Al
    {
      if (i == 0)  // İlk Stır
      {
        Serial.print(String(buf[i], DEC));
      }
      if (i == 1)  // 2. Satır
      {
        Serial.print("-");
        Serial.println(String(buf[i], DEC));
      }
    }
    digitalWrite(RX_data_led, LOW);  // Verinin Bittiğini Belirten Led
  }
  /**************************************************************/
}
