// Phone module running on an Arduino Nano.
#include <PJONSoftwareBitBang.h>

#define PJON_Bus_ID  19
#define PJON_Comm_Pin 8

// PJON Bus Declaration.
// 20 for Command Module.
// 19 for Phone Module.
PJONSoftwareBitBang bus(PJON_Bus_ID);

void receiver_function(uint8_t *payload, uint16_t length, const PJON_Packet_Info &packet_info) {
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
