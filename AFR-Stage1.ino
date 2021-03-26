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
 * CLK - pin 13
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
unsigned long DTime = 0;                      // Used to keep time from initial power-up

float temp, pres, alt;                         // Variables for bmp280

const int SDselect = 4;                        // Pin for CS
int numFiles = 0;                             
String fileName = "Data";                      // File name format "filename"XX.csv

long gyroX, gyroY, gyroZ, accX, accY, accZ, angleX, angleY, angleZ;

////////////////////// Setup //////////////////////
void setup()
{
  Serial.begin(115200);                         // Used for debug can be disabled later
  while(!Serial) { }                            // Wait for Serial (IMPORTANT TO COMMENT OUT LATER OR DEVICE WON'T WORK)

  Wire.begin();
  mpu6050.begin();
  mpu6050.calcGyroOffsets(true);                // The device will need to be stable for at least 10 sec
  
  bmp280.begin();
  bmp280.setPresOversampling(OVERSAMPLING_X4);  // Options are OVERSAMPLING_SKIP, _X1, _X2, _X4, _X8, _X16
  bmp280.setTempOversampling(OVERSAMPLING_X4);  // Options are OVERSAMPLING_SKIP, _X1, _X2, _X4, _X8, _X16
  bmp280.setTimeStandby(TIME_STANDBY_62MS);     // Options are TIME_STANDBY_05MS, _62MS, _125MS, _250MS, _500MS, _1000MS, 2000MS, 4000MS
  bmp280.setIIRFilter(IIR_FILTER_16);           // Options are IIR_FILTER_OFF, _2, _4, _8, _16
  bmp280.startNormalConversion();

  Serial.print("Initializing SD card...");
  if (!SD.begin(SDselect))                      // Stops device is no SD card is found
  {
    Serial.println("Card failed");
    while(1);                                   // Write some form of led debug function here ****
  }
  Serial.println("card initialized.");
  dataFile = SD.open("/");                      // Open at highest directory
  fileName = fileName + String(numOfFiles(dataFile)) + String(".csv");
  dataFile = SD.open(fileName, FILE_WRITE);
  if (dataFile)                                 // Print header of CSV file
  {
    dataFile.print("Time(ms),Temperature(C),Pressure(hPa),Altitude(m),GyroX,GyroY,GyroZ,AccX,AccY,AccZ,AngleX(°),AngleY(°),AngleZ(°)");
    dataFile.close();
  }
}

////////////////////// Main ////////////////////// 
void loop()
{
  DTime = millis();                              // Keep track of time in milliseconds
  
  mpu6050.update();                              // Get latest measurements from MPU6050
  gyroX = mpu6050.getGyroX();
  gyroY = mpu6050.getGyroY();
  gyroZ = mpu6050.getGyroZ();
  accX = mpu6050.getAccX();
  accY = mpu6050.getAccY();
  accZ = mpu6050.getAccZ();
  angleX = mpu6050.getAngleX();
  angleY = mpu6050.getAngleY();
  angleZ = mpu6050.getAngleZ();

  if (bmp280.getMeasurements(temp, pres, alt))   // Temp in C, Pres in hPa, alt in m
  {
    Serial.print(temp);                          // Display the results (for debug)   
    Serial.print("*C   ");
    Serial.print(pres);    
    Serial.print("hPa   ");
    Serial.print(alt);
    Serial.println("m");
  }
  saveData();
}

////////////////////// Functions //////////////////////
int numOfFiles(File dir)                          // Returns number of files on SD card
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

void saveData() 
{
  if (dataFile)
  {
    dataFile.print(DTime);
    dataFile.print(",");
    dataFile.print(temp);
    dataFile.print(",");
    dataFile.print(pres);
    dataFile.print(",");
    dataFile.print(alt);
    dataFile.print(",");
    dataFile.print(gyroX);
    dataFile.print(",");
    dataFile.print(gyroY);
    dataFile.print(",");
    dataFile.print(gyroZ);
    dataFile.print(",");
    dataFile.print(accX);
    dataFile.print(",");
    dataFile.print(accY);
    dataFile.print(",");
    dataFile.print(accZ);
    dataFile.print(",");
    dataFile.print(angleX);
    dataFile.print(",");
    dataFile.print(angleY);
    dataFile.print(",");
    dataFile.print(angleZ);
    dataFile.print("\n");
    dataFile.close();
  }
}
