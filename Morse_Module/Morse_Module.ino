#include <Bounce2.h>
#include <PJONSoftwareBitBang.h>

// PJON initialization.
#define PJON_Morse_Id     19  // Id for the Phone module.
#define PJON_Command_Id   20  // Id for the Command module.
#define PJON_Comm_Pin     8   // Communication pin for PJON on the target PLC.

// Pin initialization.
#define Key_Pin           12  // Pin for the Telegraph Key button.

// PJON Bus Declaration.
PJONSoftwareBitBang bus(PJON_Command_Id);

// Bounce Switch Initialization.
Bounce keySwitch = Bounce(); // Bounce object for the start button.

// STRUCTS
struct payLoad {
  uint8_t cmd;          // Defines the what to do on the other side.
  char msgLine[20];     // Character to send any phone number.
};

// Script response array. Parameters needed to give a response to the user.
typedef struct {
  char morseChar;     // Character of the alphabet.
  char morseCode[4];  // Translated morse code.
} morseArray;

morseArray morseDictionary[] = {
  {"a", ".-" },
  {"b", "-..."},
  {"c", "-.-."},
  {"d", "-.."},
  {"e", "."},
  {"f", "..-."},
  {"g", "--."},
  {"h", "...."},
  {"i", ".."},
  {"j", ".---"},
  {"k", "-.-"},
  {"l", ".-.."},
  {"m", "--"},
  {"n", "-."},
  {"o", "---"},
  {"p", ".--."},
  {"q", "--.-"},
  {"r", ".-."},
  {"s", "..."},
  {"t", "-"},
  {"u", "..-"},
  {"v", "...-"},
  {"w", ".--"},
  {"x", "-..-"},
  {"y", "-.--"},
  {"z", "--.."},
};

void send_command(uint8_t id, uint8_t cmd) {
  // A function that handles all the messaging to the other modules.
  payLoad pl;
  pl.cmd = cmd;
  bus.send(id, &pl, sizeof(pl));
};

void send_command(uint8_t id, uint8_t cmd, char msgLine[20]) {
  // A function that handles all the messaging to the other modules.
  payLoad pl;
  strcpy(pl.msgLine , msgLine);
  pl.cmd = cmd;
  bus.send(id, &pl, sizeof(pl));
};

// Receiver function to handle all incoming messages.
void receiver_function(uint8_t *payload, uint16_t length, const PJON_Packet_Info &packet_info) {
  // Read where the message is from.
  // Copy the payload byte array into struct.
  payLoad pl;
  memcpy(&pl, payload, sizeof(pl));
};

void setup() {
  // Setup of the PJON communication.
  bus.strategy.set_pin(PJON_Comm_Pin);
  bus.set_receiver(receiver_function);
  bus.begin();

  // Start Button Pin and Switch Intialization.
  pinMode(Key_Pin, INPUT_PULLUP);
  keySwitch.attach(Key_Pin);
  keySwitch.interval(20);
}

void loop() {
  // put your main code here, to run repeatedly:

}
