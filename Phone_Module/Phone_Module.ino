// Phone module running on an Arduino Nano.
#include <PJONSoftwareBitBang.h>
#include <Bounce2.h>

#define PJON_Phone_Id 19
#define PJON_Command_Id 20
#define PJON_Comm_Pin 8
#define hookPin 2

// PJON Bus Declaration.
// 20 for Command Module.
// 19 for Phone Module.
PJONSoftwareBitBang bus(PJON_Phone_Id);

// Phone Switches.
Bounce hookSwitch = Bounce();

// STRUCTS
struct payLoad {
  uint8_t cmd;          // Defines the what to do on the other side.
  char msgLine[20];     // Character to send either phone number.
};

payLoad pl;

// CONSTANTS

// GLOBAL VARIABLES

// FUNCTIONS
void send_command(uint8_t id, uint8_t cmd) {
  // A function that handles all the messaging to the command module.
  payLoad r;
  r.cmd = cmd;
  bus.send(id, &r, sizeof(&r));
  bus.update();
};

// Receiver function to handle all incoming messages.
void receiver_function(uint8_t *payload, uint16_t length, const PJON_Packet_Info &packet_info) {
  // Read where the message is from.
  // packet_info.tx.id > Number of the transmitter Id.
  // packet_info.rx.id > Number of the receiver Id.

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
}

void loop() {
  // put your main code here, to run repeatedly:
  hookSwitch.update();

  if (hookSwitch.fell()) {
    send_command(PJON_Command_Id, 1);
  }

  bus.update();
  bus.receive(10000);
}
