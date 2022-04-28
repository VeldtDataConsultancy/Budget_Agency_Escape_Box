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

String morseWord = "help";       // String to place the morse word in.
String morseTranslated; // String to place the translated morse code in.

void convertWordToMorse(String wordGiven) {
  morseTranslated = "";
  Serial.println(wordGiven);
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
  Serial.println(morseTranslated);
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

void setup() {
  //Setup a Serial Connection.
  Serial.begin(9600);

  // 1. Translate the given word into a morse code.
  convertWordToMorse(morseWord);

  // 2. Create a signal array depending on the morse code.
  convertMorseToSignalArray(morseTranslated);
}

void loop() {
}
