
/* 
Original sketch from Paul Electronics 7-segment shift register posting
RTC libraries from http://jeelabs.net/projects/cafe/wiki/RTClib
Setting realtime clock on-compile was from Ladyada.net

Pin assignment on the 74595 to 7-segment common cathode as follows :-

QA - a
QB - b
QC - c
QD - d
QE - e
QF - f
QG - g

3 & 8 are common cathode
resistors is 220R

I just combine both the codes together and make it work...

Can add in temp sensors or a piezo for hourly chime

Stanley 
http://arduino-for-beginners.blogspot.com


*/

#include <Wire.h>
#include "RTClib.h"


const int  g_pinData = 10;
const int  g_pinCommLatch = 11;
const int  g_pinClock = 12;
const int ledPin = 13;
const int speakerPin = 9;



RTC_DS1307 RTC;

// Definitions of the 7-bit values for displaying digits
byte g_digits [10];

// Current number being displayed
int g_numberToDisplay = 0;

// Number of shift registers in use
const int g_registers = 4;

// Array of numbers to pass to shift registers
byte g_registerArray [g_registers];

void setup()
{

 Wire.begin();
 RTC.begin();
 

  
 // Only set the time on compile if the RTC is not running...
 if ( !RTC.isrunning()) {
   Serial.println("RTC is NOT running!"); }
   // following line sets the RTC to the date & time this sketch was compiled
   RTC.adjust(DateTime(F(__DATE__), F(__TIME__)));

       // This line sets the RTC with an explicit date & time, for example to set
    // January 21, 2014 at 3am you would call:
  //  RTC.adjust(DateTime(2019, 10, 17, 11,54, 00));   
  

 pinMode (g_pinCommLatch, OUTPUT);
 pinMode (g_pinClock, OUTPUT);
 pinMode (g_pinData, OUTPUT);
 pinMode (speakerPin, OUTPUT);
 pinMode (ledPin, OUTPUT);
 
 Serial.begin (9600);

 int a = 1, b = 2, c = 4, d = 8, e = 16, f = 32, g = 64;

 g_digits [0] = a + b + c + d +e + f;
 g_digits [1] = b + c;
 g_digits [2] = a + b + g + e + d;
 g_digits [3] = a + b + g + c + d;
 g_digits [4] = f + g + b + c;
 g_digits [5] = a + f + g + c + d;
 g_digits [6] = a + f + g + c + d + e;
 g_digits [7] = a + b + c;
 g_digits [8] = a + b + c + d + e + f + g;
 g_digits [9] = a + b + c + d + g + f;


} // setup

// Simple function to send serial data to one or more shift registers by iterating backwards through an array.
// Although g_registers exists, they may not all be being used, hence the input parameter.
void sendSerialData 
(
byte registerCount,  // How many shift registers?
byte *pValueArray)   // Array of bytes with LSByte in array [0]
{
 // Signal to the 595s to listen for data
 digitalWrite (g_pinCommLatch, LOW);

 for (byte reg = registerCount; reg > 0; reg--)
 {
   byte value = pValueArray [reg - 1];

   for (byte bitMask = 128; bitMask > 0; bitMask >>= 1)
   {
     digitalWrite (g_pinClock, LOW);
     digitalWrite (g_pinData, value & bitMask ? HIGH : LOW);
     digitalWrite (g_pinClock, HIGH);
   }
 }
 // Signal to the 595s that I'm done sending
 digitalWrite (g_pinCommLatch, HIGH);
}  // sendSerialData

// Print a message specifying valid inputs, given the number of registers defined and then consume all current input.
void badNumber ()
{
 int dummy;

 Serial.print ("Please enter a number from 0 to ");
 for (int loop = 0; loop < g_registers; loop++)
 {
   Serial.print ("9");
 }
 Serial.println (" inclusive.");

 while (Serial.available () > 0)
 {
   dummy = Serial.read ();
   delay (10);
 }
} //badNumber

// Read a number from the PC with no more digits than the defined number of registers.
// Returns: number to display. If an invalid number was read, the number returned is the current number being displayed
//
int readNumberFromPC ()
{
 byte incomingByte;
 int  numberRead;
 byte incomingCount;

 if (Serial.available () > 0)
 {
   numberRead = 0;
   incomingCount = 0;

   while (Serial.available () > 0)
   {
     incomingByte = Serial.read () - 48;
     incomingCount++;

     if (incomingByte < 0 || incomingByte > 9 || incomingCount > g_registers)
     {
       badNumber ();
       return g_numberToDisplay;
     }

     numberRead = 10 * numberRead + incomingByte;

     // Necessary to get all input in one go.
     delay (10);
   }

   Serial.print ("Now displaying: ");
   Serial.println (numberRead, DEC);

   return numberRead;
 }

 return g_numberToDisplay;
} // readNumberFromPC


void loop()
{
 int hour,minute,sec,disp = 0;

 DateTime now = RTC.now();

 hour = now.hour();
 minute = now.minute();
 sec = now.second();

// Serial output debugging for the date & time 
 
  Serial.print(now.year(), DEC);
  Serial.print('/');
  Serial.print(now.month(), DEC);
  Serial.print('/');
  Serial.print(now.day(), DEC);
  Serial.print(' ');

  Serial.print(hour);
  Serial.print(':');
  Serial.print(minute);
  Serial.print(':');    
  Serial.print(sec);
  Serial.println();
 
 
 // Original code
 //g_numberToDisplay = readNumberFromPC ();
 
 
 // Push the hour 2 digits to the left by multiplying 100
 disp = (hour * 100) + minute;
 g_numberToDisplay = disp;

 if (g_numberToDisplay < 10)
 {
   g_registerArray [0] = g_digits [0];
   g_registerArray [1] = g_digits [0];
   g_registerArray [2] = g_digits [0];
   g_registerArray [3] = g_digits [g_numberToDisplay];
 }
 else if (g_numberToDisplay < 100)
 {
   g_registerArray [0] = g_digits [0];
   g_registerArray [1] = g_digits [0];
   g_registerArray [2] = g_digits [g_numberToDisplay / 10];
   g_registerArray [3] = g_digits [g_numberToDisplay % 10];
 }
 else if (g_numberToDisplay < 1000)
 {
   g_registerArray [0] = g_digits [0];
   g_registerArray [1] = g_digits [g_numberToDisplay / 100];
   g_registerArray [2] = g_digits [(g_numberToDisplay % 100) / 10];
   g_registerArray [3] = g_digits [g_numberToDisplay % 10];
 }
 else
 {
   g_registerArray [0] = g_digits [g_numberToDisplay / 1000];
   g_registerArray [1] = g_digits [(g_numberToDisplay % 1000) / 100];
   g_registerArray [2] = g_digits [(g_numberToDisplay % 100) / 10];
   g_registerArray [3] = g_digits [g_numberToDisplay % 10];
 }

 sendSerialData (g_registers, g_registerArray);
 
 // Blink the LED to indicate seconds
 digitalWrite(ledPin,HIGH);
 delay(500);
 digitalWrite(ledPin,LOW);
 delay(500);

} // loop
