/*
 An extensible data logger
 Stores data to SD card for easy analysis
 
 Parses NMEA sentences from an EM406 running at 4800bps into readable 
 values for date, time, latitude, longitude, elevation, course, and 
 speed. Make sure the shield switch is set to DLINE.

 Barometric pressure added 
*/

// use TinyGPS library from arduiniana.org 
#include <SoftwareSerial.h>
#include <TinyGPS.h>
#include <SD.h>

// pin for the SD card I/O
const int chipSelect = 10;

// the number of the pushbutton pin
const int buttonPin = 6;     
// variable for reading the pushbutton status
int buttonState = LOW;         

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

  // setup SD card or give up
  Serial.print("Initializing SD card...");
  pinMode(chipSelect, OUTPUT);
  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    // don't do anything more
    return;
  }

  Serial.println("card initialized.");

  //Set baud rate of GPS
  uart_gps.begin(GPSBAUD);

  Serial.println("");
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
  Serial.println("");

  // attempt to wait for GPS to settle before starting
  delay(GPSDELAY);

}

// This is the main loop of the code. All it does is check for data on 
// the RX pin of the Arduino, makes sure the data is valid NMEA sentences, 
// then jumps to the getgps() function.
void loop()
{
  // read the state of the pushbutton value:
  buttonState = digitalRead(buttonPin);
  Serial.println(buttonState);

  if (buttonState == HIGH) {     

    // For one second parse GPS data and report some key values
    for (unsigned long start = millis(); millis() - start < 1000;)
    {
      while(uart_gps.available())     // While there is data on RX 
      {
        int c = uart_gps.read();    // load data into a variable
        //Serial.write(c); // uncomment this line to see raw GPS data
        if (gps.encode(c)) // Did a new valid sentence come in?
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
  Serial.print("\nDate: "); 
  Serial.print(day, DEC); 
  Serial.print("/"); 
  Serial.print(month, DEC); 
  Serial.print("/"); 
  Serial.print(year);
  Serial.print("  Time: "); 
  Serial.print(hour, DEC); 
  Serial.print(":"); 
  Serial.print(minute, DEC); 
  Serial.print(":"); 
  Serial.print(second, DEC); 
  Serial.print("."); 
  Serial.println(hundredths, DEC);


  /* the following code is unreliable
   attempts to check for single digit date/time values and pad with zero
   should be replaced with strcat/strcpy/etc
   */
  String mo = dtostrf(month, 2, 0, s);
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
  Serial.print("Lat/Long: "); 
  Serial.print(latitude == TinyGPS::GPS_INVALID_F_ANGLE ? 0.0 : latitude, 6); 
  Serial.print(", "); 
  Serial.println(longitude == TinyGPS::GPS_INVALID_F_ANGLE ? 0.0 : longitude, 6);

  // number of satellites
  int sats = gps.satellites() == TinyGPS::GPS_INVALID_SATELLITES ? 0 : gps.satellites();
  Serial.print("Satellites in view: "); 
  Serial.println(sats);

  // print the altitude and course values
  Serial.print("Altitude (meters): "); 
  Serial.println(gps.f_altitude());  
  Serial.print("Course (degrees): "); 
  Serial.println(gps.f_course()); 
  Serial.print("Speed (kmph): "); 
  Serial.println(gps.f_speed_kmph());
  Serial.println();

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
    dataFile.print(sats); //no. of sats locked
    dataFile.print(",");
    dataFile.print(dtostrf(gps.f_altitude(),1,2,s)); //alt
    dataFile.print(",");
    dataFile.print(dtostrf(gps.f_course(),1,2,s)); //course
    dataFile.print(",");
    dataFile.println(dtostrf(gps.f_speed_kmph(),1,2,s)); // speed and last line
    dataFile.close();
  }  
  // if the file isn't open, write an error to serial
  else {
    Serial.println("error opening datalog.txt");
  } 

  // print statistics on the NMEA sentences.
  //unsigned long chars;
  //unsigned short sentences, failed_checksum;
  //gps.stats(&chars, &sentences, &failed_checksum);
}

