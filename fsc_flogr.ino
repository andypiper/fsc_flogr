/*
 An extensible data logger
 Stores data to SD card for easy analysis
 
 Parses NMEA sentences from an EM406 running at 4800bps into readable 
 values for date, time, latitude, longitude, elevation, course, and 
 speed. Make sure the shield switch is set to DLINE.

 Barometric pressure from BMP085 
*/

// Leaving the Serial logging enabled can cause avrdude to fail to transfer
// compiled binary to the board!!

// use TinyGPS library from arduiniana.org 
#include <SoftwareSerial.h>
#include <TinyGPS.h>
#include <SD.h>
#include <Wire.h>

// pin for the SD card I/O
const int chipSelect = 10;

// the number of the pushbutton pin
const int buttonPin = 6;     
// variable for reading the pushbutton status
int buttonState = LOW;         

// barometer BMP085 values
#define BMP085_ADDRESS 0x77  // I2C address of BMP085
const unsigned char OSS = 0;  // Oversampling Setting

// Calibration values
int ac1;
int ac2; 
int ac3; 
unsigned int ac4;
unsigned int ac5;
unsigned int ac6;
int b1; 
int b2;
int mb;
int mc;
int md;

// b5 is calculated in bmp085GetTemperature(...), 
// this variable is also used in bmp085GetPressure(...)
// so ...Temperature(...) must be called before ...Pressure(...).
long b5; 

short temperature;
long pressure;

// GPS TX/RX pins
#define RXPIN 2
#define TXPIN 3
// GPS baud rate
#define GPSBAUD 4800
// time to wait for GPS 'settle'
#define GPSDELAY 6000 // 6 seconds

// Create an instance of the TinyGPS object
TinyGPS gps;
SoftwareSerial uart_gps(RXPIN, TXPIN);

// declare prototypes for the functions that will be 
// using the TinyGPS library
void getgps(TinyGPS &gps);
char s[32];

// initialize two serial ports; the standard hardware serial port 
// (Serial()) and another serial port (NewSoftSerial()) for GPS
void setup()
{
  pinMode(buttonPin, INPUT);     

  // fast serial rate for terminal program - if slower the messages 
  // might not be valid -> checksum errors
  Serial.begin(115200);

  // setup BMP085
  Wire.begin();
  bmp085Calibration();

  // setup SD card or give up
  Serial.print("Initializing SD card...");
  pinMode(chipSelect, OUTPUT);
  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    return; // exit
  }

  Serial.println("card initialized.");

  //Set baud rate of GPS
  uart_gps.begin(GPSBAUD);

  Serial.print("TinyGPS library v. "); 
  Serial.println(TinyGPS::library_version());

  File dataFile = SD.open("datalog.txt", FILE_WRITE);
  // if the file is available, write to it:
  if (dataFile) {
    dataFile.println("------ ");
    dataFile.print("TinyGPS library v. ");
    dataFile.println(TinyGPS::library_version());
    dataFile.close();
  }  
  // if the file isn't open, pop up an error:
  else {
    Serial.println("error opening datalog.txt");
  } 
  Serial.println("...waiting for lock...");
  // attempt to wait for GPS to settle before starting
  delay(GPSDELAY);
  Serial.println("ready");
}

// This is the main loop of the code. Check for data on 
// the RX pin of the Arduino, makes sure the data is valid NMEA sentences, 
// then jumps to the getdata() function.
void loop()
{
  // read the state of the pushbutton value:
  buttonState = digitalRead(buttonPin);

  if (buttonState == HIGH) {     // if button is pressed

    // For one second parse GPS data and report some key values
    for (unsigned long start = millis(); millis() - start < 1000;)
    {
      while(uart_gps.available())     // While there is data on RX 
      {
        int c = uart_gps.read();    // load data into a variable
        //Serial.write(c); // uncomment this line to see raw GPS data
        if (gps.encode(c)) // if a new valid sentence arrived
          getdata(gps);
      }
    }
  }
}

