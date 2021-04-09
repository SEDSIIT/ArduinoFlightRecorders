/* 
 * This program is part of stage 1 of the Arduino Flight Recorders for high power rockets
 * Reads data from MPU6050 and BMP280 sensors and saves it on a SD card
 * Uses I2C to read values from sensors and SPI to save data on SD cards
 * Originally written for Arduino Nano (ATmega328P)
 * 
 * Initially public libraries will be used but we will transition to our own libraries soon enough
 * 
 * Created by: Michael Gromski
 *
 * Wiring Configuration:
 * --- SD CARD ---
 * MOSI - pin 11
 * MISO - pin 12
 * CLK/sck - pin 13
 * CS - 4
 * VCC - 5V
 * GND - GND
 * --- BMP280 ---
 * SCL - pin A5
 * SDA - pin A4
 * Vin - 5V
 * GND - GND
 * --- MPU6050 ---
 * SCL - pin A5
 * SDA - pin A4
 * ADO - GND
 * INT - pin 2
 * VCC - 5V
 * GND - GND
 * 
 */

#include <MPU6050_tockn.h>
#include <BMP280_DEV.h>
#include <SPI.h>
#include <SD.h>
#include <Wire.h>

MPU6050 mpu6050(Wire);                         // Create an object for the MPU6050 device
BMP280_DEV bmp280;                             // Create an object for the BMP280 device for I2C (address 0x77)
File dataFile;                                 // Create file object for the SD card

////////////////////// Global Variables //////////////////////
const int SDselect = 4;                        // Pin for CS
int numFiles = 0;                             
String fileName = "Data";                      // File name format "filename"XX.csv

float DATA[12]; // (DTime, temp, pres, alt, gyroX, gyroY, gyroZ, accX, accY, accZ, angleX, angleY, angleZ)

////////////////////// Setup //////////////////////
void setup()
{
  //Serial.begin(115200);                         // Used for debug can be disabled later
  
  Wire.begin();
  mpu6050.begin();
  mpu6050.calcGyroOffsets(true);                // The device will need to be stable for at least 5 sec
  
  bmp280.begin(BMP280_I2C_ALT_ADDR);
  bmp280.setPresOversampling(OVERSAMPLING_X4);  // Options are OVERSAMPLING_SKIP, _X1, _X2, _X4, _X8, _X16
  bmp280.setTempOversampling(OVERSAMPLING_X4);  // Options are OVERSAMPLING_SKIP, _X1, _X2, _X4, _X8, _X16
  bmp280.setTimeStandby(TIME_STANDBY_62MS);     // Options are TIME_STANDBY_05MS, _62MS, _125MS, _250MS, _500MS, _1000MS, 2000MS, 4000MS
  bmp280.setIIRFilter(IIR_FILTER_4);            // Options are IIR_FILTER_OFF, _2, _4, _8, _16
  bmp280.startNormalConversion();

  if (!SD.begin(SDselect))                      // Stops device is no SD card is found
  {
    Serial.println("Card failed");
    while(1);
  }
  dataFile = SD.open("/");                      // Opens at highest directory
  fileName = fileName + String(numOfFiles(dataFile)) + String(".csv");
  dataFile.close();   
  //Serial.println(fileName);                   // For debug
  dataFile = SD.open(fileName, FILE_WRITE);
  if (dataFile)                                 // Print header of CSV file
  {
    dataFile.print("Time(ms),Temp(C),Pres(hPa),Alt(m),GyroX,GyroY,GyroZ,AccX,AccY,AccZ,AngX,AngleY,AngZ,\n");
  }
  dataFile.close();
}

////////////////////// Main ////////////////////// 
void loop()
{
  DATA[0] = millis();                             // Keep track of time in milliseconds

  mpu6050.update();                               // Get latest measurements from MPU6050
  DATA[4] = mpu6050.getGyroX();
  DATA[5] = mpu6050.getGyroY();
  DATA[6] = mpu6050.getGyroZ();
  DATA[7] = mpu6050.getAccX();
  DATA[8] = mpu6050.getAccY();
  DATA[9] = mpu6050.getAccZ();
  DATA[10] = mpu6050.getAngleX();
  DATA[11] = mpu6050.getAngleY();
  DATA[12] = mpu6050.getAngleZ();
  
  bmp280.getMeasurements(DATA[1], DATA[2], DATA[3]);   // (temp,pres,alt) Temp in C, Pres in hPa, alt in m
  
  //printData();
  saveData(fileName);
}

////////////////////// Functions //////////////////////
void printData()                                  // Prints data in serial monitor
{
  for (int i = 0; i < 13; i++) 
  {
   Serial.print(DATA[i]);
   Serial.print(", ");
  }
  Serial.println();
}

int numOfFiles(File dir)                           // Determines number of files located in directory
{
  while (true) 
  {
    File entry = dir.openNextFile();
    if (! entry)
    {
      break;
    }
    numFiles++;
    entry.close();
  }
  return numFiles;
}

void saveData(String fileName)                      // Saves data on SD card
{
  dataFile = SD.open(fileName, FILE_WRITE);
  if (dataFile)
  {
    for (int i = 0; i < 13; i++)
    {
      dataFile.print(DATA[i]);
      dataFile.print(",");
    }
    dataFile.print("\n");
  }
  dataFile.close();
}
