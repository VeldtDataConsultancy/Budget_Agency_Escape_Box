// Command module running on an ESP32.
#include <Wire.h>
#include <SoftwareSerial.h>
#include <LiquidCrystal_I2C.h>
#include <Bounce2.h>
#include <PJONSoftwareBitBang.h>
#include "DFRobotDFPlayerMini.h"

// PJON initialization.
#define PJON_Phone_Id          19 // Id for the Phone module.
#define PJON_Command_Id        20 // Id for the Command module.
#define PJON_Comm_Pin          25 // Communication pin for PJON on the target PLC.

// MP3 initialization.
#define MP3_RX_PIN              5     //GPIO5/D1
#define MP3_TX_PIN              4     //GPIO4/D2
#define MP3_SERIAL_SPEED        9600  //DFPlayer Mini suport only 9600-baud
#define MP3_SERIAL_BUFFER_SIZE  32    //software serial buffer size in bytes, to send 8-bytes you need 11-bytes buffer (start byte+8-data bytes+parity-byte+stop-byte=11-bytes)

// PJON Bus Declaration.
PJONSoftwareBitBang bus(PJON_Command_Id);

// SoftwareSerial and MP3player declaration.
SoftwareSerial mp3Serial; // RX, TX
DFRobotDFPlayerMini mp3;

// LCD initialization.
LiquidCrystal_I2C lcd(0x27, 20, 4);

// STRUCTS
struct payLoad {
  uint8_t cmd;          // Defines the what to do on the other side.
  char msgLine[20];     // Character to send either phone number.
};

payLoad pl;

// CONSTANTS

// GLOBAL VARIABLES
uint8_t mp3ToPlay = 4;
unsigned long startDialTime;
unsigned long currDialTime;

// FUNCTIONS
void send_command(uint8_t id, uint8_t cmd) {
  // A function that handles all the messaging to the command module.
  payLoad pl;
  pl.cmd = cmd;
  bus.send(id, &pl, sizeof(pl));
};

void send_command(uint8_t id, uint8_t cmd, char msgLine[20]) {
  // A function that handles all the messaging to the command module.
  payLoad pl;
  strcpy(pl.msgLine , msgLine);
  pl.cmd = cmd;
  bus.send(id, &pl, sizeof(pl));
};

void playDialMp3(uint8_t mp3ToPlay) {
  send_command(PJON_Phone_Id, 2);
  startDialTime = millis();
  currDialTime = millis();
  mp3.play(3);
  uint16_t waitTime = (random(0, 8) * 1000) + 5000;
  while (currDialTime - startDialTime < waitTime) {
    currDialTime = millis(); 
  }
  mp3.play(mp3ToPlay);
}

// Receiver function to handle all incoming messages.
void receiver_function(uint8_t *payload, uint16_t length, const PJON_Packet_Info &packet_info) {
  // Read where the message is from.
  // Copy the payload byte array into struct.
  memcpy(&pl, payload, sizeof(pl));
  Serial.println(pl.cmd);
  Serial.println(pl.msgLine);

  if (packet_info.tx.id == 19) {
    if (pl.cmd == 1) mp3.stop();    // Phone horn went on the hook. Stop the MP3 player.
    if (pl.cmd == 2) mp3.loop(1);   // Phone horn went off the hook. Loop the dial tone.
    if (pl.cmd == 3);               // Repeat button is pressed. Play the last played MP3.
    if (pl.cmd == 4) mp3.stop();    // Phone starts to dial. Stop the MP3 plpayer.
    if (pl.cmd == 10) {             // Phone number dialled. Check if it is correct.
      if (strcmp(pl.msgLine,"12345")== 0) {
        send_command(PJON_Phone_Id,2);
        playDialMp3(4);
      }
    }
    if (pl.cmd == 11) {             // Phone number entered is too big. Play disconnect song and set phone state to disconnect.
      mp3.loop(2);
      send_command(19,3);
    }
    if (pl.cmd == 12);              // Phone horn off the hook after Ringing State. Play suggested MP3.
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

  // LCD Config Settings.
  lcd.init();
  lcd.setBacklight(100);
  delay(100);
  lcd.print("Console v0.1");
  lcd.setCursor(0, 3);
  String msgLine = "Test if it works.";
  lcd.print(msgLine);
}

void loop() {
  // put your main code here, to run repeatedly:
  bus.update();
  bus.receive(10000);
}
