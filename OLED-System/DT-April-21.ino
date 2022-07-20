/*************************************************************************

   DT Safety System - 29-3-21

   This system requires:


          ESP32 OLED - https://www.google.com/search?q=esp32+oled+pinout&tbm=isch&ved=2ahUKEwjK6439udXvAhUOBGMBHRwNDhMQ2-cCegQIABAA&oq=esp32+oled+pinout&gs_lcp=CgNpbWcQA1AAWABgqBBoAHAAeACAAQCIAQCSAQCYAQCqAQtnd3Mtd2l6LWltZw&sclient=img&ei=SsFhYMqeII6IjLsPnJq4mAE&bih=625&biw=1366&rlz=1C1AWFA_enGB745GB745&safe=strict#imgrc=aG-Jxn8LpU867M

          RFID Scanner - https://microcontrollerslab.com/wp-content/uploads/2020/01/RC522-RFID-CARD-READERS-Pinout.png

          Voltage level shifter - https://www.ebay.com/itm/5x-4-Channel-Bi-Directional-Logic-Level-Converter-Shifter-5V-3-3V-For-Arduino-/223622889601

          Arduino Nano - https://892962.smushcdn.com/2087382/wp-content/uploads/2019/08/Arduino-Nano-Pinout-1.png?lossy=1&strip=1&webp=1

          Current Sensor - https://www.theengineeringprojects.com/wp-content/uploads/2017/05/DC-Current-Sensor-ACS712-Arduino-Interfacing.png

          Double Relay - https://i0.wp.com/iothyngs.com/wp-content/uploads/2016/01/image-2.jpg?fit=500%2C475&ssl=1

          Standard 2 socket box and blanking plate

          3d printed scanner box & back

          4 way wire to connect the boxes - LED strip wire - https://www.amazon.co.uk/Extension-Cable-Wire-Strip-Ribbon/dp/B00JPZK934/ref=pd_lpo_23_t_1/260-9866615-4138131?_encoding=UTF8&pd_rd_i=B00JPZK934&pd_rd_r=c26bd44d-8932-4fdd-8d5d-93de6080ff9d&pd_rd_w=k2RZV&pd_rd_wg=GpSBC&pf_rd_p=d8b3a5b8-7766-4dff-b57a-24ee5d46250d&pf_rd_r=WJ88X4M95JWYTDR59G0P&psc=1&refRID=WJ88X4M95JWYTDR59G0P

          2 x 13A trailing sockets

          1 x 13A plug

          16A PVC cable
          

          Voltage level shifter needed to connect Arduin Nano (5v) to ESP32 (3.3v)
          ========================================================================

          GND - Black Ground (ESP32 - GND Pin 2
          LV - Orange - 3.3v (ESP32 - 3v3 Pin 5)
          HV - RED - 5v (ESP32 - 5v Pin 1)

          Soft Serial

          TX0 - Low voltage RX to Nextion from ESP8266 D4 Pin D2
          RX0 - Low voltage TX from Nextion to  ESP8266 D5 Pin D1



          RFID-RC522
          ==========

          SDA     -     D2
          SCK     -     D5
          MOSI    -     D7
          MISO    -     D6
          IRQ - N/A
          GND     -     GND
          RST     -
          3.3v    -     3.3v
*/
//

// Libraries used...

/*************************************************************************
   This is for the ESP32 with built in OLED screen and RFID reader attached
  NB - Change C:\Users\Mr Westthorp\AppData\Local\Arduino15\packages\esp32\hardware\esp32\1.0.3\variants\lolin32\pins_arduino.h to the old copy when done!

  Wiring:

  ESP32-OLED    RFID-RC522
  3v3           3v3
  GND           GND
  GPIO 25       SDA //Swapped
  GPIO 26       RST //Swapped
  GPIO 14       CLK
  GPIO 13       MOSI
  GPIO 12       MISO

*/


#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include <SPI.h>
#include <MFRC522.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <HTTPClient.h>
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

#define USE_SERIAL Serial
#define SS_PIN 25 //Hard wired White / yellow 
#define RST_PIN 26 //
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance.


// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// Which way up? - 0 (Desk mount) or 2 (Wall mount)
int rotation = 0;
// How big do you like your text? 1-8
int text_size = 2;
String User_Key_Value = "";
// Scanner 1 - Sign In, 2 = Sign Out, 3 = Matron
String Scanner="DT-1";
bool override = false;


// Set up Machine value - This will need to be hard coded!
String Machine_Key_Value = "211"; //211 = Small bandsaw


// A couple of contstants

const char* ssid = "LTS_CDT";
const char* password =  "ProCurve1800-24G";
const char* connection_test = "http://enrichment.longridgetowers.com/dt/connection_test.php?Scanner=";
String check_in = "http://enrichment.longridgetowers.com/scan_rfid/check_in.php?Scanner=" + Scanner + "&IP=";
unsigned long previousMillis = 0;        // will store last time the WiFi was active
const long interval = 5 * 60000;           // WiFi connection interval for pinging (every 5 minutes)

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

  // Port defaults to 3232
  // ArduinoOTA.setPort(3232);

  // Hostname defaults to esp3232-[MAC]
  ArduinoOTA.setHostname("RFID-8");

  // Password "iamgroot" hashed
  // ArduinoOTA.setPassword("plaintext");

  // Password can be set with it's md5 value as well
  // MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
  ArduinoOTA.setPasswordHash("473c0812623754d187d1e4c96af5d5cb");

  ArduinoOTA
    .onStart([]() {
      String type;
      if (ArduinoOTA.getCommand() == U_FLASH)
        type = "sketch";
      else // U_SPIFFS
        type = "filesystem";

      // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
      Serial.println("Start updating " + type);
    })
    .onEnd([]() {
      Serial.println("\nEnd");
    })
    .onProgress([](unsigned int progress, unsigned int total) {
      Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    })
    .onError([](ota_error_t error) {
      Serial.printf("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
      else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
      else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
      else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
      else if (error == OTA_END_ERROR) Serial.println("End Failed");
    });

  ArduinoOTA.begin();

  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  
  Wire.begin(5, 4);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C, false, false)) {
    // Address 0x3C for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);

    Serial.println("Ready!");
  }
  // Clear the buffer.
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(text_size);
  display.setCursor(0, 10);
  display.print("Checking  RFID");
  display.display();
  Serial.println("Setting up RFID...");
  SPI.begin();      // Initiate  SPI bus
  mfrc522.PCD_Init();   // Initiate MFRC522

  display.clearDisplay();

delay(1000);
  
  display.setCursor(0, 10);
  display.print("Checking  WiFi");
  display.display();
delay(1000);
  Serial.println(WiFi.localIP());
  check_in = check_in + WiFi.localIP().toString();
  Serial.println("Address = " + check_in);
  display.print("Address = " + check_in);
  display.display();
delay(1000);
}

int value = 0;

void loop() {

  ArduinoOTA.handle();

  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {
    // save the last time you blinked the LED
    previousMillis = currentMillis;
    check_in_to_keep_WiFi(check_in);

  }

  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(text_size);
  display.setCursor(0, 10);
  display.print("Please tapyour tag");

  display.display();





  // Look for new cards
  if ( ! mfrc522.PICC_IsNewCardPresent())
  {
    return;
  }
  // Select one of the cards
  if ( ! mfrc522.PICC_ReadCardSerial())
  {
    return;
  }
  //Show UID on serial monitor
  Serial.println();
  Serial.print(" UID tag :");
  String content = "";
  byte letter;
  for (byte i = 0; i < mfrc522.uid.size; i++)
  {
    Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
    Serial.print(mfrc522.uid.uidByte[i], HEX);
    content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
    content.concat(String(mfrc522.uid.uidByte[i], HEX));
  }
  content.toUpperCase();
  Serial.println();
  // User_Key_Value = content.substring(1);
  // Remove any white spaces..
  User_Key_Value = content;
  User_Key_Value.replace(" ", "");

  Serial.println(User_Key_Value);

  //previousMillis = currentMillis;
  check_with_server();
}

