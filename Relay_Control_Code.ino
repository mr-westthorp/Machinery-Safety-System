
/*
   Relay board control code

   This takes two inputs:

   Cycle_Start - From the User panel - pulled LOW to switch on the machine's relay and start counting down
   Machine_Status - From the current sensor - Currently employing a D1Mini - seems wasteful but couldn't get the current sesning code to work on this ESp12 chip

   Output

   Cycle_Complete - Held HIGH until time is up, then pulled LOW to let user panel know that the machine is finished with. Hence - tidy up!

*/

unsigned long startMillis;  //some global variables available anywhere in the program
unsigned long currentMillis;
const unsigned long period = 1000;  //the value is a number of milliseconds


// OTA Code from the basic example

#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

#ifndef STASSID
#define STASSID "LTS_CDT"
#define STAPSK  "ProCurve1800-24G"
#endif


const char* ssid = STASSID;
const char* password = STAPSK;



// Input Lines
#define cycle_start             12          // GPIO Line from User Panel (Active LOW)
#define machine_status          5          // GPIO Line from D1-Mini - HIGH= ON, LOW = OFF - When machine is drawing current, this will go HIGH

// Output Devices
#define device_relay            16         // GPIO Line on PCB with short to RLY1 
#define dust_extractor_relay    14         // GPIO Line on PCB with short to RLY2
#define cycle_status            13          // GPIO Line to User Panel (Active LOW)

// Starting conditions
int max_time = 30;
int seconds_remaining = max_time;  // Set countdown timer

String machine_in_use = "No";
String previous_machine_in_use = "No";

boolean Cycle_Complete = true;


void setup() {
  Serial.begin(115200);
  Serial.println("Booting");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Connection Failed! Rebooting...");
    delay(5000);
    ESP.restart();
  }

  // Port defaults to 8266
  // ArduinoOTA.setPort(8266);

  // Hostname defaults to esp8266-[ChipID]
  ArduinoOTA.setHostname("30A-RelayPCB-8266");

  // Password "iamgroot" hashed
  // ArduinoOTA.setPassword("plaintext");

  // Password can be set with it's md5 value as well
  // MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
  ArduinoOTA.setPasswordHash("473c0812623754d187d1e4c96af5d5cb");

  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else { // U_FS
      type = "filesystem";
    }

    // NOTE: if updating FS this would be the place to unmount FS using FS.end()
    Serial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      Serial.println("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      Serial.println("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      Serial.println("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      Serial.println("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      Serial.println("End Failed");
    }
  });
  ArduinoOTA.begin();
  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  Serial.println("Machine control board");
  Serial.println("=====================");


  // Set up relays
  pinMode(device_relay, OUTPUT);
  pinMode(dust_extractor_relay, OUTPUT);
  pinMode(machine_status, INPUT);

  // Set up control lines - if not using serial comms
  pinMode(cycle_start, INPUT_PULLUP); // - Pull LOW starts the cycle
  pinMode(cycle_status, OUTPUT); // - Normally HIGH, goes LOW to indicate cycle in operation, then goes HIGH at end


  // Switch both OFF to start with
  digitalWrite(device_relay, LOW);   // turn OFF device relay
  digitalWrite(dust_extractor_relay, LOW);   // turn OFF device relay

  machine_in_use = "No";
  previous_machine_in_use = "No";
  seconds_remaining = max_time;
  digitalWrite(cycle_status, LOW);
  startMillis = millis();  //initial start time
}




void loop()
{

  /*  The basic plan:
      =================

      A user scans their card and the User Panel pulls the cycle_start line LOW

      So we need to check to see cycle_start has been pulled LOW

      If Machine is not already in running state, then activate Relay to turn on power and start countdown.


      Check to see if machine_status is HIGH

      If so, then machine is in use, so turn on extractor Relay and reset countdown timer.

      If cycle_start is LOW and machine is in run state, check on the count down timer
  */

  ArduinoOTA.handle();


  // Check if machine is turned on - If so then turn on dust extractor and reset countdown timer

  if (digitalRead(machine_status) == 1) {

    //    Serial.println("Current Sensor: Machine is drawing power");
    machine_in_use = "Yes";
    digitalWrite(dust_extractor_relay, HIGH);   // turn on extraction relay
    seconds_remaining = max_time;
  }
  else
  {

    //    Serial.println("Current Sensor: Machine is off");
    machine_in_use = "No";
    // Could have a run on time for the extractor here?
    digitalWrite(dust_extractor_relay, LOW);   // turn off extraction relay

  }

  if (machine_in_use != previous_machine_in_use) {
    Serial.println("Machine in use : " + machine_in_use);
    previous_machine_in_use = machine_in_use;
  }


  // Ready to start?

  if (digitalRead(cycle_start) == 0) {


    Serial.println("************************");
    Serial.println("Cycle_Start : Pulled LOW");
    Serial.println("Machine Relay : ON");
    Serial.println("************************");

    digitalWrite(device_relay, HIGH);   // turn on device relay
    digitalWrite(cycle_status, HIGH);   // let user panel know that cycle is not complete.

    // Set countdown timer to MAX value and start coountdown.
    seconds_remaining = max_time;
    Cycle_Complete = false;


  }

  else
  {

    currentMillis = millis();  //get the current "time" (actually the number of milliseconds since the program started)

    if (currentMillis - startMillis >= period)  //test whether a second is up...
    {


    Serial.println("Wait for 1 second");
      if (seconds_remaining > 0 ) {
        
        seconds_remaining =   seconds_remaining - 1;
      }
      else if (seconds_remaining <= 1)
      {
        seconds_remaining = 0;
      }

      Serial.println("Countdown : ");
      Serial.print(seconds_remaining);
      Serial.println("");

      startMillis = currentMillis;  


    }


    // Are we finished?

    if (seconds_remaining == 0) {

    if (Cycle_Complete==false){
      Serial.println("Cycle Complete");

      digitalWrite(device_relay, LOW);   // turn OFF device relay
      digitalWrite(dust_extractor_relay, LOW);   // turn off extraction relay
      Cycle_Complete = true;
      digitalWrite(cycle_status, LOW);
    }
    
    }




  }

}
