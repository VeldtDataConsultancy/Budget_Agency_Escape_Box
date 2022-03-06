// Phone module running on an Arduino Nano.
#include <PJONSoftwareBitBang.h>
#include <Bounce2.h>

#define PJON_Phone_Id 19 // Id for the Phone module.
#define PJON_Command_Id 20 // Id for the Command module.
#define PJON_Comm_Pin 8 // Communication pin for PJON on the target PLC.

#define hookPin 2
#define repeatPin 3
#define dialPin 4
#define numberPin 5

// PJON Bus Declaration.
// 20 for Command Module.
// 19 for Phone Module.
PJONSoftwareBitBang bus(PJON_Phone_Id);

// Phone Switches.
Bounce hookSwitch = Bounce();
Bounce repeatSwitch = Bounce();
Bounce dialSwitch = Bounce();
Bounce numberSwitch = Bounce();

// STRUCTS
struct payLoad {
  uint8_t cmd;          // Defines the what to do on the other side.
  char msgLine[20];     // Character to send either phone number.
};

payLoad pl;

// ENUMERATORS
typedef enum {Idle, Dialtone, Dialling, Connected, Disconnected, Ringing} stateType;
stateType state = Idle;

// GLOBAL VARIABLES
const int ringerPins[] = {6, 7};
const uint8_t maxPhoneNumber = 6; // Maximum phone number to accept. Above results in disconnect.
uint8_t pulseCount = 0;           // Count of the amount of pulses observed while dialing.
uint8_t currentDigit = 0;         // current digit of the number being Dialled.
char number[maxPhoneNumber + 1];  // Phone number length plus terminal signal.

bool checkReceive = true;
unsigned long lastRingTime;       // Timestamp for ringing the bells while dialling.
unsigned long dialTime;      // Start time to dial.
unsigned long waitTimeTillConnect = 5000; // Wait time until a number gets sent.

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

void receiver_function(uint8_t *payload, uint16_t length, const PJON_Packet_Info &packet_info) {
  // cmd values for the phone module.
  // 0. Start game.
  // 1. Ring the phone if phone is idle.
  // 2. Correct number entered. Switch state to Connected.
  // 3. False number entered. Switch state to Disconnected.
  memcpy(&pl, payload, sizeof(pl));
  Serial.println(pl.cmd);
  if (pl.cmd == 0) Serial.println("TODO: Create start flag.");
  if (pl.cmd == 1) {
    if (state == Idle) state = Ringing;
    else Serial.println("TODO: Create Message Queue");
  }
  if (pl.cmd == 2) state = Connected;
  if (pl.cmd == 3) state = Disconnected;
};

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);

  // Setup PJON bus.
  bus.strategy.set_pin(PJON_Comm_Pin);
  bus.set_receiver(receiver_function);
  bus.begin();

  // Setup hookSwitch and link it to hookPin.
  pinMode(hookPin, INPUT_PULLUP);
  hookSwitch.attach(hookPin);
  hookSwitch.interval(20);

  // Define repeatSwitch and link it to helpPin.
  pinMode(repeatPin, INPUT_PULLUP);
  repeatSwitch.attach(repeatPin);
  repeatSwitch.interval(20);

  // Define dialSwitch and link it to dialPin.
  pinMode(dialPin, INPUT_PULLUP);
  dialSwitch.attach(dialPin);
  dialSwitch.interval(10);

  // Define numberSwitch and link it to numberPin.
  pinMode(numberPin, INPUT_PULLUP);
  numberSwitch.attach(numberPin);
  numberSwitch.interval(20);

  // Define the ringerpins as OUTPUT, initial state low.
  pinMode(ringerPins[0], OUTPUT);
  pinMode(ringerPins[1], OUTPUT);
  digitalWrite(ringerPins[0], LOW);
  digitalWrite(ringerPins[1], LOW);
}

void loop() {
  // put your main code here, to run repeatedly:
  hookSwitch.update();
  repeatSwitch.update();
  dialSwitch.update();
  numberSwitch.update();

  // Regardless of the phone state, hanging up the horn will bring it back to Idle state.
  if (hookSwitch.rose()) {
    memset(number, 0 , sizeof number);
    currentDigit = 0;
    pulseCount = 0;
    state = Idle;
    send_command(PJON_Command_Id, 1);
  }

  switch (state) {
    case Idle:
      {
        if (hookSwitch.fell()) { // if horn is off the hook, head to Dialtone state.
          state = Dialtone;
          send_command(PJON_Command_Id, 2);
        }

        if (repeatSwitch.fell()) { // if repeat button is pressed, send signal to command.
          send_command(PJON_Command_Id, 3);
        }
      }
      break;

    case Dialtone:
      {
        if (dialSwitch.fell()) { // Start Dialling as soon as the dialSwitch is open.
          send_command(PJON_Command_Id, 4);
          checkReceive = false;
          dialTime = millis();
          state = Dialling;
        }
      }
      break;

    case Dialling:
      if (numberSwitch.fell()) {
        pulseCount++;
      }

      if (dialSwitch.rose()) {
        checkReceive = true;
        if (pulseCount >= 10) {
          pulseCount = 0;
        }
        number[currentDigit] = pulseCount | '0';
        currentDigit++;
        pulseCount = 0;
      }

      if (currentDigit > maxPhoneNumber) {
        state = Disconnected;
        send_command(PJON_Command_Id, 11);
      }

      if (dialSwitch.fell()) {
        checkReceive = false;
        dialTime = millis();
      }

      if (millis() - dialTime > waitTimeTillConnect) {
        send_command(PJON_Command_Id, 10, number);
        state = Connected;
      }
      break;

    case Connected:
      break;

    case Disconnected:
      break;

    case Ringing:
      int nu = millis();
      if (nu - lastRingTime > 4000) {
        for (int j = 0; j < 2; j++) {
          for (int i = 0; i < 20; i++) {
            hookSwitch.update();
            if (hookSwitch.fell()) {
              j = 2;
              break;
            }
            digitalWrite(ringerPins[0], i % 2);
            digitalWrite(ringerPins[1], 1 - (i % 2));
            delay(40);  // Set to 40 on Brown Phone, 20 on Ivory Phone.
          }
          delay(200);
        }
        digitalWrite(ringerPins[0], LOW);
        digitalWrite(ringerPins[1], LOW);
        lastRingTime = nu;
      }
      if (hookSwitch.fell()) {
        state = Connected;
        send_command(PJON_Command_Id, 12);
      }
      break;
  }

  bus.update();
  if (checkReceive == true) bus.receive(10000);
}
