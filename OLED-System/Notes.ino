/*
   Basic Plan


  A person decides to use a machine.
  They switch in the plug.
  The system boots and tries to connect.
  (If all good then go to the next step, otherwise report the problem but allow a user with a master key to still use the tool. A master key is just their rfid card which has an extra piece of data stored in addition to its unique identifier. This could be a simple bit.)
  The system says “Please scan your card” or something similar.
  The person taps their rfid card of tag on the reader.
  The reader scans the tag and checks if the person is allowed to use the machine.

  https://enrichment.longridgetowers.com/dt/authorised.php?Machine_Key=211&User_Key=1237

  If not, a message such as “Sorry, you are not authorised to use this machine”.
  The system then waits a few seconds and jumps back to step 5.

  Authorised use

  The system says “Name of operator” and perhaps “please take care”.
  The operator uses the machine.
  The system takes a current measurement to determine if the machine is in use.
  Once the machine stops, the current drops to zero.
  The system runs a time out, for say 1 minute.
  If the current detector notes that the machine is in use, then we jump back to step 3.
  If the timer runs out, then the system jumps back to step 5 in the authorisation stage.


  Detailed Description:

  Steps 1 & 2 are self explanatory.

  3. Bootup and connection

  The system has to initialise the WiFi, the RFID card read and the OLED screen.

  It seems sensible to enable the OLED first, so that it can display information:

  Booting up…

  Initialising RFID Reader…. (Hopefully all ok, if not then we have a big problem!)

  Connecting to WiFi…. (Just check that we have a connection, otherwise say so)

  Connecting to the Internet … (Check that we can access a test file at a specific address - perhaps this file could be used for updating purposes as required)



  4. Master Key
  The two demo files rfid_red_personal_data and rfid_write_personal_data can be used to create a master key and read the data. These files ask for a family name, but we coil adjust as easily write the words “master key” onto the RFID card.

*/
