// Command module running on an ESP32.
#include <PJONSoftwareBitBang.h>

#define PJON_Phone_Id 19 // Id for the Phone module.
#define PJON_Command_Id 20 // Id for the Command module.
#define PJON_Comm_Pin 25 // Communication pin for PJON on the target PLC.

// PJON Bus Declaration.
PJONSoftwareBitBang bus(PJON_Command_Id);

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
  Serial.print(" Transmitter id: ");
  Serial.println(packet_info.tx.id);

  // Copy the payload byte array into struct.
  memcpy(&pl, payload, sizeof(pl));
  Serial.println(pl.cmd);
};

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);

  bus.strategy.set_pin(PJON_Comm_Pin);
  bus.set_receiver(receiver_function);
  bus.begin();
}

void loop() {
  // put your main code here, to run repeatedly:
  bus.receive(10000);
}
