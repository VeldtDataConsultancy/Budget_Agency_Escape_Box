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
typedef enum {Idle, Dialtone, Dialling, Connecting_Init, Connecting, Connected, Disconnected, Ringing} stateType;
stateType state = Idle;

// GLOBAL VARIABLES
const uint8_t maxPhoneNumber = 4; // Maximum phone number to accept. Above results in disconnect.
uint8_t pulseCount = 0;           // Count of the amount of pulses observed while dialing.
uint8_t currentDigit = 0;         // current digit of the number being Dialled.
char number[maxPhoneNumber + 1];  // Phone number length plus terminal signal.

// FUNCTIONS
void send_command(uint8_t id, uint8_t cmd) {
  // A function that handles all the messaging to the command module.
  payLoad pl;
  pl.cmd = cmd;
  bus.send(id, &pl, sizeof(&pl));
  bus.update();
};

void send_command(uint8_t id, uint8_t cmd, char msgLine[20]) {
  // A function that handles all the messaging to the command module.
  payLoad pl;
  strcpy(pl.msgLine , msgLine);
  pl.cmd = cmd;
  bus.send(id, &pl, sizeof(&pl));
  bus.update();
};

void receiver_function(uint8_t *payload, uint16_t length, const PJON_Packet_Info &packet_info) {
  // Receiver function to handle all incoming messages.
  // Copy the payload byte array into struct.
  memcpy(&pl, payload, sizeof(pl));
  Serial.println(pl.cmd);
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
        if (dialSwitch.fell()) { // Start Dialling soon as the dialSwitch is open.
          send_command(PJON_Command_Id, 5);
          state = Dialling;
        }
      }
      break;

    case Dialling:
      if (numberSwitch.fell()) {
        pulseCount++;
      }
      
      if (dialSwitch.rose()) {
        if (pulseCount >= 10) {
          pulseCount = 0;
        }
        number[currentDigit] = pulseCount | '0';
        
        send_command(PJON_Command_Id,10,number);
        currentDigit++;
        pulseCount = 0;
      }

      if (currentDigit > maxPhoneNumber) {
        state = Disconnected;
        send_command(PJON_Command_Id,11);
      }
      break;

    case Connecting_Init:
      {
      }
      break;

    case Connecting:
      {
      }
      break;

    case Connected:
      {
      }
      break;

    case Disconnected:
      {
      }
      break;

    case Ringing:
      {
      }
      break;
  }
  // Intermediate solution.
  // While Dialing, waiting for the receive bus is hampering the pulseCount.
  // If properly working, this should be shut off as little as possible.
  if (state != Dialling) bus.receive(10000); 
}
