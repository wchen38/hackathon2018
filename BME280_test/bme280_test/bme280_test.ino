#include <stdint.h>
#include "SparkFunBME280.h"
#include "Wire.h"

#include <LoRa.h>
#include <Arduino.h>
#include <SPI.h>
#include "SSD1306.h" // alias for `#include "SSD1306Wire.h"`

//gas sensor includes
#include "MQ135.h"

#define MQ135_AO 35 //  Define Analog PIN on Arduino Board 
#define RZERO 206.85    //  Define RZERO Calibration Value

#define MQ7_AO 34   //  Define Analog PIN on Arduino Board 

#define SS      18
#define RST     14
#define DI0     26
#define BAND    433E6  //915E6 -- 这里的模式选择中，检查一下是否可在中国实用915这个频段


BME280 BME280;
char stringa[30];
SSD1306 display(0x3c, 4, 15);
MQ135 gasSensor = MQ135(MQ135_AO);
int counter = 0;
void setup()
{
  /* OLED display set up */
  pinMode(16, OUTPUT);
  digitalWrite(16, LOW);    // set GPIO16 low to reset OLED
  delay(50);
  digitalWrite(16, HIGH); // while OLED is running, must set GPIO16 in high

  /*OLED display Setup end*/

  BME280.settings.commInterface = I2C_MODE;
  BME280.settings.I2CAddress = 0x77;


  //runMode- 0 = Sleep mode, 1 and 2 = Forced Mode, 3 = Normal Mode
  BME280.settings.runMode = 3; //Normal mode

  //tStandby can be:
  //  0 = 0.5ms, 1 = 62.5ms, 2 = 125ms, 3 = 250ms, 4 = 500ms, 5 = 1000ms, 6 = 10ms, 7 = 20ms
  BME280.settings.tStandby = 0;

  //filter can be off or number of FIR coefficients to use:
  //  0 = filter off, 1 = coefficients = 2, 2 = coefficients = 4, 3 = coefficients = 8, 4 = coefficients = 16
  BME280.settings.filter = 0;

  //tempOverSample can be:
  //  0, skipped
  //  1 through 5, oversampling *1, *2, *4, *8, *16 respectively
  BME280.settings.tempOverSample = 1;

  //pressOverSample can be:
  //  0, skipped
  //  1 through 5, oversampling *1, *2, *4, *8, *16 respectively
  BME280.settings.pressOverSample = 1;

  //humidOverSample can be:
  //  0, skipped
  //  1 through 5, oversampling *1, *2, *4, *8, *16 respectively
  BME280.settings.humidOverSample = 1;

  Serial.begin(115200);
  
  Serial.print("Program Started\n");
  Serial.print("Starting BME280... result of .begin(): 0x");

  //Calling .begin() causes the settings to be loaded
  delay(10);  //Make sure sensor had enough time to turn on. BME280 requires 2ms to start up.
  Serial.println(BME280.begin(), HEX);

  Serial.print("Displaying ID, reset and ctrl regs\n");

  Serial.print("ID(0xD0): 0x");
  Serial.println(BME280.readRegister(BME280_CHIP_ID_REG), HEX);
  Serial.print("Reset register(0xE0): 0x");
  Serial.println(BME280.readRegister(BME280_RST_REG), HEX);
  Serial.print("ctrl_meas(0xF4): 0x");
  Serial.println(BME280.readRegister(BME280_CTRL_MEAS_REG), HEX);
  Serial.print("ctrl_hum(0xF2): 0x");
  Serial.println(BME280.readRegister(BME280_CTRL_HUMIDITY_REG), HEX);


  //lora set up ----------------------------
  SPI.begin(5, 19, 27, 18);
  LoRa.setPins(SS, RST, DI0);
  //  Serial.println("LoRa Sender");

  if (!LoRa.begin(BAND)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
  Serial.println("LoRa Initial OK!");


  // Initialising the UI will init the display too.
  
  // display.init();
 // display.flipScreenVertically();
 // display.setFont(ArialMT_Plain_10);

//------------------Gas sensor --------------------
  //pinMode(MQ_DO, INPUT);
  // pinMode(MQ7_AO, INPUT);
 float rzero = gasSensor.getRZero();
   delay(3000);
  Serial.print("MQ135 RZERO Calibration Value : ");
  Serial.println(rzero);
}



void loop()
{

  //BME280
  Serial.print("Temperature: ");
  float tempC = BME280.readTempC();
  Serial.print(tempC, 2);
  Serial.println(" degrees C");

  Serial.print("Temperature: ");
  Serial.print(BME280.readTempF(), 2);
  Serial.println(" degrees F");

  Serial.print("Pressure: ");
  Serial.print(BME280.readFloatPressure(), 2);
  Serial.println(" Pa");

  Serial.print("Altitude: ");
  Serial.print(BME280.readFloatAltitudeMeters(), 2);
  Serial.println("m");

  Serial.print("Altitude: ");
  Serial.print(BME280.readFloatAltitudeFeet(), 2);
  Serial.println("ft");

  Serial.print("%RH: ");
  float humidity = BME280.readFloatHumidity();
  Serial.print(humidity, 2);
  Serial.println(" %");

  Serial.println();

  //  delay(1000);

  // above is BME280 Code


  // digitalWrite(25, HIGH);   // turn the LED on (HIGH is the voltage level)
 // delay(1000);                       // wait for a second
  // digitalWrite(25, LOW);    // turn the LED off by making the voltage LOW
  // delay(1000);                       // wait for a second

  //delay(4000);

  //-------------- gas sensor firmware -----------------------
  float mq135_ppm = gasSensor.getPPM();
  delay(1000);
  Serial.print("CO2 ppm value : ");
  Serial.print(mq135_ppm);
  Serial.print("\n");

    //gas sensor
  int mq7_val = analogRead(MQ7_AO);
  Serial.print("MQ7---------->");
  Serial.print(mq7_val);
  Serial.print("\n");

  sprintf(stringa, "%0.2f %0.2f %0.2f %d\n", tempC, humidity, mq135_ppm, mq7_val); 
  
    // send packet
  Serial.print("Sending packet: ");
  Serial.println(stringa);
  LoRa.beginPacket();
  // LoRa.print("hello ");
  LoRa.print(stringa);
  LoRa.endPacket();

  counter++;

}
