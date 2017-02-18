// Date and time functions using a DS1307 RTC connected via I2C and Wire lib
#include <Wire.h>
#include "RTClib.h"
#include <avr/sleep.h>
#include <TimeLib.h>

RTC_DS1307 RTC;

// this section lists the days of the week that will desplay in the serial console
char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

// One leg of the output relay should be attached to this pin.
// The other to the VCC Supply.
int TXRelay = 7;

// Trigger Pin for mp3 Sound
int Sound = 6;

// Trigger Pin for Wake Up Button
int wakePin = 2;

// Trigger Pin for Manual Transmit Button
int TXButton = 3;

// Trigger Blue Sleep Indicator
int SleepLED = 4;

// Trigger Green Operational Indicator
int onLED = 12;

// Check to see if Serial ready for incoming byte data
int inByte;

/* This will correct for drift.
    Enter the number of seconds the clock is off by per month.
    If the clock is fast, enter a negative number.  */
int drift = -60;

boolean AdjustTimeFlag = 1;

void setup () {

  Serial.begin(9600);
  // Display boot info - text can be found in the (Void)Info section at the bottom
  Info();
  Serial.println("");
  Serial.println("");
  delay(100);

  Wire.begin();
  RTC.begin();

  RTC.adjust(DateTime(F(__DATE__), F(__TIME__)));
  //   RTC.adjust(DateTime(2017, 2, 12, 19, 45, 0));

  //Check to see if RTC is running, if it is pull TIME and DATE from PC and apply to RTC
  if (! RTC.isrunning())
  {
    Serial.println("RTC is NOT running!");

  }

  // Check to see if RTC is plugged in
  if (! RTC.begin())
  {
    Serial.println("Couldn't find RTC");
  }

  // Set the Functions of the PINS
  pinMode(TXRelay, OUTPUT);
  pinMode(Sound, OUTPUT);
  pinMode(wakePin, INPUT);
  pinMode(TXButton, INPUT);
  pinMode(SleepLED, OUTPUT);
  pinMode(onLED, OUTPUT);

  digitalWrite(Sound, HIGH); // Set the Sound
  digitalWrite(TXRelay, HIGH); // Set the Relay
  digitalWrite(wakePin, HIGH); // Set Wake State Pin
  digitalWrite(TXButton, HIGH);
  digitalWrite(SleepLED, HIGH);
  digitalWrite(onLED, LOW);

}

void loop () {

  //Start Sleep Cycle when not in use
  sleepNow();

  //Pool TIME and DATE and load it
  DateTime now = RTC.now();

  // Display TIME and DATE in serial Monitor
  Serial.print(now.month(), DEC);
  Serial.print('/');
  Serial.print(now.day(), DEC);
  Serial.print('/');
  Serial.print(now.year(), DEC);
  Serial.print(" (");
  Serial.print(daysOfTheWeek[now.dayOfTheWeek()]);
  Serial.print(") ");
  Serial.print(now.hour(), DEC);
  Serial.print(':');
  Serial.print(now.minute(), DEC);
  Serial.print(':');
  Serial.print(now.second(), DEC);
  //Serial.println();

  // This section waits for input from the Serial Monitor to perform functions
  // This allows you to input a lower case "t" to turn the TX function on manually
  // Or input a lower case "i" to display the boot info

  if (Serial.available() > 0) {
    // get incoming byte:
    inByte = Serial.read();
    if (inByte == 't') {
      Serial.println("");
      Transmit();
    } else if (inByte == 'i') {
      Serial.println("");
      Info();
    }
  }
  
  // This sections sets minutes and seconds to start the timer
  // This is currently set to key the radio and play audio every 15 minutes past the hour
  if (now.minute() == 15 && now.second() == 0)

  {
    // this lets you know when the Transmitter is on when in Serial Console
    Serial.println(" ");
    Serial.println("TRANSMITTER ON ");
    Serial.print("");
    digitalWrite(TXRelay, LOW); // Keys up Radio
    digitalWrite(Sound, LOW);  // Turns MP3 board on to play message
    delay(100);  // waits 100 milliseconds
    digitalWrite(Sound, HIGH); // Releases the MP3 board do the message does not loop

  }

  // This sections sets minutes and seconds to stop the timer
  // This is currently set to un-key the radio and turn off the audio every 9 seconds past
  // the 15 minute start. This essentially gives you a 9 second timer.
  else  if (now.minute() == 15 && now.second() == 9)

  {
    digitalWrite(TXRelay, HIGH); // Un-Keys up Radio

    // this lets you know when the Transmitter is off when in Serial Console
    Serial.println(" ");
    Serial.println("TRANSMITTER OFF ");
    Serial.print("");
  }

  Serial.println(); // Prints a blank line inbetween TIME and DATE in Serial Console
  delay(1000);  // Deleays the desplay of each blank line for 1 second

  // This Turns on the manual transmit function when the TX button is pressed on the Radio Console
  if (digitalRead(TXButton) == LOW) {
    Transmit();
  }

  // This section controls the status LED
  if (digitalRead(wakePin) == LOW)
  {
    digitalWrite(SleepLED, HIGH);
    digitalWrite(onLED, LOW);

  } else {
    digitalWrite(SleepLED, LOW);
    digitalWrite(onLED, HIGH);

  }

  if ((now.unixtime() % 2629800L == 10800L) && (AdjustTimeFlag == 1)) {
    //once a month at 3am, correct the time
    DateTime AdjustTime (now.unixtime() + drift);
    RTC.adjust(AdjustTime);
    //and set a flag, to avoid adjusting the drift more than once
    AdjustTimeFlag = 0;
  }

  if (now.unixtime() % 604800L == 14400L) {
    //an hour later, reset the flag back to true
    AdjustTimeFlag = 1;
  }

}

