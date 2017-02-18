/******************************************************************************************
   Dispatch Automated Message Timer
   By Chris Dusseau - KE6FGY - CETsr motoapx1@gmail dot com
   This will allow you to connect to a radio transmitter and play an automated
   Message at pre-programmed intervals.

   This is currently set to
   60 Minutes off / RX time
   9 Seconds on / Length of message to play / TX time

   I have built this to suit my needs and as such is free for anyone else to modify or
   Use as needed. Just give credit where it is due.

   2/23/2016
 ******************************************************************************************/

  // These are the libraries needed
  #include <EEPROM.h>
  #include <avr/sleep.h>

  // One leg of the output relay should be attached to this pin.
  // The other to the VCC Supply.
  int TXRelay = 7;

  // Trigger Pin for mp3 sound
  int Sound = 6;

  // Trigger Pin sleep
  int wakePin = 2;

  // Trigger Pin sleep
  int TXButton = 3;

  // Trigger Blue Sleep Indicator
  int SleepLED = 4;

  // Trigger Green Operational Indicator
  int onLED = 12;

  // Check to see if Serial ready for incoming byte data
  int inByte;

  // I don't want to lose my interval settings when the Arduino loses power, so we use the Arduino's
  // built in EEPROM memory cache to store settings when they are changed. These set the memory addresses
  // for the off and on interval settings
  int EEPROM_OFF_INTERVAL_ADDRESS = 0;
  int EEPROM_ON_INTERVAL_ADDRESS = 1;

// These are the interval variables
long LastTX;
long OffTime;
long OnTime;


void setup(void) {

  //Start Serial
  Serial.begin(9600);

  // Enable this code and push program once on the first run of a new board or to move EEPROM registers.
  // Then disable and push the program to the board again for normal operation.
  // EEPROM.write(EEPROM_OFF_INTERVAL_ADDRESS, 1);
  // EEPROM.write(EEPROM_ON_INTERVAL_ADDRESS, 10);

  // This grabs the stored intervals from the EEPROM cache on startup -- use this for testing --
  // Comment (add the two slashes in front of each line) these two lines out when not testing and in normal operation
  OffTime = EEPROM.read(EEPROM_OFF_INTERVAL_ADDRESS) * 60000;
  OnTime = EEPROM.read(EEPROM_ON_INTERVAL_ADDRESS) * 900;

  // Uncomment (Remove the two slashes in front of each line) to run normal operation
  // OffTime = EEPROM.read(EEPROM_OFF_INTERVAL_ADDRESS) * 3600000;
  // OnTime = EEPROM.read(EEPROM_ON_INTERVAL_ADDRESS) * 900;

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

  // Display boot info - text can be found in the (Void)Info section at the bottom
  Info();

  // Initilize LastTX time buffer, This sets the buffer to this exact moment in time.
  LastTX = millis();

  // use interrupt 0 (pin 2) and run function wakeUpNow when pin 2 gets LOW
  // attachInterrupt(0, wakeUpNow, HIGH);
}
void loop(void) {

  sleepNow(); //Start Sleep Cycle when not in use

  // Write the DEBUG out to the Serial Monitor
  // Where time is displayed after the text, add spaces so the screen will clear out the text field
  // so things display properly. Add enough spaces to compensate for times that contain multiple digits

  Serial.println(" ");
  Serial.println("Relay Time");
  Serial.println(" ");
  Serial.print(" RX: ");
  Serial.print(OffTime / 1000 / 60);
  Serial.print("m ");
  Serial.println(" ");
  Serial.print(" TX: ");
  Serial.print(OnTime / 1000);
  Serial.print("s ");
  Serial.println(" ");
  Serial.print("Next TX: ");
  Serial.print(((LastTX + OffTime) -  millis()) / 1000 / 60);
  Serial.print("m ");
  Serial.print((((LastTX + OffTime) -  millis()) / 1000 ) % 60);
  Serial.print("s         ");
  Serial.println(" ");
  delay(1000);

  {
    if (LastTX + OffTime < millis()) {
      Transmit();
      LastTX = millis();
    }
  }

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
  if (digitalRead(TXButton) == LOW) { //SET back to low is using normally a open switch
    Transmit();
  }

  if (digitalRead(wakePin) == LOW) {
      digitalWrite(SleepLED, HIGH);
      digitalWrite(onLED, LOW);

  } else {
    digitalWrite(SleepLED, LOW);
    digitalWrite(onLED, HIGH);
  }

}
void Transmit(void)

{
  digitalWrite(TXRelay, LOW); // Turns relay on to PTT the radio
  delay(300);  // waits 300 milliseconds
  digitalWrite(Sound, LOW);  // Turns MP3 board on to play message
  delay(100);  // waits 100 milliseconds
  digitalWrite(Sound, HIGH); // Releases the MP3 board do the message does not loop

  long TXBuffer = millis() + OnTime;
  while (millis() < TXBuffer) {

    // this lets you know when the Transmitter is on
    Serial.println(" ");
    Serial.println("TRANSMITTER ON ");
    Serial.print("");

    // This lets you know how much longer the Transmitter will be on
    Serial.print((TXBuffer -  millis()) / 1000 );
    Serial.print("s        ");
    Serial.println("");
    delay(1000); // waits 1000 milliseconds / 1 second
  }

  digitalWrite(TXRelay, HIGH); // this turns the relay off and the Transmitter

}

// This is the boot message
void Info(void)

{
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
  Serial.println("60 Minutes off / RX time");
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
  Serial.println(" 7/23/2016");
  Serial.println("**********************************************************************************************");
  Serial.println("");
  Serial.println("                            DURING DEBUGGING");
  Serial.println("WE MAY SKIP 1 SECOND DO TO TIMMING AND TIME TO DISPLAY AND THAT IS OK!!!");

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
// END OF LINE

