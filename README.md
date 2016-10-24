# AAA_GSM_to_Relay_v1.7
boiler command via SMS using arduino nano and GSM shield

developed by JL Stoefs 19/03/2016 jlstoefs@gmail.com This example is in the public domain.
 * Based on ReceiveSMS, SendSMS from GSM.h library and Arduino Nano v3.
 * Description: sketch to control the boiler status (normal mode vs ECO mode) via a SIMM900 GPS shield and arduino. Activity monitoring enabled via the serial monitor
 * When the system is unpowered, the boiler will be on normal mode. When the system is initialized, it switches to ECO mode by default.
 * 
 * Boiler Wiring: in our case, the boiler is functionning normally if 2 boiler wires (A & B below) are NOT connected. When A&B are connected, the boiler switches to ECO mode. 
 ==>A & B wires will be connected to the "normally open circuit" connections of the relay (left and middle pins of the relay).See your own boiler description for correct setup.

 ==>The system has 2 status:
              *Status "HIGH" = A & B disconnected (Relay inactive) = warming system in normal mode = kLedGreen ON
              *Status "LOW" =  A & B connected (Relay active) = ECO mode = kLedRed blinking

Physical description: the system requires one arduino, 1 GSM shield, 3 status leds (green/yellow/red), one button and one switch:
-3 leds:  Green led (kLedGreen): Normal Mode Status led (corresponding to SMS "ON" command): lights when "Status" var is HIGH 
          Yellow led (kLedYellow):SMS confirmation led : lignts on if the system is allowed to send command confirmation SMS.
          Red led (kLedRed): ECO Mode blinking Status led (corresponding to SMS "OFF" command) : blinks when "Status" var is LOW 
-1 switch (kConfirmPin): to allow the sending of confirmation SMS. Operates the yellow led.
-1 Button (kButton): added to manually change the status
-1 Relay: A & B boiler wires are connected to the "normally open circuit" switches of the relay, 

Valid SMS command lines:
 Send "ON" by SMS to activate normal mode. Confirmation SMS then sent if kConfirmPin is HIGH
 Send "OFF" to activate ECO mode. Confirmation SMS then sent if kConfirmPin is HIGH
 Send "Status" to know the current system status. Confirmation SMS then sent if kConfirmPin is HIGH
 After reading, the incoming SMS will be automatically deleted. 
v1.4: Buzzer functionality added
v1.5: lighter, minor improvements ( StartupLedShow function)
v1.6: F() function used for Serial.print
v1.7: SMS_Content defined in the loop and not globally (ensures that its content is deleted once out of SMS.available {} )

-------------------------------------------------------------------------------------------------------
