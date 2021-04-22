/*
 * Written by: Kyle Duong
 * 
 * Components for this part:
 *    x1 Arduino Nano
 *    x1 Breadboard
 *    x1 RF24 (Transmitter)
 *    x1 DHT11
 *    x1 Resistor
 *    
 * This part is the outdoor portion.
 * This half of the projects uses the DHT sensor 
 * to read the temperature and humidity of the area.
 * It then updates the information of the package and
 * sends the package out using the RF24. 
 */

//------------------------------------------------------------------

// Libraries for the transmitter
#include <SPI.h>
#include "RF24.h"

// Library for the sensor
#include <DHT.h>

//------------------------------------------------------------------

// Sensor pin
const int DHT_SENSOR = 2;

// Setting the dht sensor
DHT dht(DHT_SENSOR, DHT11);

//------------------------------------------------------------------

// Setting the transmitter pins
RF24 transmitter(7, 8);

// Address at 0
byte addresses [][6] = {"0"};

// Information transmitted
struct package
{
  float temp = 0.0;
  float humid = 0;
};

typedef struct package Package;
Package data;

//------------------------------------------------------------------

void setup() {
  // Setting up the transmitter
  transmitter.begin();  
  transmitter.setChannel(113); 
  transmitter.setPALevel(RF24_PA_MAX);
  transmitter.setDataRate(RF24_250KBPS) ; 
  transmitter.openWritingPipe(addresses[0]);

  // Setting up the dht sensor
  dht.begin();
}

//------------------------------------------------------------------

void loop() {
  updatePackage();
  
  // Transmits the data package
  transmitter.write(&data, sizeof(data)); 
}

//------------------------------------------------------------------

// Using the sensor, updates the package information
void updatePackage(){
  // Reads the temp
  data.temp = dht.readTemperature();
  // Reads the humidity
  data.humid = dht.readHumidity();
}

//------------------------------------------------------------------
