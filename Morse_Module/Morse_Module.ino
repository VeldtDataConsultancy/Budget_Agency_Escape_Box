#include <Bounce2.h>

#define Morse_Pin 9

// Bounce Switch Initialization.
Bounce morseSwitch = Bounce(); // Bounce object for the start button.

// Script response array. Parameters needed to give a response to the user.
typedef struct {
  char morseChar;     // Character of the alphabet.
  String morseCode;  // Translated morse code.
} morseArray;

morseArray morseDictionary[] = {
  {'a', ".-" },
  {'b', "-..."},
  {'c', "-.-."},
  {'d', "-.."},
  {'e', "."},
  {'f', "..-."},
  {'g', "--."},
  {'h', "...."},
  {'i', ".."},
  {'j', ".---"},
  {'k', "-.-"},
  {'l', ".-.."},
  {'m', "--"},
  {'n', "-."},
  {'o', "---"},
  {'p', ".--."},
  {'q', "--.-"},
  {'r', ".-."},
  {'s', "..."},
  {'t', "-"},
  {'u', "..-"},
  {'v', "...-"},
  {'w', ".--"},
  {'x', "-..-"},
  {'y', "-.--"},
  {'z', "--.."}
};

uint8_t dot = 16;
uint8_t dah = 48;

//uint8_t morseSignal[];

String morseWord = "sos";       // String to place the morse word in.
String morseTranslated; // String to place the translated morse code in.

void convertWordToMorse(String wordGiven) {
  morseTranslated = "";
  int len = wordGiven.length();
  for (int i = 0; i < len; i++) {
    char morseCharacter = wordGiven.charAt(i);
    for (int j = 0; j < sizeof morseDictionary / sizeof morseDictionary[0]; j++) {
      if (morseCharacter == morseDictionary[j].morseChar) {
        morseTranslated = morseTranslated + morseDictionary[j].morseCode;
        if (i < len - 1) morseTranslated = morseTranslated + " ";
        break;
      }
    }
  }
};

void convertMorseToSignalArray(String morseGiven) {
  morseGiven.replace(" ", "");
  int len = morseGiven.length();
  uint8_t morseSignal[len];
  int k = 0;
  for (int i = 0; i < len; i++) {
    char morseCharacter = morseGiven.charAt(i);
    if (morseCharacter == '.') morseSignal[k] = dot;
    if (morseCharacter == '-') morseSignal[k] = dah;
    k++;
  }
};

void convertMorseToRestArray(String morseGiven) {
  int k = 0;
  String a = morseGiven;
  a.replace(" ", "");
  int len = a.length() - 1;
  uint8_t restSignal[len];
  for (int i = 1; i < morseGiven.length(); i++) {
    if (morseGiven.charAt(i) != ' ' && morseGiven.charAt(i - 1) != ' ') {
      restSignal[k] = dot;
      k++;
    }
    if (morseGiven.charAt(i) == ' ') {
      restSignal[k] = dah;
      k++;
    }
  }
};

void setup() {
  // Start Button Pin and Switch Intialization.
  pinMode(Morse_Pin, INPUT_PULLUP);
  morseSwitch.attach(Morse_Pin);
  morseSwitch.interval(20);
  
  //Setup a Serial Connection.
  Serial.begin(9600);
}

void loop() {
  if (morseSwitch.fell()) {
    Serial.println("Morse start.");
  }
}
