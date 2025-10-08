#include <Servo.h>

// --- Notes ---
#define NOTE_D4 294
#define NOTE_E4 330
#define NOTE_F4 349
#define NOTE_G4 392
#define NOTE_A4 440
#define NOTE_C5 523

// --- Choice bits ---
#define CHOICE_OFF      0
#define CHOICE_NONE     0
#define CHOICE_RED      (1 << 0)
#define CHOICE_BLUE     (1 << 1)
#define CHOICE_YELLOW   (1 << 2)
#define CHOICE_GREEN    (1 << 3)

// --- LED pins ---
#define LED_RED     3
#define LED_BLUE    5
#define LED_YELLOW  7
#define LED_GREEN   9

// --- Button pins ---
#define BUTTON_RED    2
#define BUTTON_BLUE   4
#define BUTTON_YELLOW 6
#define BUTTON_GREEN  8

// --- Buzzer pins ---
#define BUZZER1  10
#define BUZZER2  12

// --- Servo pin ---
Servo myservo;

// --- Game parameters ---
#define ROUNDS_TO_WIN    15
#define ENTRY_TIME_LIMIT 3000 // ms timeout

// --- Modes ---
#define MODE_MEMORY  0
#define MODE_BATTLE  1
#define MODE_BEEGEES 2

byte gameMode = MODE_MEMORY;
byte gameBoard[32];
byte gameRound = 0;

// --- Setup ---
void setup() {
  myservo.attach(11);
  myservo.write(90); // neutral

  // Setup buttons
  pinMode(BUTTON_RED, INPUT_PULLUP);
  pinMode(BUTTON_BLUE, INPUT_PULLUP);
  pinMode(BUTTON_YELLOW, INPUT_PULLUP);
  pinMode(BUTTON_GREEN, INPUT_PULLUP);

  // Setup LEDs
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);

  // Setup buzzers
  pinMode(BUZZER1, OUTPUT);
  pinMode(BUZZER2, OUTPUT);

  // --- Mode selection at startup ---
  byte pressed = checkButton();
  if (pressed == CHOICE_YELLOW) play_beegees();
  if (pressed == CHOICE_GREEN) {
    gameMode = MODE_BATTLE;
    setLEDs(CHOICE_GREEN);
    playTone(CHOICE_GREEN, 150);
    setLEDs(CHOICE_RED | CHOICE_BLUE | CHOICE_YELLOW);
    while (checkButton() != CHOICE_NONE);
  }

  play_winner(); // startup hello
}

// --- Loop ---
void loop() {
  attractMode();

  // Flash all LEDs to start
  setLEDs(CHOICE_RED | CHOICE_GREEN | CHOICE_BLUE | CHOICE_YELLOW);
  myservo.write(90);
  delay(1000);
  setLEDs(CHOICE_OFF);
  delay(250);

  if (gameMode == MODE_MEMORY) {
    if (play_memory()) play_winner();
    else play_loser();
  }

  if (gameMode == MODE_BATTLE) {
    play_battle();
    play_loser();
  }
}

// ---------------- GAME LOGIC ----------------

// --- Memory mode ---
boolean play_memory(void) {
  randomSeed(millis());
  gameRound = 0;

  while (gameRound < ROUNDS_TO_WIN) {
    add_to_moves();
    playMoves();

    // Wait for player to repeat
    for (byte i = 0; i < gameRound; i++) {
      byte choice = wait_for_button();
      if (choice == 0) return false; // timeout
      if (choice != gameBoard[i]) return false; // wrong
    }

    delay(500);
  }
  return true;
}

// --- Battle mode ---
boolean play_battle(void) {
  gameRound = 0;
  while (1) {
    byte newButton = wait_for_button();
    if (newButton == 0) return false;
    gameBoard[gameRound++] = newButton;

    for (byte i = 0; i < gameRound; i++) {
      byte choice = wait_for_button();
      if (choice == 0) return false;
      if (choice != gameBoard[i]) return false;
    }
    delay(100);
  }
  return true;
}

// --- Add new random move ---
void add_to_moves(void) {
  byte newButton = random(0, 4);
  if (newButton == 0) newButton = CHOICE_RED;
  else if (newButton == 1) newButton = CHOICE_GREEN;
  else if (newButton == 2) newButton = CHOICE_BLUE;
  else if (newButton == 3) newButton = CHOICE_YELLOW;

  gameBoard[gameRound++] = newButton;
}

// --- Replay all moves ---
void playMoves(void) {
  for (byte i = 0; i < gameRound; i++) {
    playTone(gameBoard[i], 150);
    delay(150);
  }
}

// ---------------- INPUT / OUTPUT ----------------

