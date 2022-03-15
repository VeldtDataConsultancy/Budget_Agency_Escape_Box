// Command module running on an ESP32.
#include <SoftwareSerial.h>
#include <Bounce2.h>
#include <PJONSoftwareBitBang.h>
#include "DFRobotDFPlayerMini.h"

// PJON initialization.
#define PJON_Phone_Id          19 // Id for the Phone module.
#define PJON_Command_Id        20 // Id for the Command module.
#define PJON_Comm_Pin          25 // Communication pin for PJON on the target PLC.

// MP3 initialization.
#define MP3_RX_PIN              5     // GPIO5/D1
#define MP3_TX_PIN              4     // GPIO4/D2
#define MP3_SERIAL_SPEED        9600  // DFPlayer Mini suport only 9600-baud
#define MP3_SERIAL_BUFFER_SIZE  32    // Software serial buffer size in bytes, to send 8-bytes you need 11-bytes buffer (start byte+8-data bytes+parity-byte+stop-byte=11-bytes)

// Pin initialization.
#define Start_Pin               12    // GPIO12. Pin for the start button.
#define Mp3_Pin                 23    // Pin to check if the Mp3 player is busy.

// PJON Bus Declaration.
PJONSoftwareBitBang bus(PJON_Command_Id);

// SoftwareSerial and MP3player declaration.
SoftwareSerial mp3Serial; // RX, TX
DFRobotDFPlayerMini mp3;

// Bounce Switch Initialization.
Bounce startSwitch = Bounce();

// STRUCTS
struct payLoad {
  uint8_t cmd;          // Defines the what to do on the other side.
  char msgLine[20];     // Character to send either phone number.
};

payLoad pl;

// ENUMERATORS
typedef enum {Idle_Init, Idle, Dialtone, Connecting_Init, Connecting, Connected, Disconnected, Ringing} phoneStateType;
phoneStateType phoneState = Idle;

// CONSTANTS

// GLOBAL VARIABLES
uint8_t mp3ToPlay = 0;        // Mp3 number that needs to be heard.
unsigned long ringDelayTime;  // Start time of the delay before the message is played.
uint16_t ringWaitTime;        // A slight delay of 1,5 seconds before an mp3 plays. Gives time to put the horn to the ear.

unsigned long startTime;      // Start time of the game. Used to display the timer.
int32_t clockSec;             // Current second in the game.
int32_t oldSec;               // Previous second of the game
char timeString[20];          // String to display the time on the LCD screen.

// FUNCTIONS
void send_command(uint8_t id, uint8_t cmd) {
  // A function that handles all the messaging to the command module.
  payLoad pl;
  pl.cmd = cmd;
  Serial.println("Message Sent without line");
  bus.send(id, &pl, sizeof(pl));
};

void send_command(uint8_t id, uint8_t cmd, char msgLine[20]) {
  // A function that handles all the messaging to the command module.
  payLoad pl;
  strcpy(pl.msgLine , msgLine);
  pl.cmd = cmd;
  Serial.println("Message Sent with line");
  bus.send(id, &pl, sizeof(pl));
};

// Receiver function to handle all incoming messages.
void receiver_function(uint8_t *payload, uint16_t length, const PJON_Packet_Info &packet_info) {
  // Read where the message is from.
  // Copy the payload byte array into struct.
  memcpy(&pl, payload, sizeof(pl));
  Serial.println(pl.cmd);
  Serial.println(pl.msgLine);

  if (packet_info.tx.id == 19) {
    if (pl.cmd == 1) phoneState = Idle_Init;    // Phone horn went on the hook. State to idle. Stop the MP3 player.
    if (pl.cmd == 2) phoneState = Dialtone;     // Phone horn went off the hook. Loop the dial tone.
    if (pl.cmd == 3) {                          // Repeat button is pressed. Play the last played MP3.
      if (mp3ToPlay != 0) {
        phoneState = Ringing;
      }
    }
    if (pl.cmd == 4) phoneState = Idle_Init;    // Phone starts to dial. Stop the MP3 plpayer.
    if (pl.cmd == 10) {                         // Phone number dialled. Check if it is correct.
      if (strcmp(pl.msgLine, "12345") == 0) {
        mp3ToPlay = 6;
        ringWaitTime = (random(4, 8) * 1000) + 4000;
        phoneState = Connecting_Init;
      }
      else {
        phoneState = Disconnected;
      }
    }
    if (pl.cmd == 11) phoneState = Disconnected;  // Phone number entered is too big. Play disconnect song and set phone state to disconnect.
    if (pl.cmd == 12) {             // Phone horn off the hook after Ringing State. Play suggested MP3.
      ringDelayTime = millis();
      ringWaitTime = 1500;
      phoneState = Connecting;
    }
  }
};

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  mp3Serial.begin(MP3_SERIAL_SPEED, SWSERIAL_8N1, MP3_RX_PIN, MP3_TX_PIN, false, MP3_SERIAL_BUFFER_SIZE, 0);

  bus.strategy.set_pin(PJON_Comm_Pin);
  bus.set_receiver(receiver_function);
  bus.begin();

  if (!mp3.begin(mp3Serial)) {  //Use softwareSerial to communicate with mp3.
    Serial.println(F("DFPlayer unable to begin:"));
    while (true);
  }
  Serial.println(F("DFPlayer Mini online."));

  // mp3 Config Settings
  mp3.setTimeOut(400); //Set serial communictaion time out 400ms
  mp3.volume(30);  //Set volume value (0~30).
  mp3.EQ(DFPLAYER_EQ_NORMAL);
  mp3.outputDevice(DFPLAYER_DEVICE_SD);

  // Start Button Pin and Switch Intialization.
  pinMode(Start_Pin, INPUT_PULLUP);
  startSwitch.attach(Start_Pin);
  startSwitch.interval(20);

  // Pin to read from if the Mp3 player is busy.
  pinMode(Mp3_Pin, INPUT);

  // Start the game time.
  startTime = millis();
}

void loop() {
  // put your main code here, to run repeatedly:
  startSwitch.update();

  clockSec = (millis() - startTime) / 1000;

  if (clockSec != oldSec) {
    int sec = clockSec % 60;
    int mint = clockSec / 60 % 60;
    int hr = clockSec / 3600;

    sprintf(timeString, "Time: %02d:%02d:%02d", hr, mint, sec);
    Serial.println(timeString);
    oldSec = clockSec;
  }

  switch (phoneState) {
    case Idle_Init:
      mp3.stop();
      phoneState = Idle;
      break;

    case Idle:
      break;

    case Dialtone:
      mp3.loop(1);
      phoneState = Idle;
      break;

    case Connecting_Init:
      ringDelayTime = millis();
      mp3.play(3);
      phoneState = Connecting;
      break;

    case Connecting:
      if (millis() - ringDelayTime > ringWaitTime) {
        mp3.play(mp3ToPlay);
        send_command(PJON_Phone_Id, 2);
        phoneState = Connected;
        delay(20);
      }
      break;

    case Connected:
      if (digitalRead(Mp3_Pin) == 1) {
        send_command(PJON_Phone_Id, 3);
        phoneState = Disconnected;
      }
      break;

    case Disconnected:
      mp3.loop(2);
      phoneState = Idle;
      break;

    case Ringing:
      send_command(PJON_Phone_Id, 1);
      phoneState = Idle;
      break;
  }

  bus.update();
  bus.receive(10000);
}
