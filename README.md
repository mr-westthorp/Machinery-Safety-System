RFID-based machine safety system


As a design technology teacher the most 
important factor in any design technology 
workshop is keeping the children safe. 

In some cases this means that certain tools 
and machinery are declared “teacher only” 
whilst other tools may be used with training 
and supervision. 

I firmly believe that children should be taught 
how to use the various tools and pieces of 
machinery in a safe and competent fashion. 

How can we ensure that we can restrict access 
to the machinery so that only those students 
who have passed some sort of competency training
 and assessment can use them?

When pupils leave the school and move onto the 
next phase, it might be nice for them to have a 
record of their competencies.

Another aspect of running the DT department 
relates to keeping all of the machines in 
working order, so that they can be used as 
needed. Naturally, things will wear out, 
such as sanding discs or bandsaw blades, etc. 
The dust extraction system will need to be 
safely cleaned and filters will need to be 
replaced on a fairly regular basis. 

Typically, we would schedule maintenance 
operations on a regular basis, for example, 
a couple of weeks. However, for a variety of 
simple reasons, it is possible that maintenance 
operations may be missed. 

Wouldn’t it be handy if, just as with your car,
 a service indicator could alert you to the fact 
that a machine needs some form of maintenance? If we knew how many hours a machine had been used or how many operations had been carried out using the machine, then we might have a better idea of when certain components will need to be replaced. 

Replacement parts and consumables could be 
ordered in advance, so that there will be no 
significant interruptions.


How does it work?

The basic principle is quite simple, 
but to help explain it I have divided it into 
the following steps:

Each pupil is issued with an RFID card or key tag. Many schools may already employ some form of RFID system for their attendance or lunch payment, so the pupils can use their existing tags if preferred.


Each machine has an RFID scanner and box of electronics attached to it. When the machine is powered up, a small  microcontroller carries out a quick power on self test (POST) routine to ensure that all is well. 

A message then appears on the screen inviting people to scan their card or tag.
When the pupil wants to use the machine, they scan their tag / card.

The box then checks with a central web-based database to ensure that the pupil is authorised to use the machinery. 
The box then activates the relay for the machine to operate. It also starts a count-down timer for a predetermined time. A message is displayed on the screen to remind the student to take care and use appropriate PPE.
When the pupil presses the green button on the machine, a current sensor detects this and activates the dust extraction system. The count-down timer is also reset at this point.

As the machine runs, the current sensor is checked to ensure that the machine relay is not turned off half way through an operation.
When the machine stop button is pressed, the current sensor detects this and the dust extraction is allowed to run for a few more seconds before turning off.
The count-down timer starts again and gives the pupil a short window to make another operation if needed.
If the green start button is pressed again, then the system goes back to step 7.
If the green button is not pressed before the count-down timer reaches zero, then the system displays a friendly “Tidy up time” message.
After a short amount of time, the system reverts back to step 3.

Please see accompanying video demonstrations of Nextion and OLED versions:

Nextion HIM Video Demonstration: https://photos.app.goo.gl/MLrm7PYxA9FNsKv16

ESP32 OLED Video Demonstration: https://photos.app.goo.gl/LhkMh6XkSVJw7ab58




What benefits can this system provide?

The main benefit of using this machine will be to prevent inexperienced pupils turning on machinery that they shouldn’t. This should in turn reduce the chance of certain accidents from happening.

As the machine is being used, various pieces of data are passed to the RDBMS to record the following:
Which machine?
Who used it?
For how long?

This information can then be interrogated and used to predict when maintenance operations are needed. Additionally, the system will record when maintenance has taken place and it would be possible to have a “technician dashboard” to highlight each day, what jobs may need to be completed.

Please refer to the following videos for an idea of how this aspect works:

https://photos.app.goo.gl/aBuzhqXeez2h8WQY7


https://photos.app.goo.gl/uCatcJGUZReua9dz9
 
Staff Use

The system identifies the pupil via the RFID card serial number but each card also has an additional 1kb of non-volatile storage available. Staff members will be able to store a special passcode within this 1kb to identify them as having extra levels of access. T 

Power On Self Test (POST)

When the box is turned on it checks the following:

Does the RFID reader work properly?
Does the WiFi connect?
Can we connect to the RDBMS?

The RFID scanner should work properly at all times, unless some form of damage has occurred. If this happens, then it will have to be replaced, although it is not a common scenario. As a work-around, it may be useful to add a manual override via a simple physical key?

If the system cannot connect to WiFi, or the RDBMS, then it will go into “Staff Only” mode. This means that it will only respond to a staff key card / tag. Hence it can read the special passcode from the card memory to ensure that the person is a member of staff. The passcode could be a simple password or a hashed value based on the staff member’s credentials. This second option may provide an extra layer of security.




What’s in the box?

There are currently two working prototype circuits. They are essentially the same, with the slight differences being the choice of microcontroller and display.

ESP32 OLED

Parts List

The components are all available “off the shelf”:

Microcontroller choice:

ESP8266 NodeMCU

 
The ESP8266 is the less expensive of the two microcontrollers, but it is more than capable in this application. The only extra requirement was for a display of some sort. It is possible to interface an OLED screen to the ESP8266 but a more elegant solution is to use the Wemos ESp32 with a built-in OLED screen.

Wemos ESP32 OLED


This has the advantage of having a built-in OLED screen. However, the screen is quite small and monochrome.
RFID Scanner

This particular model RC522 is very popular and there is lots of information concerning how to use it with Arduoino, etc. It is available from a number of sources, but the very cheap versions can be temperamental.

Nextion HMI Display


This is a full-featured touchscreen display which can be easily programmed. However, it is a little on the pricey side, if we were to use it for every machine in the workshop.
Other TFT screens are available for Arduino circuits, although they are dumb screens, whereas the Nextion contains its own microprocessor which handles everything and makes interfacing incredibly simple. 

The touchscreen feature is not used in the current prototype (this was due in part to thoughts regarding COVID. However, this could be activated so that the operator could be asked questions - such as have you set the guard correctly? Is there any sign of damage to the machine? Are you wearing appropriate PPE? Have you cleaned up?


ACS712 Current Sensor

The current sensor is used to check that the machine is in operation. This allows the system to record when the machine has been turned on and turned off and thus work out the usage time. It also plays a part in activating the dust extractor.


Relays

 The prototype uses 2 relays. One for the machine and one for the dust extractor. This may not be needed if the dust extractor is already taken care of. A single relay module is readily available.

It should be noted that the relay can handle up to 10A. This may be a problem, and may need to be swapped if the machinery has a higher inrush current.