// The getgps function will get and print values from the NMEA structure
void getdata(TinyGPS &gps)
{
  // To see the complete list of functions see keywords.txt file in 
  // the TinyGPS and NewSoftSerial libs.

  // date and time
  int year;
  byte month, day, hour, minute, second, hundredths;
  gps.crack_datetime(&year,&month,&day,&hour,&minute,&second,&hundredths);

  // Print date and time
//  Serial.print("\nDate: "); 
//  Serial.print(day, DEC); 
//  Serial.print("/"); 
//  Serial.print(month, DEC); 
//  Serial.print("/"); 
//  Serial.print(year);
//  Serial.print("  Time: "); 
//  Serial.print(hour, DEC); 
//  Serial.print(":"); 
//  Serial.print(minute, DEC); 
//  Serial.print(":"); 
//  Serial.print(second, DEC); 
//  Serial.print("."); 
//  Serial.println(hundredths, DEC);
//

  /* FIXME: the following code is unreliable
   attempts to check for single digit date/time values and pad with zero
   should be replaced with strcat/strcpy/etc
   */
  String mo = dtostrf(month, 1, 0, s);
  if (mo.length() < 2) { 
    mo = "0" + mo; 
  }
  String da = dtostrf(day, 1, 0, s);
  if (da.length() < 2) { 
    da = "0" + da; 
  }

  String hr = dtostrf(hour, 1, 0, s);
  if (hr.length() < 2) { 
    hr = "0" + hr; 
  }  
  String mn = dtostrf(minute, 1, 0, s);
  if (mn.length() < 2) { 
    mn = "0" + mn; 
  }
  String sc = dtostrf(second, 1, 0, s); 
  if (sc.length() < 2) { 
    sc = "0" + sc; 
  }

  // Define the lat/lon variables
  float latitude, longitude;
  unsigned long age;
  gps.f_get_position(&latitude, &longitude, &age);

  // print latitude and longitude
//  Serial.print("Lat/Long: "); 
//  Serial.print(latitude == TinyGPS::GPS_INVALID_F_ANGLE ? 0.0 : latitude, 6); 
//  Serial.print(", "); 
//  Serial.println(longitude == TinyGPS::GPS_INVALID_F_ANGLE ? 0.0 : longitude, 6);

  // number of satellites
  int sats = gps.satellites() == TinyGPS::GPS_INVALID_SATELLITES ? 0 : gps.satellites();
//  Serial.print("Satellites in view: "); 
//  Serial.println(sats);

  // print the altitude and course values
//  Serial.print("Altitude (meters): "); 
//  Serial.println(gps.f_altitude());  
//  Serial.print("Course (degrees): "); 
//  Serial.println(gps.f_course()); 
//  Serial.print("Speed (kmph): "); 
//  Serial.println(gps.f_speed_kmph());
//  Serial.println();

  temperature = bmp085GetTemperature(bmp085ReadUT());
  pressure = bmp085GetPressure(bmp085ReadUP());

//  Serial.print("Temperature: ");
//  Serial.print(temperature, DEC);
//  Serial.println(" *0.1 deg C");
//  Serial.print("Pressure: ");
//  Serial.print(pressure, DEC);
//  Serial.println(" Pa");
//  Serial.println();

  Serial.println("data capture...");
  
  // open and write to file
  File dataFile = SD.open("datalog.txt", FILE_WRITE);
  // if the file is available, write to it:
  if (dataFile) {
    dataFile.print(dtostrf(year, 4, 0, s)); //year
    dataFile.print(mo); //month
    dataFile.print(da); //day
    dataFile.print("T");
    dataFile.print(hr); //hour
    dataFile.print(mn); //min
    dataFile.print(sc); //sec
    dataFile.print("+00:00");
    dataFile.print(",");
    dataFile.print(dtostrf(latitude, 1, 5, s)); //lat
    dataFile.print(",");
    dataFile.print(dtostrf(longitude, 1, 5, s)); //lon
    dataFile.print(",");
    dataFile.print(sats); //no. of sats
    dataFile.print(",");
    dataFile.print(dtostrf(gps.f_altitude(),1,2,s)); //alt
    dataFile.print(",");
    dataFile.print(dtostrf(gps.f_course(),1,2,s)); //course
    dataFile.print(",");
    dataFile.print(dtostrf(gps.f_speed_kmph(),1,2,s)); //speed kmph
    dataFile.print(",");
    dataFile.print(dtostrf(temperature,1,2,s)); //temp *0.1 deg C
    dataFile.print(",");
    dataFile.println(dtostrf(pressure,1,2,s)); //pressure Pa
    dataFile.close();
  }  
  // if the file isn't open, write an error to serial
  else {
    Serial.println("error opening datalog.txt");
  } 

}

/* helper functions for the barometer BMP085 */

// Stores all of the bmp085's calibration values into global variables
// Calibration values are required to calculate temp and pressure
// This function should be called at the beginning of the program
void bmp085Calibration()
{
  ac1 = bmp085ReadInt(0xAA);
  ac2 = bmp085ReadInt(0xAC);
  ac3 = bmp085ReadInt(0xAE);
  ac4 = bmp085ReadInt(0xB0);
  ac5 = bmp085ReadInt(0xB2);
  ac6 = bmp085ReadInt(0xB4);
  b1 = bmp085ReadInt(0xB6);
  b2 = bmp085ReadInt(0xB8);
  mb = bmp085ReadInt(0xBA);
  mc = bmp085ReadInt(0xBC);
  md = bmp085ReadInt(0xBE);
}

