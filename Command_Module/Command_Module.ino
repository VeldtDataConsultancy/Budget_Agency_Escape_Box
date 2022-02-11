// Command module running on an ESP32.
#include <PJONSoftwareBitBang.h>

#define PJON_Bus_ID 20
#define PJON_Comm_Pin 25

// PJON Bus Declaration.
// 20 for Command Module.
// 19 for Phone Module.
PJONSoftwareBitBang bus(PJON_Bus_ID);

// STRUCTS
struct payLoad {
  uint8_t cmd;          // Defines the what to do on the other side.
  char msgLine[20];     // Character to send either phone number.
};

payLoad pl;

// CONSTANTS

// GLOBAL VARIABLES

// Receiver function to handle all incoming messages.
void receiver_function(uint8_t *payload, uint16_t length, const PJON_Packet_Info &packet_info) {
  // Read where the message is from.
  Serial.print(" Transmitter id: ");
  Serial.println(packet_info.tx.id);

  // Copy the payload byte array into struct.
  memcpy(&pl, payload, sizeof(pl));
  Serial.println(pl.cmd);
  bus.reply(&pl, sizeof(&pl));
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
  bus.update();
  bus.receive(10000);
}