void Transmit(void) {

  // this lets you know when the Transmitter is on
  Serial.println(" ");
  Serial.println("TRANSMITTER ON ");
  Serial.print("");

  digitalWrite(TXRelay, LOW); // Turns relay on to PTT the radio
  delay(300);  // waits 300 milliseconds
  digitalWrite(Sound, LOW);  // Turns MP3 board on to play message
  delay(100);  // waits 100 milliseconds
  digitalWrite(Sound, HIGH); // Releases the MP3 board do the message does not loop
  delay(8000);  //This delays the transmit function 8 seconds to allow the audio to finish playing before un-keying the radio
  digitalWrite(TXRelay, HIGH); //This un-keys the radio
  
  // this lets you know when the Transmitter is on
  Serial.println(" ");
  Serial.println("TRANSMITTER OFF ");
  Serial.print(""); 
}

void wakeUpNow() {
  // execute code here after wake-up before returning to the loop() function
  // timers and code using timers (serial.print and more...) will not work here.
  // we don't really need to execute any special functions here, since we
  // just want the thing to wake up
}

void sleepNow(void) {
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);   // sleep mode is set here
  sleep_enable();          // enables the sleep bit in the mcucr register
  attachInterrupt(0, wakeUpNow, LOW); // use interrupt 0 (pin 2) and run function
  sleep_mode();            // here the device is actually put to sleep!!

  // THE PROGRAM CONTINUES FROM HERE AFTER WAKING UP
  sleep_disable();         // first thing after waking from sleep: disable sleep...
  detachInterrupt(0);      // disables interrupt 0 on pin 2 so the wakeUpNow code will not be executed during normal running time.

}



void Info(void) {
  Serial.println("**********************************************************************************************");
  Serial.println("");
  Serial.println("                            CMD TECHNOLOGY  *  FIRMWARE  *  VERSION 4.0");
  Serial.println("");
  Serial.println("**********************************************************************************************");
  Serial.println("");
  Serial.println("Dispatch Automated Message Timer");
  Serial.println("");
  Serial.println("By Chris Dusseau - KE6FGY - CETsr motoapx1@gmail.com");
  Serial.println("");
  Serial.println("This will allow you to connect to a radio transmitter and play an automated");
  Serial.println("message at pre-programmed intervals.");
  Serial.println("");
  Serial.println("This is currently set to ");
  Serial.println("15 Minutes after the hour");
  Serial.println("9 Seconds on / TX time");
  Serial.println("");
  Serial.println("*^*^*^*^PRESS LOWER CASE 't' TO MANUALLY SET OFF THE TRANSMITTER^*^*^*^*");
  Serial.println("      *^*^*^*^PRESS LOWER CASE 'i' TO DISPLAY THE BOOT INFO^*^*^*^*");
  Serial.println("");
  Serial.println("This unit connects to a XTL5000 Consollette and to the MCC7500 AUX I/O");
  Serial.println("and is controlled by the MCC7500");
  Serial.println("");
  Serial.println("I have built this to suit my needs and as such is free for anyone else to modify or");
  Serial.println("use as needed. Just give credit where it is due.");
  Serial.println("");
  Serial.println(" 2/14/2017");
  Serial.println("*********************************************************************************************");
}