// Calculate temperature given ut.
// Value returned will be in units of 0.1 deg C
short bmp085GetTemperature(unsigned int ut)
{
  long x1, x2;
  
  x1 = (((long)ut - (long)ac6)*(long)ac5) >> 15;
  x2 = ((long)mc << 11)/(x1 + md);
  b5 = x1 + x2;

  return ((b5 + 8)>>4);  
}

// Calculate pressure given up
// calibration values must be known
// b5 is also required so bmp085GetTemperature(...) must be called first.
// Value returned will be pressure in units of Pa.
long bmp085GetPressure(unsigned long up)
{
  long x1, x2, x3, b3, b6, p;
  unsigned long b4, b7;
  
  b6 = b5 - 4000;
  // Calculate B3
  x1 = (b2 * (b6 * b6)>>12)>>11;
  x2 = (ac2 * b6)>>11;
  x3 = x1 + x2;
  b3 = (((((long)ac1)*4 + x3)<<OSS) + 2)>>2;
  
  // Calculate B4
  x1 = (ac3 * b6)>>13;
  x2 = (b1 * ((b6 * b6)>>12))>>16;
  x3 = ((x1 + x2) + 2)>>2;
  b4 = (ac4 * (unsigned long)(x3 + 32768))>>15;
  
  b7 = ((unsigned long)(up - b3) * (50000>>OSS));
  if (b7 < 0x80000000)
    p = (b7<<1)/b4;
  else
    p = (b7/b4)<<1;
    
  x1 = (p>>8) * (p>>8);
  x1 = (x1 * 3038)>>16;
  x2 = (-7357 * p)>>16;
  p += (x1 + x2 + 3791)>>4;
  
  return p;
}

// Read 1 byte from the BMP085 at 'address'
char bmp085Read(unsigned char address)
{
  unsigned char data;
  
  Wire.beginTransmission(BMP085_ADDRESS);
  Wire.write(address);
  Wire.endTransmission();
  
  Wire.requestFrom(BMP085_ADDRESS, 1);
  while(!Wire.available())
    ;
    
  return Wire.read();
}

// Read 2 bytes from the BMP085
// First byte will be from 'address'
// Second byte will be from 'address'+1
int bmp085ReadInt(unsigned char address)
{
  unsigned char msb, lsb;
  
  Wire.beginTransmission(BMP085_ADDRESS);
  Wire.write(address);
  Wire.endTransmission();
  
  Wire.requestFrom(BMP085_ADDRESS, 2);
  while(Wire.available()<2)
    ;
  msb = Wire.read();
  lsb = Wire.read();
  
  return (int) msb<<8 | lsb;
}

// Read the uncompensated temperature value
unsigned int bmp085ReadUT()
{
  unsigned int ut;
  
  // Write 0x2E into Register 0xF4
  // This requests a temperature reading
  Wire.beginTransmission(BMP085_ADDRESS);
  Wire.write(0xF4);
  Wire.write(0x2E);
  Wire.endTransmission();
  
  // Wait at least 4.5ms
  delay(5);
  
  // Read two bytes from registers 0xF6 and 0xF7
  ut = bmp085ReadInt(0xF6);
  return ut;
}

// Read the uncompensated pressure value
unsigned long bmp085ReadUP()
{
  unsigned char msb, lsb, xlsb;
  unsigned long up = 0;
  
  // Write 0x34+(OSS<<6) into register 0xF4
  // Request a pressure reading w/ oversampling setting
  Wire.beginTransmission(BMP085_ADDRESS);
  Wire.write(0xF4);
  Wire.write(0x34 + (OSS<<6));
  Wire.endTransmission();
  
  // Wait for conversion, delay time dependent on OSS
  delay(2 + (3<<OSS));
  
  // Read register 0xF6 (MSB), 0xF7 (LSB), and 0xF8 (XLSB)
  Wire.beginTransmission(BMP085_ADDRESS);
  Wire.write(0xF6);
  Wire.endTransmission();
  Wire.requestFrom(BMP085_ADDRESS, 3);
  
  // Wait for data to become available
  while(Wire.available() < 3)
    ;
  msb = Wire.read();
  lsb = Wire.read();
  xlsb = Wire.read();
  
  up = (((unsigned long) msb << 16) | ((unsigned long) lsb << 8) | (unsigned long) xlsb) >> (8-OSS);
  
  return up;
}


