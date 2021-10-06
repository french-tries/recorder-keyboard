#include <BleKeyboard.h>

#define NUM_BUTTONS 10
#define NUM_NOTE_BUTTONS 9
#define MOD_BUTTON_ID 9

// A3 is defined in esp32 arduino.h ...
// enum Note{N_F = 0, N_G, N_A, N_Bb, N_B, N_C, N_Cs, N_D, N_Eb, N_E, N_F2, N_F2s,
//   N_G2, N_G2s, N_A2, N_B2b, N_B2, N_C2, N_C2s, N_D2, N_D2s, N_E2, N_F3, N_G3, N_G3s, N_A3, N_B3, N_C3, NUM_NOTES};

const char buttonsPin[NUM_BUTTONS] = {34, 35, 32, 33, 25, 26, 14, 12, 13, 27};    // the number of the flute button pins

const char fingerings[NUM_NOTES][NUM_NOTE_BUTTONS] = 
{
   {1, 0, 1, 1, 1, 1, 1, 1, 1}, // N_F
   {1, 0, 1, 1, 1, 1, 1, 1, 0}, // N_G
   {1, 0, 1, 1, 1, 1, 1, 0, 0}, // N_A
   {1, 0, 1, 1, 1, 1, 0, 1, 1}, // N_Bb
   {1, 0, 1, 1, 1, 0, 1, 1, 0}, // N_B
   {1, 0, 1, 1, 1, 0, 0, 0, 0}, // N_C
   {1, 0, 1, 1, 0, 1, 1, 0, 0}, // N_Cs
   {1, 0, 1, 1, 0, 0, 0, 0, 0}, // N_D
   {1, 0, 1, 0, 1, 1, 0, 0, 0}, // N_Eb
   {1, 0, 1, 0, 0, 0, 0, 0, 0}, // N_E
   {1, 0, 0, 1, 0, 0, 0, 0, 0}, // N_F2
   {0, 0, 1, 1, 0, 0, 0, 0, 0}, // N_F2s
   {0, 0, 0, 1, 0, 0, 0, 0, 0}, // N_G2
   {0, 0, 0, 1, 1, 1, 1, 1, 0}, // N_G2s
   {0, 1, 1, 1, 1, 1, 1, 0, 0}, // N_A2
   {0, 1, 1, 1, 1, 1, 0, 1, 0}, // N_B2b
   {0, 1, 1, 1, 1, 0, 1, 0, 0}, // N_B2
   {0, 1, 1, 1, 1, 0, 0, 0, 0}, // N_C2
   {0, 1, 1, 1, 0, 1, 0, 0, 0}, // N_C2s
   {0, 1, 1, 1, 0, 0, 0, 0, 0}, // N_D2
   {0, 1, 1, 1, 0, 0, 1, 1, 1}, // N_D2s
   {0, 1, 1, 1, 0, 1, 1, 0, 0}, // N_E2
   {0, 1, 1, 0, 0, 1, 1, 0, 0}, // N_F3
   {0, 1, 1, 0, 1, 1, 0, 1, 1}, // N_G3
   {0, 1, 0, 1, 1, 0, 0, 0, 0}, // N_G3s
   {0, 1, 0, 1, 1, 0, 1, 1, 0}, // N_A3
   {0, 1, 1, 1, 0, 1, 1, 0, 0}, // N_B3
   {0, 1, 1, 0, 0, 1, 0, 0, 0} // N_C3
}; 

const NoteMap keys[NUM_NOTES] =
{
  {'a', 'b'}, // N_F
  {'c', 'd'}, // N_G
  {'e', 'f'}, // N_A
  {'g', 'h'}, // N_Bb
  {'i', 'j'}, // N_B
  {'k', 'l'}, // N_C
  {'m', 'n'}, // N_Cs
  {'o', 'p'}, // N_D
  {'q', 'r'}, // N_Eb
  {'s', 't'}, // N_E
  {'u', 'v'}, // N_F2
  {'w', 'x'}, // N_F2s
  {'y', 'z'}, // N_G2
  {' ', '0'}, // N_G2s
  {'1', '2'}, // N_A2
  {'3', '4'}, // N_B2b
  {'5', '6'}, // N_B2
  {'7', '8'}, // N_C2
  {'9', '`'}, // N_C2s
  {',', '.'}, // N_D2
  {'/', ';'}, // N_D2s
  {'\'', '\\'}, // N_E2
  {'[', ']'}, // N_F3
  {'-', '='}, // N_G3
  {' ', ' '}, // N_G3s
  {' ', ' '}, // N_A3
  {' ', ' '}, // N_B3
  {' ', ' '} // N_C3
};

struct NoteMap {
   const char key;
   const char modified;
};

// Variables will change:
char reading[NUM_BUTTONS]; 
char buttonState[NUM_BUTTONS];             // the current reading from the input pins
char lastButtonState[NUM_BUTTONS] = {LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW};   // the previous reading from the input pins

// the following variables are long's because the time, measured in miliseconds,
// will quickly become a bigger number than can be stored in an int.
long lastDebounceTime = 0;  // the last time one of the outputs pin was toggled
long debounceDelay = 150;    // the debounce time; increase if the output flickers

BleKeyboard bleKeyboard;

boolean same(const unsigned int len, const char* first, const char* second)
{
  for (int i = 0; i < len; ++i)
  {
    if (first[i] != second[i])
    {
      return false;
    }
  }
  return true;
}

boolean opposites(const unsigned int len, const char* first, const char* second)
{
  for (int i = 0; i < len; ++i)
  {
    if (first[i] == second[i])
    {
      return false;
    }
  }
  return true;
}

int findNote(const char buttons[NUM_BUTTONS])
{
  for (int i = 0; i < NUM_NOTES; ++i)
  {
    if (opposites(NUM_NOTE_BUTTONS, buttons, fingerings[i]))
    {
      return i;
    }
  }
  return -1;
}

char getKey(int note, bool modified)
{
  if (modified)
  {
    return keys[note].modified;
  }
  return keys[note].key;
}

void setup()
{
  for (int i = 0; i < NUM_BUTTONS; ++i)
  {
    pinMode(buttonsPin[i], INPUT_PULLUP);
  }

  // start serial port at 9600 bps:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }
  
  // initialize control over the keyboard:
  bleKeyboard.begin();
}

void loop()
{
  // read the state of the switch into a local variable:
  for (int i = 0; i < NUM_BUTTONS; ++i)
  {
    reading[i] = digitalRead(buttonsPin[i]);
  }

  // check to see if you just pressed the button 
  // (i.e. the input went from LOW to HIGH),  and you've waited 
  // long enough since the last press to ignore any noise:  

  // If the switch changed, due to noise or pressing:
  if (!same(NUM_BUTTONS, reading, lastButtonState)) {
    // reset the debouncing timer
    lastDebounceTime = millis();
  } 
  
  if ((millis() - lastDebounceTime) > debounceDelay) {
    // whatever the reading is at, it's been there for longer
    // than the debounce delay, so take it as the actual current state:

    // if the button state has changed:
    if (!same(NUM_BUTTONS, reading, buttonState)) {
      memcpy(buttonState, reading, NUM_BUTTONS);

      // find the note
      int note = findNote(buttonState);
      if (note >= 0)
      {
        char key = getKey(note, !buttonState[MOD_BUTTON_ID]);
        
        bleKeyboard.print(key);
        Serial.println(key);
      }
    }
  }

  // save the reading.  Next time through the loop,
  // it'll be the lastButtonState:
  memcpy(lastButtonState, reading, NUM_BUTTONS);
}
