// This example just provide basic LoRa function test;
// Not the LoRa's farthest distance or strongest interference immunity.
// For more informations, please vist www.heltec.cn or mail to support@heltec.cn

#include <SPI.h>
#include <LoRa.h>
#include "SSD1306.h"
#include <stdio.h>
#include <string.h>

// WIFI_LoRa_32 ports

// GPIO5  -- SX1278's SCK
// GPIO19 -- SX1278's MISO
// GPIO27 -- SX1278's MOSI
// GPIO18 -- SX1278's CS
// GPIO14 -- SX1278's RESET
// GPIO26 -- SX1278's IRQ(Interrupt Request)
SSD1306 display(0x3c, 4, 15);

#define SS      18
#define RST     14
#define DI0     26
#define BAND    433E6
#define LED     25

double temperature, humidity, co2 = 0.0;
int co = 0, i = 0;
char *token0, *token1, *token2, *token3;

void setup() {
  Serial.begin(115200);
  while (!Serial); //if just the the basic function, must connect to a computer
  delay(1000);

  pinMode(16, OUTPUT);
  digitalWrite(16, LOW); // set GPIO16 low to reset OLED
  delay(50);
  digitalWrite(16, HIGH);

  display.init();
  display.flipScreenVertically();
  display.setFont(ArialMT_Plain_10);

  Serial.println("LoRa Receiver");

  SPI.begin(5, 19, 27, 18);
  LoRa.setPins(SS, RST, DI0);

  if (!LoRa.begin(BAND)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }

  Serial.println("LoRa Initial OK!");

  display.drawString(5, 25, "LoRa Initializing OK!");
  display.display();

  pinMode(LED, OUTPUT);

}

void loop() {
  // try to parse packet

  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    // received a packet
    Serial.print("Received packet ");
    display.clear();
    display.setTextAlignment(TEXT_ALIGN_LEFT);
    display.setFont(ArialMT_Plain_10);
    display.drawString(0, 0, "Received packet");
    // display.display();

    // read packet
    while (LoRa.available()) {
      // Serial.print((char)LoRa.read());
      String datastr = LoRa.readString();
      char *cpy, data[100];
      cpy = &datastr[0u];
      strcpy(data, cpy);
      //---------------parsing--------------
      token0 = strtok(data, " ");
      temperature = atof(token0);
      token1 = strtok(NULL, " ");
      humidity = atof(token1);
      token2 = strtok(NULL, " ");
      co2 = atof(token2);
      token3 = strtok(NULL, " ");
      co = atoi(token3);


      Serial.print(data);
      Serial.print("T: ");
      Serial.print(temperature);
      Serial.print(" H: ");
      Serial.print(humidity);
      Serial.print(" CO2: ");
      Serial.print(co2);
      Serial.print(" CO: ");
      Serial.print(co);
      Serial.print("\n");

      display.setTextAlignment(TEXT_ALIGN_LEFT);
      display.setFont(ArialMT_Plain_10);
      display.drawString(0, 11, String("Temp: ") + token0);
      display.drawString(0, 21, String("RH    : ") + token1);
      display.drawString(0, 31, String("CO2  : ") + token2);
      display.drawString(0, 41, String("CO    : ") + token3);
      display.display();

      
      if(co2 >= 40 || co >= 250){
        digitalWrite(LED, HIGH);
       // delay(100);  
      }else{
        digitalWrite(LED, LOW);
       // delay(100);
      }
    } 

    


    // print RSSI of packet
    Serial.print("' with RSSI ");
    Serial.println(LoRa.packetRssi());
  }
}
