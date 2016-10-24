/*developed by JL Stoefs 19/03/2016 jlstoefs@gmail.com This example is in the public domain.
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
v1.7: SMS_Content defined in the loop and not globally (ensures that its content is deleted once out of SMS.available {}
-------------------------------------------------------------------------------------------------------
*/

// include the GSM library
#include <GSM.h>

// PIN Number for the SIM
#define PINNUMBER "" //"" for unlocked sim, otherwise insert the PIN code here

// initialize the library instances
GSM gsm;
GSM_SMS sms;

// define below the INPUT pins:
const int kConfirm=A0;  // Pin must be set to GND to disable SMS confirmation 
const int kButton=A3;  // button to manually switch the status (ON vs ECO)

// define below the OUTPUT pins:
const int kLedYellow=A1;  // Confirmation SMS led
const int kLedGreen=A5; // Normal Mode ("ON") Status led
const int kLedRed=13; // ECO Mode ("OFF") Status led (blinking)
const int kRelay=11; // to connect to Relay "IN" pin
const int kBuzzer=12; //Buzzer +5V
const int kLeds[]={kLedGreen,kLedYellow,kLedRed,kBuzzer};

boolean ConfirmSMS= HIGH; // must be HIGH to receive confirmation SMS (controlled by kConfirmPin)
boolean previousConfirmSMS=LOW; //check change in Confirm
boolean Status=LOW; // used for Relay and Status SMS: LOW: ECO mode (default at start)
boolean Toggle=HIGH; // used to flash the Status Led in case of ECO mode
boolean ButtonState=HIGH;
boolean PreviousButtonState= HIGH; //used to measure kbutton activity

//-----------------------------------------------------------------------------------------------------

void setup()                                                                                            { 
  // initialize serial communications and wait for port to open:
pinMode(kConfirm,INPUT_PULLUP);
pinMode(kButton,INPUT_PULLUP);
pinMode(kLedYellow,OUTPUT); 
pinMode(kLedRed,OUTPUT);
pinMode(kLedGreen,OUTPUT);
pinMode(kRelay, OUTPUT);  
pinMode(kBuzzer,OUTPUT);
ConfirmSMS=digitalRead(kConfirm);  //  check if kConfirmPin is HIGH (i.e. send  confirmation SMS)
digitalWrite(kRelay, LOW);          //  krelay in ECO mode (in case system reboots and cannot start GSM connection) 

Serial.begin(57600); //set to 57.600 to be compatible with Bluetooth module

while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
                }
  Serial.println(F("GSM relay v1.7"));

boolean notConnected = true; // connection state
  // Start GSM connection
  while (notConnected) {
    if (gsm.begin(PINNUMBER)==GSM_READY) notConnected = false;
    else {
      Serial.println(F("Not connected"));
      delay(1000);
          }
                        }
                    
// Send info on serial monitor:
Serial.print(F("ConfirmSMS: "));
Serial.println(ConfirmSMS);
Serial.print(F("Default Status: "));
Serial.println(Status);
Serial.println(F("Ready"));
           
StartupLedShow(3);  //blinking all leds when connection to network is established
                                                                                                        }
