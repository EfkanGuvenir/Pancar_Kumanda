//** V1.0 Beta  **//

#include <Arduino.h>
#include <avr/wdt.h> // Watchdog için gerekli kütüphane

//* Pin OUT /IN
const int opsiyonel_led = 2;
const int out1 = 12;
const int out2 = 3;
const int out3 = 11;
const int out4 = 4;
const int out5 = 10;
const int out6 = 5;
const int out7 = 9;
const int out8 = 6;
const int out9 = 8;
const int out10 = 7;

const int in1 = A5;
const int in2 = A4;
const int in3 = A3;
const int in4 = A2;

/**************************************************************/
//* Zamanlayıcı
unsigned long ISR1_Zaman = 10; // ms
unsigned long ISR1_evvelkiMILLIS;
/**************************************************************/

void remote_data(byte bayt_4, byte bayt_5) // Gelen Datanın Ayıklanması
{
  //* Gelen 3. Data
  byte receivedData1 = bayt_4; // Gelen veri buraya atanacak

  // Gelen veriden her bir biti alıp ayrı değişkenlere atama
  byte bit0 = receivedData1 & 0x01;
  byte bit1 = (receivedData1 >> 1) & 0x01;
  byte bit2 = (receivedData1 >> 2) & 0x01;
  byte bit3 = (receivedData1 >> 3) & 0x01;
  byte bit4 = (receivedData1 >> 4) & 0x01;
  byte bit5 = (receivedData1 >> 5) & 0x01;
  byte bit6 = (receivedData1 >> 6) & 0x01;
  byte bit7 = (receivedData1 >> 7) & 0x01;

  byte binaryData_1 = 0;
  binaryData_1 |= bit0;
  binaryData_1 |= bit1 << 1;
  binaryData_1 |= bit2 << 2;
  binaryData_1 |= bit3 << 3;
  binaryData_1 |= bit4 << 4;
  binaryData_1 |= bit5 << 5;
  binaryData_1 |= bit6 << 6;
  binaryData_1 |= bit7 << 7;
  /**************************************************************/
  //* Gelen 4. Data
  byte receivedData2 = bayt_5; // Gelen veri buraya atanacak

  // Gelen veriden her bir biti alıp ayrı değişkenlere atama
  byte bit8 = receivedData2 & 0x01;
  byte bit9 = (receivedData2 >> 1) & 0x01;
  byte bit10 = (receivedData2 >> 2) & 0x01;
  byte bit11 = (receivedData2 >> 3) & 0x01;
  byte bit12 = (receivedData2 >> 4) & 0x01;
  byte bit13 = (receivedData2 >> 5) & 0x01;
  byte bit14 = (receivedData2 >> 6) & 0x01;
  byte bit15 = (receivedData2 >> 7) & 0x01;

  byte binaryData_2 = 0;
  binaryData_2 |= bit8;
  binaryData_2 |= bit9 << 1;
  binaryData_2 |= bit10 << 2;
  binaryData_2 |= bit11 << 3;
  binaryData_2 |= bit12 << 4;
  binaryData_2 |= bit13 << 5;
  binaryData_2 |= bit14 << 6;
  binaryData_2 |= bit15 << 7;
  /**************************************************************/
  /* Örnek Bir Kod
  if (bit1 == true)
    digitalWrite(out3, HIGH);
  else
    digitalWrite(out3, LOW);
  */
  /**************************************************************/
}

void setup()
{
  /**************************************************************/
  //* Pin
  pinMode(opsiyonel_led, OUTPUT);
  pinMode(out1, OUTPUT);
  pinMode(out2, OUTPUT);
  pinMode(out3, OUTPUT);
  pinMode(out4, OUTPUT);
  pinMode(out5, OUTPUT);
  pinMode(out6, OUTPUT);
  pinMode(out7, OUTPUT);
  pinMode(out8, OUTPUT);
  pinMode(out9, OUTPUT);
  pinMode(out10, OUTPUT);

  pinMode(in1, INPUT);
  pinMode(in2, INPUT);
  pinMode(in3, INPUT);
  pinMode(in4, INPUT);
  /**************************************************************/
  //* watchdog
  delay(3000);         // Yanlış yapılandırma durumunda Arduino'nun sonsuza kadar sıfırlanmaya devam etmemesi için yapıldı
  wdt_enable(WDTO_2S); // 2 saniyelik bir zaman aşımı ile bekçi köpeğini etkinleştir
  /**************************************************************/
}

void loop()
{
  unsigned long currentMillis = millis(); // zamanlayıcıyı oku

  if (currentMillis - ISR1_evvelkiMILLIS >= ISR1_Zaman)
  {
    Serial.begin(1200);
    ISR1_evvelkiMILLIS = currentMillis;

    if (Serial.available() >= 5)
    { // 5 baytlık veri gelene kadar bekle
      digitalWrite(opsiyonel_led, HIGH);
      // İlk üç sabit baytı kontrol et
      byte byte1 = Serial.read();
      byte byte2 = Serial.read();
      byte byte3 = Serial.read();

      // İlk üç sabit bayt 68, 84 ve 89 mu diye kontrol et
      if (byte1 == 68 && byte2 == 84 && byte3 == 89)
      {
        // Eğer doğruysa, son iki değişken baytı al
        byte byte4 = Serial.read();
        byte byte5 = Serial.read();
        remote_data(byte4, byte5);
        Serial.end();
      }
    }
    digitalWrite(opsiyonel_led, LOW);
  }
  /**************************************************************/
  wdt_reset(); // watchdog Yenile (Aksi Halde 2sn içinye resset çeker)
}
