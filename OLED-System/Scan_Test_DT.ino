/*************************************************************************
  This should connect to the DT site and scan a card
*/


String User_Key_Value = "";
String return_key = "";
String user = "Blank";
String authorised = "0";

float threshold = 0.2; // The current drawn by the machine - if less than threshold then we think machine is off - may need to adjust!
long time_out = 20000; // The time allowed to use the machine

// A couple of contstants

const char* host = "http://enrichment.longridgetowers.com/dt/";
String authorise = "new_auth.php?Machine_Key=211&User_Key=";
String record_end = "update_end_time.php?Key=";


//    The boolean function "connection_to_server" returns true or false if we get a response from a web page

bool connection_to_server() {

  String payload = "Not Connected";
  bool connection_status;
  connection_status = false;
  Serial.println("Testing connection with Server");
  Serial.println("==============================");
  Serial.println("");
  Serial.println("Just checking that we can connect to:");
  String request = String(connection_test) + String(Scanner);
  Serial.println(request);

  if ((WiFi.status() == WL_CONNECTED)) { //Check the current connection status

    HTTPClient http;

    http.begin(request); // Send the message

    int httpCode = http.GET();

    if (httpCode > 0) { //Check for the returning code

      payload = http.getString();

      Serial.println(httpCode);
      Serial.println(payload);
      if (payload == "OK") {
        connection_status = true;
        Serial.println("Connection Good");
      }
      else
      {
        connection_status = false;
        Serial.println("Connection Bad");
      }

    }

    else {
      Serial.println(httpCode);
      Serial.println("Error on HTTP request");
      connection_status = false;
    }

    http.end(); //Free the resources
    return connection_status;
  }
}

// The scan_loop scans the card and checks the database
int value = 0;

void scan() {

  Serial.println("scan - called");
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


  check_with_server();
}

// Here we need to check if the person is authorised to use the machine.

void check_with_server() {

  String request = String(host) + authorise + String(User_Key_Value);
  Serial.println(request);

  //request = "http://enrichment.longridgetowers.com/dt/new_auth.php?Machine_Key=211&User_Key=9A571D28";

  if ((WiFi.status() == WL_CONNECTED)) { //Check the current connection status

    HTTPClient http;

    http.begin(request); // Send the message

    int httpCode = http.GET();

    if (httpCode > 0) { //Check for the returning code

      String payload = http.getString();
      Serial.println("HTTP Code: ");
      Serial.print(httpCode);
      Serial.println("Response: ");
      Serial.print(payload);
      Serial.println("End of payload");
      What_shall_we_do_with_the(payload);
    }

    else {
      Serial.println(httpCode);
      Serial.println("Error on HTTP request");
    }

    http.end(); //Free the resources
  }
}

// Checking card details and extracting information about the user and the return key

void What_shall_we_do_with_the(String payload)
{

  if (payload.indexOf("valid") > 0) {

    Serial.println("Card Invalid");
    Serial.println("Scan again...");
    //Display a message to say - no chance mate!
    not_authorised_screen.show();
    delay(2000);
    // Now display the scanning screen again
    scanning_screen.show();

  }
  else
  {
    Serial.println("Deconstruct the payload");
    Serial.println(payload);

    // Copy the name up to the first delimiter which is a ,
    user = payload.substring(0, payload.indexOf(","));
    Serial.println("User : ");
    Serial.println(user);


    // Authority?
    authorised = payload.substring(payload.indexOf(",") + 1, payload.indexOf("&"));
    Serial.println("Authorised : ");
    Serial.println(authorised);

    // If the [person has been authorised, they will have a return key

    if (authorised == "1") {


      // Display on screen - would be nice to have the name of the person on screen - but doesn't seem to like variables!
      ppe_screen.show();
      tUserName.setText("Authorised");
      // Keyfield
      return_key = payload.substring(payload.indexOf("&") + 1, payload.length());
      Serial.println("Key : ");
      Serial.println(return_key);
/*
 * The plan:
 * 
 * 1.Give the user 1 minute to turn on the machine
 * 
 *    If they do that, then wait until they turn it off again
 *    Then Jump back to step 1.
 *    
 * 2.After 1 minute of inactivity, switch off relay as they have finished.
 */

      // Now use the machine - we need to poll the current sensor to see when they have stopped using it.

      Serial.println("Relay on");
      unsigned long start = millis ();
      boolean powered_up= false;
      while ((millis () - start <= time_out) && !powered_up)   // for 60 seconds
      {
        Serial.println("Check if machine is on");
        Serial.println("Time remaining : ");
        Serial.print(time_out - (millis () - start));
        take_reading();
        if (Amps_TRMS > threshold) {
          Serial.println("Machine On");
          powered_up = true;
          // Loop until they have turned the machine off
          while (powered_up)
          {
            delay(1000); // Wait for a second to avoid lock up
            take_reading();
            if (Amps_TRMS < threshold) {
              powered_up = false;
              Serial.println("Machine turned off");
            }
            
          }
          // Reset start time
          start = millis ();
        }
        else
        {
          Serial.println("Off");
        }
        delay(1000);
      }

      Serial.println("End of use, turn relay off");
      
      update_end_time(return_key.toInt());

      tidy_screen.show();
      delay(3000);

    }
    else
    {

      //Display a message to say - no chance mate!
      not_authorised_screen.show();
      delay(2000);

    }
    // Now display the scanning screen again
    scanning_screen.show();


  }
}

// This routine updates the end_time field for a given record - signifiying that the machine has been used.

void update_end_time(int key_field)
{

  String request = String(host) + record_end + String(key_field);

  Serial.println(request);

  if ((WiFi.status() == WL_CONNECTED)) { //Check the current connection status

    HTTPClient http;

    http.begin(request); // Send the message

    int httpCode = http.GET();

    if (httpCode > 0) { //Check for the returning code

      String payload = http.getString();
      Serial.println("HTTP Code: ");
      Serial.print(httpCode);
      Serial.println("Response: ");
      Serial.print(payload);
      Serial.println("End of payload");
    }

    else {
      Serial.println(httpCode);
      Serial.println("Error on HTTP request");
    }

    http.end(); //Free the resources
  }
}