// --- LED control ---
void setLEDs(byte leds) {
  digitalWrite(LED_RED,    leds & CHOICE_RED);
  digitalWrite(LED_GREEN,  leds & CHOICE_GREEN);
  digitalWrite(LED_BLUE,   leds & CHOICE_BLUE);
  digitalWrite(LED_YELLOW, leds & CHOICE_YELLOW);
}

// --- Button reading ---
byte checkButton(void) {
  if (!digitalRead(BUTTON_RED))    return CHOICE_RED;
  if (!digitalRead(BUTTON_GREEN))  return CHOICE_GREEN;
  if (!digitalRead(BUTTON_BLUE))   return CHOICE_BLUE;
  if (!digitalRead(BUTTON_YELLOW)) return CHOICE_YELLOW;
  return CHOICE_NONE;
}

// --- Wait for press ---
byte wait_for_button(void) {
  long startTime = millis();
  while (millis() - startTime < ENTRY_TIME_LIMIT) {
    byte b = checkButton();
    if (b != CHOICE_NONE) {
      playTone(b, 150);
      while (checkButton() != CHOICE_NONE);
      delay(50); // debounce
      return b;
    }
  }
  return CHOICE_NONE;
}

// --- Play tone + LED (pleasant version) ---
void playTone(byte which, int duration_ms) {
  setLEDs(which);
  int freq = 0;
  switch (which) {
    case CHOICE_RED:    freq = NOTE_C5; break;
    case CHOICE_GREEN:  freq = NOTE_G4; break;
    case CHOICE_BLUE:   freq = NOTE_E4; break;
    case CHOICE_YELLOW: freq = NOTE_A4; break;
  }
  if (freq > 0) {
    tone(BUZZER2, freq, duration_ms);
    delay(duration_ms);
    noTone(BUZZER2);
  }
  setLEDs(CHOICE_OFF);
}

// ---------------- SOUND EFFECTS ----------------

// --- Winner ---
void play_winner(void) {
  myservo.write(180);
  setLEDs(CHOICE_GREEN | CHOICE_BLUE);
  playTone(CHOICE_GREEN, 150);
  playTone(CHOICE_BLUE, 150);
  setLEDs(CHOICE_RED | CHOICE_YELLOW);
  playTone(CHOICE_RED, 150);
  playTone(CHOICE_YELLOW, 150);
  myservo.write(90);
}

// --- Loser ---
void play_loser(void) {
  myservo.write(0);
  for (int i = 0; i < 2; i++) {
    setLEDs(CHOICE_RED | CHOICE_GREEN);
    playTone(CHOICE_RED, 200);
    playTone(CHOICE_GREEN, 200);
    setLEDs(CHOICE_BLUE | CHOICE_YELLOW);
    playTone(CHOICE_BLUE, 200);
    playTone(CHOICE_YELLOW, 200);
  }
  myservo.write(90);
}

// ---------------- ATTRACT MODE ----------------
void attractMode(void) {
  myservo.write(90);
  while (1) {
    setLEDs(CHOICE_RED); delay(200);
    if (checkButton() != CHOICE_NONE) return;
    setLEDs(CHOICE_BLUE); delay(200);
    if (checkButton() != CHOICE_NONE) return;
    setLEDs(CHOICE_GREEN); delay(200);
    if (checkButton() != CHOICE_NONE) return;
    setLEDs(CHOICE_YELLOW); delay(200);
    if (checkButton() != CHOICE_NONE) return;
  }
}

// ---------------- BEE GEES MODE ----------------
int melody[] = {
  NOTE_G4, NOTE_A4, 0, NOTE_C5, 0, 0, NOTE_G4, 0, 0, 0,
  NOTE_E4, 0, NOTE_D4, NOTE_E4, NOTE_G4, 0,
  NOTE_D4, NOTE_E4, 0, NOTE_G4, 0, 0,
  NOTE_D4, 0, NOTE_E4, 0, NOTE_G4, 0, NOTE_A4, 0, NOTE_C5, 0
};

int noteDuration = 115;
byte LEDnumber = 0;

void play_beegees() {
  setLEDs(CHOICE_YELLOW);
  playTone(CHOICE_YELLOW, 150);
  setLEDs(CHOICE_RED | CHOICE_GREEN | CHOICE_BLUE);
  while (checkButton() == CHOICE_NONE) {
    for (int i = 0; i < 32; i++) {
      changeLED();
      if (melody[i] != 0)
        tone(BUZZER2, melody[i], noteDuration);
      delay(noteDuration * 1.3);
      noTone(BUZZER2);
    }
  }
}

// --- Cycle LEDs correctly ---
void changeLED(void) {
  byte leds[] = {CHOICE_RED, CHOICE_GREEN, CHOICE_BLUE, CHOICE_YELLOW};
  setLEDs(leds[LEDnumber]);
  LEDnumber = (LEDnumber + 1) % 4;
}
