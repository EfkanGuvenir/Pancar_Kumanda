#include <Arduino.h>
#include "BluetoothSerial.h"
#include <HardwareSerial.h>

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

BluetoothSerial SerialBT;

/**************************************************************/
//* Değişkenler
bool bit7 = false, bit6 = false, bit5 = false, bit4 = false, bit3 = false, bit2 = false, bit1 = false, bit0 = false;       // Mainboard'a Gidecek Birinci Veri
bool bit15 = false, bit14 = false, bit13 = false, bit12 = false, bit11 = false, bit10 = false, bit9 = false, bit8 = false; // Mainboard'a Gidecek  İkinci Veri

byte binaryData_1 = 0;
byte binaryData_2 = 0;

/**************************************************************/
void setup()
{
  delay(5000);
  Serial.begin(9600);
  Serial2.begin(1200);            // 2. UART başlatılıyor
  SerialBT.begin("Doruk_Teknik"); // Bluetooth device name
}

void loop()
{
  unsigned long currentMillis = millis(); // zamanlayıcıyı oku
  /**************************************************************/
  if (Serial.available())
  {
    SerialBT.write(Serial.read());
  }
  /**************************************************************/
  if (SerialBT.available())
  { // Bluetooth'dan Gelen Veriyi Oku
    switch (SerialBT.parseInt())
    {
    case 1:
      bit0 = true;
      SerialBT.println("bit0 = true - Byte1=1");
      break;
    case 2:
      bit1 = true;
      SerialBT.println("bit1 = true - Byte1=2");
      break;
    case 3:
      bit2 = true;
      SerialBT.println("bit2 = true - Byte1=4");
      break;
    case 4:
      bit3 = true;
      SerialBT.println("bit3 = true - Byte1=8");
      break;
    case 5:
      bit4 = true;
      SerialBT.println("bit4 = true - Byte1=16");
      break;
    case 6:
      bit5 = true;
      SerialBT.println("bit5 = true - Byte1=32");
      break;
    case 7:
      bit6 = true;
      SerialBT.println("bit6=true - Byte1=64");
      break;
    case 8:
      bit7 = true;
      SerialBT.println("bit7 = true - Byte1=128");
      break;
    case 9:
      bit8 = true;
      SerialBT.println("bit8 = true - Byte2=1");
      break;
    case 10:
      bit9 = true;
      SerialBT.println("bit9 = true - Byte2=2");
      break;
    case 11:
      bit10 = true;
      SerialBT.println("bit10 = true - Byte2=4");
      break;
    case 12:
      bit11 = true;
      SerialBT.println("bit11 = true - Byte2=8");
      break;
    case 13:
      bit12 = true;
      SerialBT.println("bit12 = true - Byte2=16");
      break;
    case 14:
      bit13 = true;
      SerialBT.println("bit13 = true - Byte2=32");
      break;
    case 15:
      bit14 = true;
      SerialBT.println("bit14 = true - Byte2=64");
      break;
    case 16:
      bit15 = true;
      SerialBT.println("bit15 = true - Byte2=128");
      break;
    case 30:
      SerialBT.println("ESP Resset!");
      delay(200);
      ESP.restart();
      break;
    case 31:
      // SerialBT.write(bit1);
      SerialBT.print("Byte1= ");
      SerialBT.print(binaryData_1);
      SerialBT.print("/DEC");
      SerialBT.print(" Byte2= ");
      SerialBT.print(binaryData_2);
      SerialBT.print("/DEC");
      SerialBT.print(" Byte3= ");
      SerialBT.println("/DEC");
      break;
    case 32:
      bit0 = false, bit1 = false, bit2 = false, bit3 = false, bit4 = false, bit5 = false, bit6 = false, bit7 = false;
      bit8 = false, bit9 = false, bit10 = false, bit11 = false, bit12 = false, bit13 = false, bit14 = false, bit15 = false;
      SerialBT.println("bit all false");
      break;
    }
  }
  /**************************************************************/
  binaryData_1 = 0;
  binaryData_1 |= bit0;
  binaryData_1 |= bit1 << true;
  binaryData_1 |= bit2 << 2;
  binaryData_1 |= bit3 << 3;
  binaryData_1 |= bit4 << 4;
  binaryData_1 |= bit5 << 5;
  binaryData_1 |= bit6 << 6;
  binaryData_1 |= bit7 << 7;
  binaryData_2 = 0;
  binaryData_2 |= bit8;
  binaryData_2 |= bit9 << true;
  binaryData_2 |= bit10 << 2;
  binaryData_2 |= bit11 << 3;
  binaryData_2 |= bit12 << 4;
  binaryData_2 |= bit13 << 5;
  binaryData_2 |= bit14 << 6;
  binaryData_2 |= bit15 << 7;

  //* Mainboard'a Gönderilen Veri
  Serial2.write(68);
  Serial2.write(84);
  Serial2.write(89);
  Serial2.write(binaryData_1);
  Serial2.write(binaryData_2);

  delay(60);
}