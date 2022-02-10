// INCLUDES
#include <Bounce2.h>
#include <PJONSoftwareBitBang.h>

PJONSoftwareBitBang bus(80);

// OBJECTS
Bounce hookSwitch = Bounce();
Bounce repeatSwitch = Bounce();
Bounce dialSwitch = Bounce();
Bounce numberSwitch = Bounce();

// STRUCTS
struct payLoad {
  uint8_t cmd;          // Defines the what to do on the other side.
  uint8_t cmd_par_1;    // Defines a parameter setting as extra input for the other side.
  uint8_t cmd_par_2;    // Defines a second parameter setting as exta input for receiving arduino.
  char msgLine[20];     // Character to send either phone number.
};
 
// CONSTANTS
const int hookPin = 2;
const int repeatPin = 3;
const int dialPin = 4;
const int numberPin = 5;

const int ringerPins[] = {6, 7};
const int maxPhoneNumber = 4;

typedef enum {Idle, Dialtone, Dialling, Connecting_Init, Connecting, Connected, Disconnected, Ringing} stateType;
stateType state = Idle;

unsigned long lastRingTime;
unsigned long startTime = 0;
unsigned long dialTime = 0;
int waitTime;
String cmdString;

// GLOBALS
bool startGame = true;
bool ringQueue = false;
char number[maxPhoneNumber + 1]; // Phone number length plus terminal signal.
int currentDigit = 0;
int pulseCount = 1;

// Functions
void send_command(uint8_t cmd) {
  // a function that handles all the phone audio messaging to the command module.
  payLoad r;
  Serial.println("Integer message sending.");
  r.cmd = cmd;
  bus.send(90,&r, sizeof(&r));
  bus.update();
};

void send_command(char msgLine[20]) {
  // a function that handles a dialled phone number with the command.
  payLoad r;
  strcpy(r.msgLine , msgLine);
  r.cmd = 5;

  bus.send(90,&r, sizeof(&r));
  bus.update();
};

void setup() {
  bus.strategy.set_pin(9);
  bus.begin();
  bus.set_receiver(receiver_function);

  // put your setup code here, to run once:
  Serial.begin(9600);

  // Define hookSwitch and link it to hookPin.
  pinMode(hookPin, INPUT_PULLUP);
  hookSwitch.attach(hookPin);
  hookSwitch.interval(20);

  // Define helpSwitch and link it to helpPin.
  pinMode(repeatPin, INPUT_PULLUP);
  repeatSwitch.attach(repeatPin);
  repeatSwitch.interval(20);

  // Define dialSwitch and link it to dialPin.
  pinMode(dialPin, INPUT_PULLUP);
  dialSwitch.attach(dialPin);
  dialSwitch.interval(25);

  // Define numberSwitch and link it to numberPin.
  pinMode(numberPin, INPUT_PULLUP);
  numberSwitch.attach(numberPin);
  numberSwitch.interval(25);

  pinMode(ringerPins[0], OUTPUT);
  pinMode(ringerPins[1], OUTPUT);
  digitalWrite(ringerPins[0], LOW);
  digitalWrite(ringerPins[1], LOW);

  Serial.println("Setup done.");
}

void loop() {
  // put your main code here, to run repeatedly:
  hookSwitch.update();
  repeatSwitch.update();
  dialSwitch.update();
  numberSwitch.update();

  if (hookSwitch.rose()) {
    Serial.println("Hook down. Back to Idle State.");
    state = Idle;
    memset(number, 0 , sizeof number);
    currentDigit = 0;
    pulseCount = 0;
    send_command(2);
  }

  switch (state) {
    case Idle:
      if (hookSwitch.fell() && startGame == true) {
        Serial.println("Hook raised. State to Dialtone");
        send_command(1);
        state = Dialtone;
      }

      if (repeatSwitch.fell() || ringQueue == true) {
        state = Ringing;
      }
      break;

    case Dialtone:
      if (dialSwitch.fell()) {
        payLoad r;
        r.cmd = 2;
        bus.send_packet(90,&r, sizeof(&r));
        state = Dialling;
      }
      break;

    case Dialling:
      // During dialling a user needs to enter a phone number.
      // If a first numer is dialled. It gives the user 4 seconds to enter a next number.
      // Else the number is sent to the command module.
      // If more than 6 numbers are entered, it goes to a disconnected state.
      
      if (numberSwitch.fell()) {
        pulseCount++;
      }
      
      if (dialSwitch.rose()) {
        if (pulseCount >= 10) {
          pulseCount = 0;
        }
        number[currentDigit] = pulseCount | '0';
        Serial.println(number);

        currentDigit++;
        pulseCount = 0;
      }

      if (currentDigit > maxPhoneNumber) {
        send_command(3);
        state = Disconnected; 
      }
      break;

    case Connecting_Init: {
        startTime = millis();
        waitTime = (random(0, 8) * 1000) + 4000;
        state = Connecting;
      }

    case Connecting:
      dialTime = millis();

      if (dialTime - startTime > waitTime) {
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
        ringQueue = false;
      }
      break;
  }
  bus.receive(8000);
}

void receiver_function(uint8_t *payload, uint16_t length, const PJON_Packet_Info &info) {
  // cmd values for the phone module.
  // 0. Start game.
  // 1. Ring the phone if phone is idle.
  // 2. Correct number entered. Switch state to Connecting_Init.
  // 3. Connect_Init over, switch state to Connected.
  // 4. False number entered. 
  //    If state is dialling, send to Disconnected state.
  //    User will hear that the phone is not responding and needs to set it to idle.
  payLoad r;
    
  memcpy(&r, payload, sizeof(r));
  Serial.print("Signal received: ");
  Serial.println(r.cmd);
  if (r.cmd == 0) startGame = true; 
  if (r.cmd == 1) ringQueue = true;
  if (r.cmd == 2 && state == Dialling) state = Connecting_Init;
  if (r.cmd == 3 && state == Connecting_Init) state = Connected;
  if (r.cmd == 4 && state == Dialling) state = Disconnected;
  
  // Avoid simultaneous transmission of Serial and SoftwareBitBang data
  Serial.flush();
};