//-----------------------------------------------------------------------------------------------------
void loop()                                                                                             { //Start Loop
                    
ConfirmSMS=digitalRead(kConfirm); // SMS confirmation sent if pin kConfirmPin is NOT grounded
digitalWrite(kLedYellow,ConfirmSMS); // led kLedYellow on if ConfirmSMS is activated 
if(ConfirmSMS!=previousConfirmSMS)              {
Serial.print(F("ConfirmSMS: "));
Serial.println(ConfirmSMS);
Serial.println("---------"); 
previousConfirmSMS=ConfirmSMS;                  }

ButtonState=digitalRead(kButton);
if( ButtonState != PreviousButtonState && ButtonState==LOW)  {//kButton pressed to manually switch the status mode
      Status=!Status;                          //change status value
      delay(10);
      PreviousButtonState=ButtonState;
      Serial.print(F("Manual status change: new status= "));
      Serial.println(Status);
                                                         }
else if( ButtonState != PreviousButtonState &ButtonState==HIGH)  { //button is released
      delay(10);
      PreviousButtonState=ButtonState;
      if (Status==HIGH) BUZZER(3,50,50);
      else BUZZER(1,500,0);
                                                         }

  // If there are SMSs available()
if (sms.available())                                                                  {
     char c;  //local var to store SMS content
     int SMS_Content_Loc=0;  // local var used to store each SMS character
     char SMS_Content[6]; // used to store SMS content
     char senderNumber[15]; //to store the number having sent the SMS command (required when sending confirmation pin)
    Serial.print(F("Msg from: "));
    sms.remoteNumber(senderNumber, 15);    // Get remote number
    Serial.println(senderNumber);
    // message disposal: Any messages not starting with 'O' ("ON" or "OFF")or 'S' ("Status")are discarded
    if (sms.peek() != 'O' && sms.peek() != 'o' && sms.peek() != 'S' && sms.peek() != 's') 
    {
      Serial.println(F("Discarded SMS"));
      sms.flush();
      BUZZER(2,750,250); //buzzer for SMS deleted
    } 

    // Read message bytes, print them and store the first 6 ones in SMS_Content
    while (c = sms.read())        {
      Serial.print(c);
      if (SMS_Content_Loc<6)    {  //Stores only the first 6 bytes of the SMS in SMS_Content
        SMS_Content[SMS_Content_Loc] = c;  
        SMS_Content_Loc++;      }
                                  }
      BUZZER(1,50,450); //buzzer for SMS received

    Serial.println(F(" / END OF MESSAGE"));

    if ( (SMS_Content[0] == 'O' || SMS_Content[0] =='o') 
      && (SMS_Content[1] == 'N' || SMS_Content[1] == 'n' ))                   {   // ON* SMS
              Status=HIGH;
              Serial.print(F("ON;Status set to "));
              Serial.println(Status);
              BUZZER(3,50,50);
              delay(700);
              SEND_SMS(senderNumber,"ON: mode chauffage actif.Liste des SMS valides: ON / OFF / Status");             
                                                                               }   // End ON* SMS
 
      else if ( ( SMS_Content[0] == 'O' || SMS_Content[0] == 'o') 
              && ( SMS_Content[1] == 'F' || SMS_Content[1] == 'f') 
              && ( SMS_Content[2] == 'F' || SMS_Content[2] == 'f') )              {     //OFF* SMS
              Status=LOW; 
              Serial.print(F("OFF;Status set to "));
              Serial.println(Status);
              BUZZER(1,500,500);  
              SEND_SMS(senderNumber,"OFF: mode ECO actif.Liste des SMS valides: ON / OFF / Status");
                                                                                   }     //End OFF* SMS


      else if (  (SMS_Content[0] =='S'|| SMS_Content[0] == 's')                                                                                 
              && (SMS_Content[1] =='T'|| SMS_Content[1] == 't') 
              && (SMS_Content[2] =='A'|| SMS_Content[2] == 'a') 
              && (SMS_Content[3] =='T'|| SMS_Content[3] == 't') 
              && (SMS_Content[4] =='U'|| SMS_Content[4] == 'u')  
              && (SMS_Content[5] =='S'|| SMS_Content[5] == 's') )                {   //Status* SMS
              Serial.print(F("STATUS: Status is "));
              Serial.println(Status);
                      if(Status==HIGH)  SEND_SMS(senderNumber,"Status: en mode chauffage (ON).Liste des SMS valides: ON / OFF / Status");
                       else             SEND_SMS(senderNumber,"Status: en mode ECO (OFF).Liste des SMS valides: ON / OFF / Status");
                                               
                                                                                  }   //End Status* SMS
    // Delete message from SIM card
    sms.flush();
    Serial.println(F("MESSAGE DELETED"));
    Serial.println(F("-------------------")); 
    
                                                                                      } //end of sms.available

if (millis()%3000 >1500 && Status==LOW) Toggle=HIGH; else Toggle=LOW; //slow flashing kLedRed if Status=LOW
digitalWrite(kRelay, Status);
digitalWrite(kLedGreen,Status);
digitalWrite(kLedRed,Toggle);  // Led de Statut
                                                                                                        }               //End Loop
//-----------------------------------------------------------------------------------------------------
void SEND_SMS(char* MOBILE_NB,char* SMS_txt)                                                           { // Start SEND_SMS
 if(ConfirmSMS==HIGH)                           {                                                                                                  
       sms.beginSMS(MOBILE_NB); 
       sms.print(SMS_txt);  
       sms.endSMS(); 
       StartupLedShow(1);
                                                }
 else  Serial.print(F("SIMULATED "));
       Serial.print(F("Sent SMS to "));
       Serial.print(MOBILE_NB); 
       Serial.print(F(" : "));
       Serial.println(SMS_txt); 
       Serial.println(F("-------------------------"));
  StartupLedShow(1);
                                                                                                        } // End SEND_SMS
//-----------------------------------------------------------------------------------------------------

void BUZZER (int CYCLE, int LENGTH_ON, int LENGTH_OFF) 
{      for (int i=1;i<CYCLE+1;i++)        {
          digitalWrite(kBuzzer,HIGH);
          delay(LENGTH_ON);
          digitalWrite(kBuzzer,LOW);
          delay (LENGTH_OFF);              }
  }
  //-----------------------------------------------------------------------------------------------------
 void StartupLedShow(int cycles)
  { 
  long previousmillistart=millis();         
  for (int j=1;j<=cycles;j++)        
      {     //blinking all leds when connection to network is established
        for (int i=0;i<=3;i++) 
          {digitalWrite(kLeds[i],HIGH);
           delay(100);       
          }
        for (int i=0;i<=3;i++) 
          {digitalWrite(kLeds[i],LOW);
           delay(100);             
          } 
      }
  }
 