void check_in_to_keep_WiFi(String request)
{
  if ((WiFi.status() == WL_CONNECTED)) { //Check the current connection status

    HTTPClient http;

    http.begin(request); // Send the message

    int httpCode = http.GET();

    if (httpCode > 0) { //Check for the returning code

      Serial.println(httpCode);
    }

    else {
      Serial.println(httpCode);
      Serial.println("Error on HTTP request");
    }

    http.end(); //Free the resources
  }
}
void check_with_server() {

  String request = String(host) + String(User_Key_Value) + "&Scanner=" + String(Scanner);
  Serial.println(request);

  if ((WiFi.status() == WL_CONNECTED)) { //Check the current connection status

    HTTPClient http;

    http.begin(request); // Send the message

    int httpCode = http.GET();

    if (httpCode > 0) { //Check for the returning code

      String payload = http.getString();
      Serial.println(httpCode);
      Serial.println(payload);
      What_shall_we_do_with_the(payload);
    }

    else {
      Serial.println(httpCode);
      Serial.println("Error on HTTP request");
    }

    http.end(); //Free the resources
  }
}

void What_shall_we_do_with_the(String payload)
{

  /* So the payload can be anything,

      At the moment if the RFID has not been recognised, then you get:

      typically it could take the JSON form:

      {"Pupil":"Lucy Westthorp", "Scanned at ":"2020/01/30 10:06:55"}

      We could then parse this first string

  */

  display.clearDisplay();
  display.setTextSize(text_size);

  display.setTextColor(WHITE);
  display.setCursor(0, 10);

  if (payload.indexOf("Invalid") > 0) {

    display.print("Card Invalid");

  }
  else
  {

    // Tidy up the payload

    //display.println("Thank You: ");
    display.println(payload);
    display.display();
    Serial.print(payload);
    delay(2000);
  }
}



void setup_everything() {

  Serial.begin(9600);

  Serial.println("Machine Authorisation Record System");
  Serial.println("===================================");
  Serial.println();
  Serial.println("Version 1");
  Serial.println();


  //Setting up RFID...
  Serial.println("Checking RFID");
  tRFIDstatus.setText("Checking...");
  SPI.begin();      // Initiate  SPI bus
  mfrc522.PCD_Init();   // Initiate MFRC522
  // Prepare the security key for the read and write functions.
  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;  //keyByte is defined in the "MIFARE_Key" 'struct' definition in the .h file of the library
  }

  if (test_reader()) {
    tRFIDstatus.setText("Working");
  }
  else
  {
    tRFIDstatus.setText("Problem");
    // We should halt now!
  }
 
  delay(1000);

  // Connecting to WiFi -   WiFi.begin(ssid, password);

  Serial.println("Checking WiFi");
  tWiFistatus.setText("Checking...");
  WiFi.mode(WIFI_STA);
  if (WiFiMulti.addAP(ssid, password)) {

    // Not sure that this works?
    Serial.println("WiFi connected to ");
    Serial.print(ssid);
    tWiFistatus.setText("Connected");
    override = false;
  } else
  {
    Serial.println("WiFi not connecting to ");
    Serial.print(ssid);
    Serial.println("We shall activate overide mode, so we can at least use the machine.");
    tWiFistatus.setText("Problem with WiFi");
    override = true;
  }

  if (!override) {


    delay(1000);
    // Connect to Internet - do a GET of the special initialise.php file to check working.
    tNetstatus.setText("Checking...");

    if (connection_to_server()) {

      Serial.println("Connected to the server ok! ");
      tNetstatus.setText("Connected");

      override = false;
    } else
    {
      Serial.println("We shall activate override mode, so we can at least use the machine.");
      tNetstatus.setText("Unreachable");
      override = true;
    }
  }
  if (override) {
    Serial.println("Manual override in operation");
  }
  else
  {
    Serial.println("All systems go!");

  }

  scanning_screen.show();
  Serial.println("Scanner-Ready");

  // Not sure why, but it seems to like to re-initialise the scanner
  SPI.begin();      // Initiate  SPI bus
  mfrc522.PCD_Init();   // Initiate MFRC522


}

void  display_authorised(String user_name)
{
  ppe_screen.show();

  tUserName.setText("Authorised");
  //tUserName.setText(String user_name);
}
