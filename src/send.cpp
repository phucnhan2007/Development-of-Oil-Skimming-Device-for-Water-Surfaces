#include <Arduino.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
// mua tay cam dieu khien + ESC + bldc motor, khong can lap trinh
// lam them phan do ph va tds cua nuoc
// day se la thiet bi //NOTE: gui nrf
//--------------------------------------
//  nrf24 defines &
#define CE 9
#define CSN 10
const uint64_t address = 0xf0f0f0f066;
//--------------------------------------
unsigned long lastTime = 0;
//--------------------------------------
#define TdsSensorPin A1 // tds sensor Analog output to Arduino Analog Input 1
#define pHPin A0        // pH meter Analog output to Arduino Analog Input 0
#define OffsetPh 21.34  // deviation compensate
//==================================================================================================
struct MeasureData // 8 bytes (max of 32 bytes)
{
    float ch1; // ph
    float ch2; // tds
    byte ch3;  // 0-1
    byte ch4;  // 0-1
    byte ch5;
    byte ch6;
    byte ch7;
    byte ch8;
} measureData;
//==================================================================================================
RF24 radio(CE, CSN);
//--------------------------------------
//==================================================================================================
void setup()
{
    // put your setup code here, to run once:
    Serial.begin(9600);
    //--------------------------------------
    if (!radio.begin())
    {
        Serial.println(F("radio hardware is not responding!!"));
    }
    else
    {
        Serial.println(F("radio hardware is  responding!!"));
    }
    radio.setAutoAck(false);
    radio.setDataRate(RF24_250KBPS); // Tốc độ dữ liệu
    // radio.openReadingPipe(1, address);
    // radio.startListening();
    radio.openWritingPipe(address);

    measureData.ch1 = 1.0;
    measureData.ch2 = 2.0;
}
//==================================================================================================
void revData()
{
    while (radio.available())
    {
        radio.read(&measureData, sizeof(measureData));
        lastTime = millis();
    }
}

//==================================================================================================
void loop()
{
    // revData();
    // put your main code here, to run repeatedly:
    if (millis() - lastTime > 1000)
    {
        int a = analogRead(pHPin);
        int b = analogRead(TdsSensorPin);
        float vol = (float)a * 5.0 / 1024.0;
        measureData.ch1 = (7 + ((2.5 - vol) / 0.167));
        // measureData.ch1 = -5.70 * vol + OffsetPh;
        Serial.print(vol);
        Serial.print("\t");
        vol = b * 5.0 / 1024.0;
        float compensationCoefficient = 1.0 + 0.02 * (30.0 - 25.0); // 30 is the temperature compensation coefficient
        float compensationVol = vol / compensationCoefficient;
        measureData.ch2 = (133.42 * compensationVol * compensationVol * compensationVol - 255.86 * compensationVol * compensationVol + 857.39 * compensationVol) / 2;
        lastTime = millis();
        Serial.print(measureData.ch1);
        Serial.print("\t");
        Serial.print(measureData.ch2);
        Serial.print("\n");
    }
    // measureData.ch1 = random(0, 10);
    // measureData.ch2 = random(0, 10);
    radio.write(&measureData, sizeof(measureData));
}
