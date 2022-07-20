/*This code should set up the Arduino Nano as a slave device for the main system
   The nano controls the relays for the machine and the LVE
   The nano also measures the current drawn by the machine

   Basic operation

  RFID-Master sends

  "Activate" - "A" - or Pin 12 LOW?


  When "A" received, machine relay is turned on and waiting for use
  Can do a countdown timer for 1 minute

  Send "In Use" back to RFID-Master - or Pull PIN 11 LOW?

  If the machine is used, then reset the timer
  If the machine is not used and timer runs out, then turn off machine

  If machine is used then wait another minute for countdown.

  At end - Send "Complete" or PUT Pin 11 HIGH


*/

#include <Filters.h>            //This library does a huge work check its .cpp file

#define ACS_Pin A0

int max_time = 5;  // Number of seconds to wait for countdown.
boolean Cycle_Complete = true;
float testFrequency = 50;                     // test signal frequency (Hz)
float windowLength = 40.0 / testFrequency;   // how long to average the signal, for statistist

float intercept = 0; // to be adjusted based on calibration testing
float slope = 0.0752; // to be adjusted based on calibration testing
//Please check the ACS712 Tutorial video by SurtrTech to see how to get them because it depends on your sensor, or look below

float Amps_TRMS;
float ACS_Value;

unsigned long myTime;

unsigned long printPeriod = 1000;
unsigned long previousMillis = 0;
RunningStatistics inputStats;                 // create statistics to look at the raw test signal

#define device_relay 5
#define  LVE_relay 4
#define cycle_start 7     // Pull LOW to start cycle
#define cycle_status 8       // Normally HIGH but pulls LOW during cycle and goes HIGH at end

// Tri Colour LED - to indicate status
// Traffic lights style
// RED = Power On - Machine OFF
// AMBER = Machine Ready - just press the start button
// GREEN = Machine in use

#define red_LED 9
#define green_LED 10
#define blue_LED 11






int seconds_remaining = 60; // Set countdown timer
int inByte = 0;         // incoming serial byte
String machine_in_use = "No";
String previous_machine_in_use = "No";

void setup()
{
  Serial.begin(9600);


  // Set up relays
  pinMode(device_relay, OUTPUT);
  pinMode(LVE_relay, OUTPUT);

  // Set up control lines - if not using serial comms
  pinMode(cycle_start, INPUT_PULLUP); // - Pull LOW starts the cycle
  pinMode(cycle_status, OUTPUT); // - Nonrmally HIGH, goes LOW to indicate cycle in operation, then goes HIGH at end

  // Set up LED PWM Control
  pinMode(red_LED, OUTPUT);
  pinMode(green_LED, OUTPUT);
  pinMode(blue_LED, OUTPUT);


  // Switch both OFF to start with
  digitalWrite(device_relay, HIGH);   // turn OFF device relay
  digitalWrite(LVE_relay, HIGH);   // turn OFF device relay

  //  Serial.println("RunningStatistics inputStats;");
  inputStats.setWindowSecs( windowLength );

  machine_in_use = "No";
  previous_machine_in_use = "No";
  seconds_remaining = max_time;
digitalWrite(cycle_status, HIGH); 
  RGB_color(255, 0, 0); // Red

}

void RGB_color(int red_light_value, int green_light_value, int blue_light_value)
{
  analogWrite(red_LED, red_light_value);
  analogWrite(green_LED, green_light_value);
  analogWrite(blue_LED, blue_light_value);
}



void loop()
{
  ACS_Value = analogRead(ACS_Pin);  // read the analog in value:

  inputStats.input(ACS_Value);  // log to Stats function

  if ((unsigned long)(millis() - previousMillis) >= printPeriod) { //Do the calculations every 1s
    previousMillis = millis();   // update time

    Amps_TRMS = intercept + slope * inputStats.sigma();

    if (Amps_TRMS > 3) {
      //Turn on LVE
      digitalWrite(LVE_relay, LOW);   // turn on extraction relay
      machine_in_use = "Yes";
      // Reset Countdown time to MAX value
      seconds_remaining = max_time;

      // Indicate on traffic light LED
      RGB_color(0, 255, 0); // Green


    }
    else
    {
      //Turn off LVE
      digitalWrite(LVE_relay, HIGH);   // turn off extraction relay
      machine_in_use = "No";
      if (seconds_remaining > 0 ) {
        seconds_remaining =   seconds_remaining - 1;
      }

    }

if (digitalRead(cycle_start) == 0){
  Serial.println("Pulled LOW");
  Serial.println("Machine Relay ON");
        digitalWrite(device_relay, LOW);   // turn on device relay

        // Set countdown timer to MAX value and start coountdown.
        seconds_remaining = max_time;
        Cycle_Complete = false;

        // Pull Cycle_Status line LOW
  digitalWrite(cycle_status, LOW);   // turn on device relay


        // Set RGB LED accordingly

        RGB_color(255, 200, 0); // Amber

}

    if (Serial.available() > 0) {
      // get incoming byte - e.g ASCII - A=65, etc.
      inByte = Serial.read();

      //   Serial.println(inByte);

      if (inByte == 65)
      {
        Serial.println("Machine Relay ON");
        digitalWrite(device_relay, LOW);   // turn on device relay

        // Set countdown timer to MAX value and start coountdown.
        seconds_remaining = max_time;
        Cycle_Complete = false;

        // Pull Cycle_Status line LOW
digitalWrite(cycle_status, LOW);   // turn on device relay


        // Set RGB LED accordingly

        RGB_color(255, 200, 0); // Amber

      }

      if (inByte == 66) // Not sure if needed
      {
        //    Serial.println("Machine Relay OFF");
        digitalWrite(device_relay, HIGH);   // turn OFF device relay

      }

    }

    if (machine_in_use != previous_machine_in_use) {
      Serial.println("Machine in use : " + machine_in_use);
      previous_machine_in_use = machine_in_use;
    }

    if (seconds_remaining > 0)
    {

      //    Serial.println(seconds_remaining);
    }

    else if (seconds_remaining <= 1)
    {
      seconds_remaining = 0;
    }

    if (seconds_remaining == 0) {
      // Out of time, so tell ESP32

      // unless we have alreday done so
      if (Cycle_Complete == false)
      {
        Serial.println("Cycle Complete");

        digitalWrite(device_relay, HIGH);   // turn OFF device relay
        digitalWrite(LVE_relay, HIGH);   // turn off extraction relay
        Cycle_Complete = true;
digitalWrite(cycle_status, HIGH); 

        RGB_color(255, 0, 0); // Red

      }



    }

  }

}
