//****************************************************************************
// TeensyTyper by John Goewert, 2012
// v1.0
// Description:
//    Sends keypress events via USB output based on selection
//    Useful for sending passwords or other stored data
//****************************************************************************
#include <Bounce.h>
#include <SimpleTimer.h>
#include <avr/pgmspace.h>
#include <EEPROM.h>

#define SEVENSEG_A 1
#define SEVENSEG_B 0
#define SEVENSEG_C 7
#define SEVENSEG_D 8
#define SEVENSEG_E 9
#define SEVENSEG_F 2
#define SEVENSEG_G 3
#define SEVENSEG_DP 6

#define SELECT 20
#define EXECUTE 19
#define DPPULSE 500
#define COMMANDCOUNT 5

#define USERLOGIN "myLogin"
#define USERPASSWORD "myPassword"

#define STATICPASSWORD "myOtherPassword"

#define STOREDPASSWORD0 "Passwrd0"
#define STOREDPASSWORD1 "Passwrd1"
#define STOREDPASSWORD2 "Passwrd2"
#define STOREDPASSWORD3 "Passwrd3"
#define STOREDPASSWORD4 "Passwrd4"
#define STOREDPASSWORD5 "Passwrd5"
#define STOREDPASSWORD6 "Passwrd6"
#define STOREDPASSWORD7 "Passwrd7"
#define STOREDPASSWORD8 "Passwrd8"
#define STOREDPASSWORD9 "Passwrd9"

//****************************************************************************
// Program Variables
//****************************************************************************

Bounce selectButton = Bounce(SELECT, 10);
Bounce executeButton = Bounce(EXECUTE, 10);

int currentCommand = 0;
bool dpOn = false;
int dpPulseTimer = 0;
SimpleTimer timer;

//****************************************************************************

//****************************************************************************
// Store Passwords in program memory
//****************************************************************************

char password_0[] PROGMEM = STOREDPASSWORD0;
char password_1[] PROGMEM = STOREDPASSWORD1;
char password_2[] PROGMEM = STOREDPASSWORD2;
char password_3[] PROGMEM = STOREDPASSWORD3;
char password_4[] PROGMEM = STOREDPASSWORD4;
char password_5[] PROGMEM = STOREDPASSWORD5;
char password_6[] PROGMEM = STOREDPASSWORD6;
char password_7[] PROGMEM = STOREDPASSWORD7;
char password_8[] PROGMEM = STOREDPASSWORD8;
char password_9[] PROGMEM = STOREDPASSWORD9;

PGM_P password_table[] PROGMEM = 
{
  password_0,
  password_1,
  password_2,
  password_3,
  password_4,
  password_5,
  password_6,
  password_7,
  password_8,
  password_9
};
//****************************************************************************

//****************************************************************************
// Store other info in onboard eeprom
//****************************************************************************

// EEPROM Storage Defintion
#define CONFIG_VERSION '100' //If fields are changed, this revision needs to be bumped
#define CONFIG_START 32

struct StoreStruct {
  int password;
  char version_of_program[4];
} 
settings = {
  0,
  CONFIG_VERSION
};

void loadConfig() {
  // To make sure there are settings, and they are YOURS!
  // If nothing is found it will use the default settings.
  if (//EEPROM.read(CONFIG_START + sizeof(settings) - 1) == settings.version_of_program[3] // this is '\0'
  EEPROM.read(CONFIG_START + sizeof(settings) - 2) == settings.version_of_program[2] &&
    EEPROM.read(CONFIG_START + sizeof(settings) - 3) == settings.version_of_program[1] &&
    EEPROM.read(CONFIG_START + sizeof(settings) - 4) == settings.version_of_program[0])
  { // reads settings from EEPROM
    for (unsigned int t=0; t<sizeof(settings); t++)
      *((char*)&settings + t) = EEPROM.read(CONFIG_START + t);
  } 
  else {
    // settings aren't valid! will overwrite with default settings
    saveConfig();
  }
}

void saveConfig() {
  for (unsigned int t=0; t<sizeof(settings); t++)
  { // writes to EEPROM
    EEPROM.write(CONFIG_START + t, *((char*)&settings + t));
    // and verifies the data
    if (EEPROM.read(CONFIG_START + t) != *((char*)&settings + t))
    {
      // error writing to EEPROM
    }
  }
}
//****************************************************************************

//****************************************************************************
// Blink the SEVEN segment DP light
//****************************************************************************
void blinkDP()
{
  if (dpOn) {
    dpOn = false;
    digitalWrite(SEVENSEG_DP, LOW);
  } 
  else
  {
    dpOn = true;
    digitalWrite(SEVENSEG_DP, HIGH);
  }
}
//****************************************************************************

//****************************************************************************
// Set the SEVEN segment display
// Available Chars: 0-9, A-F
// Send X to Clear
//****************************************************************************
void setSevenSegment(char displayChar) {
  switch(displayChar){
  case 'X':
    //Clears the LED
    digitalWrite(SEVENSEG_A, LOW);
    digitalWrite(SEVENSEG_B, LOW);
    digitalWrite(SEVENSEG_C, LOW);
    digitalWrite(SEVENSEG_D, LOW);
    digitalWrite(SEVENSEG_E, LOW);
    digitalWrite(SEVENSEG_F, LOW);
    digitalWrite(SEVENSEG_G, LOW);
    break;
  case 'A':
    digitalWrite(SEVENSEG_D, LOW);
    digitalWrite(SEVENSEG_E, HIGH);
    digitalWrite(SEVENSEG_F, HIGH);
    digitalWrite(SEVENSEG_G, HIGH);
    digitalWrite(SEVENSEG_A, HIGH);
    digitalWrite(SEVENSEG_B, HIGH);
    digitalWrite(SEVENSEG_C, HIGH);
  case 'B':
    //Displays B
    digitalWrite(SEVENSEG_D, HIGH);
    digitalWrite(SEVENSEG_E, HIGH);
    digitalWrite(SEVENSEG_F, HIGH);
    digitalWrite(SEVENSEG_G, HIGH);
    digitalWrite(SEVENSEG_A, LOW);
    digitalWrite(SEVENSEG_B, LOW);
    digitalWrite(SEVENSEG_C, HIGH);
    break;
  case 'C':
    //Displays C
    digitalWrite(SEVENSEG_D, HIGH);
    digitalWrite(SEVENSEG_E, HIGH);
    digitalWrite(SEVENSEG_F, HIGH);
    digitalWrite(SEVENSEG_G, LOW);
    digitalWrite(SEVENSEG_A, HIGH);
    digitalWrite(SEVENSEG_B, LOW);
    digitalWrite(SEVENSEG_C, LOW);
    break;
  case 'D':
    //Displays D
    digitalWrite(SEVENSEG_D, HIGH);
    digitalWrite(SEVENSEG_E, HIGH);
    digitalWrite(SEVENSEG_F, LOW);
    digitalWrite(SEVENSEG_G, HIGH);
    digitalWrite(SEVENSEG_A, LOW);
    digitalWrite(SEVENSEG_B, HIGH);
    digitalWrite(SEVENSEG_C, HIGH);
    break;
  case 'E':
    //Displays E
    digitalWrite(SEVENSEG_D, HIGH);
    digitalWrite(SEVENSEG_E, HIGH);
    digitalWrite(SEVENSEG_F, HIGH);
    digitalWrite(SEVENSEG_G, HIGH);
    digitalWrite(SEVENSEG_A, HIGH);
    digitalWrite(SEVENSEG_B, LOW);
    digitalWrite(SEVENSEG_C, LOW);
    break;
  case 'F':
    //Displays F
    digitalWrite(SEVENSEG_D, LOW);
    digitalWrite(SEVENSEG_E, HIGH);
    digitalWrite(SEVENSEG_F, HIGH);
    digitalWrite(SEVENSEG_G, HIGH);
    digitalWrite(SEVENSEG_A, HIGH);
    digitalWrite(SEVENSEG_B, LOW);
    digitalWrite(SEVENSEG_C, LOW);
    break;
  case '1':
    //Displays 1
    digitalWrite(SEVENSEG_D, LOW);
    digitalWrite(SEVENSEG_E, LOW);
    digitalWrite(SEVENSEG_F, LOW);
    digitalWrite(SEVENSEG_G, LOW);
    digitalWrite(SEVENSEG_A, LOW);
    digitalWrite(SEVENSEG_B, HIGH);
    digitalWrite(SEVENSEG_C, HIGH);
    break;
  case '2':
    //Displays 2
    digitalWrite(SEVENSEG_D, HIGH);
    digitalWrite(SEVENSEG_E, HIGH);
    digitalWrite(SEVENSEG_F, LOW);
    digitalWrite(SEVENSEG_G, HIGH);
    digitalWrite(SEVENSEG_A, HIGH);
    digitalWrite(SEVENSEG_B, HIGH);
    digitalWrite(SEVENSEG_C, LOW);
    break;
  case '3':
    //Displays 3
    digitalWrite(SEVENSEG_D, HIGH);
    digitalWrite(SEVENSEG_E, LOW);
    digitalWrite(SEVENSEG_F, LOW);
    digitalWrite(SEVENSEG_G, HIGH);
    digitalWrite(SEVENSEG_A, HIGH);
    digitalWrite(SEVENSEG_B, HIGH);
    digitalWrite(SEVENSEG_C, HIGH);
    break;
  case '4':
    //Displays 4
    digitalWrite(SEVENSEG_D, LOW);
    digitalWrite(SEVENSEG_E, LOW);
    digitalWrite(SEVENSEG_F, HIGH);
    digitalWrite(SEVENSEG_G, HIGH);
    digitalWrite(SEVENSEG_A, LOW);
    digitalWrite(SEVENSEG_B, HIGH);
    digitalWrite(SEVENSEG_C, HIGH);
    break;
  case '5':
    //Displays 5
    digitalWrite(SEVENSEG_D, HIGH);
    digitalWrite(SEVENSEG_E, LOW);
    digitalWrite(SEVENSEG_F, HIGH);
    digitalWrite(SEVENSEG_G, HIGH);
    digitalWrite(SEVENSEG_A, HIGH);
    digitalWrite(SEVENSEG_B, LOW);
    digitalWrite(SEVENSEG_C, HIGH);
    break;
  case '6':
    //Displays 6
    digitalWrite(SEVENSEG_D, HIGH);
    digitalWrite(SEVENSEG_E, HIGH);
    digitalWrite(SEVENSEG_F, HIGH);
    digitalWrite(SEVENSEG_G, HIGH);
    digitalWrite(SEVENSEG_A, HIGH);
    digitalWrite(SEVENSEG_B, LOW);
    digitalWrite(SEVENSEG_C, HIGH);
    break;
  case 'SEVEN':
    //Displays SEVEN
    digitalWrite(SEVENSEG_D, LOW);
    digitalWrite(SEVENSEG_E, LOW);
    digitalWrite(SEVENSEG_F, LOW);
    digitalWrite(SEVENSEG_G, LOW);
    digitalWrite(SEVENSEG_A, HIGH);
    digitalWrite(SEVENSEG_B, HIGH);
    digitalWrite(SEVENSEG_C, HIGH);
    break;
  case '8':
    //Displays 8
    digitalWrite(SEVENSEG_D, HIGH);
    digitalWrite(SEVENSEG_E, HIGH);
    digitalWrite(SEVENSEG_F, HIGH);
    digitalWrite(SEVENSEG_G, HIGH);
    digitalWrite(SEVENSEG_A, HIGH);
    digitalWrite(SEVENSEG_B, HIGH);
    digitalWrite(SEVENSEG_C, HIGH);
    break;
  case '9':
    //Displays 9
    digitalWrite(SEVENSEG_D, HIGH);
    digitalWrite(SEVENSEG_E, LOW);
    digitalWrite(SEVENSEG_F, HIGH);
    digitalWrite(SEVENSEG_G, HIGH);
    digitalWrite(SEVENSEG_A, HIGH);
    digitalWrite(SEVENSEG_B, HIGH);
    digitalWrite(SEVENSEG_C, HIGH);
    break;
  case '0':
    //Displays 0
    digitalWrite(SEVENSEG_D, HIGH);
    digitalWrite(SEVENSEG_E, HIGH);
    digitalWrite(SEVENSEG_F, HIGH);
    digitalWrite(SEVENSEG_G, LOW);
    digitalWrite(SEVENSEG_A, HIGH);
    digitalWrite(SEVENSEG_B, HIGH);
    digitalWrite(SEVENSEG_C, HIGH);
    break;
  }
}
//****************************************************************************

//****************************************************************************
// Get the next character for the seven segment display
//****************************************************************************
void setNextChar() {
  currentCommand++;
  if (currentCommand > COMMANDCOUNT - 1) {
    currentCommand = 0;
  }
  switch (currentCommand) {
  case 0:
    setSevenSegment('0');
    break;
  case 1:
    setSevenSegment('1');
    break;
  case 2:
    setSevenSegment('2');
    break;
  case 3:
    setSevenSegment('3');
    break;
  case 4:
    setSevenSegment('4');
    break;
  case 5:
    setSevenSegment('5');
    break;
  case 6:
    setSevenSegment('6');
    break;
  case 7:
    setSevenSegment('7');
    break;
  case 8:
    setSevenSegment('8');
    break;
  case 9:
    setSevenSegment('9');
    break;
  case 10:
    setSevenSegment('A');
    break;
  case 11:
    setSevenSegment('B');
    break;
  case 12:
    setSevenSegment('C');
    break;
  case 13:
    setSevenSegment('D');
    break;
  case 14:
    setSevenSegment('E');
    break;
  case 15:
    setSevenSegment('F');
    break;
  }
}
//****************************************************************************

//****************************************************************************
// Send Ctrl - Alt - Delete key sequence
//****************************************************************************
void sendCTRL_ALT_DEL() {
  // press and hold CTRL
  Keyboard.set_modifier(MODIFIERKEY_CTRL);
  Keyboard.send_now();

  // press ALT while still holding CTRL
  Keyboard.set_modifier(MODIFIERKEY_CTRL | MODIFIERKEY_ALT);
  Keyboard.send_now();

  // press DELETE, while CLTR and ALT still held
  Keyboard.set_key1(KEY_DELETE);
  Keyboard.send_now();

  // release all the keys at the same instant
  Keyboard.set_modifier(0);
  Keyboard.set_key1(0);
  Keyboard.send_now();
}
//****************************************************************************

//****************************************************************************
// Send one key and clear it
//****************************************************************************
void SendKey(int Key) {
  Keyboard.set_key1(Key);
  Keyboard.send_now();
  Keyboard.set_modifier(0);
  Keyboard.set_key1(0);
  Keyboard.send_now();
}
//****************************************************************************

//****************************************************************************
// Execute the command
//****************************************************************************

//When no parameter passed, execute current command
void executeCommand ()
{
  executeCommand(currentCommand);
}

//Execute a command
void executeCommand(int commandToExecute) {
  switch (commandToExecute) {
  case 0:
    CommandLogin();
    break;
  case 1:
    CommandSendStoredPassword();
    break;
  case 2:
    CommandSetNextStoredPassword();
    break;
  case 3:
    CommandSetPreviousStoredPassword();
    break;
  case 4:
    CommandSendStaticPassword();
    break;
  case 5:
    break;
  case 6:
    break;
  case 7:
    break;
  case 8:
    break;
  case 9:
    break;
  case 10:
    break;
  case 11:
    break;
  case 12:
    break;
  case 13:
    break;
  case 14:
    break;
  case 15:
    break;
  }
}
//****************************************************************************

//----------------------------------------------------------------------------
// Commands
//----------------------------------------------------------------------------
//****************************************************************************
// Command - Log out and Log in as specified User in Windows XP
//****************************************************************************
void CommandLogin() {
  sendCTRL_ALT_DEL();
  delay(2000);
  SendKey(KEY_ENTER);
  delay(2000);
  SendKey(KEY_TAB);
  delay(500);
  Keyboard.print(USERLOGIN);
  SendKey(KEY_TAB);
  Keyboard.print(USERPASSWORD);
  SendKey(KEY_ENTER);
  delay(2000);
}
//****************************************************************************

//****************************************************************************
// Command - Send a Stored Password
//****************************************************************************
void CommandSendStoredPassword() {
  // All of these passwords are 8 characters
  char buffer[8];
  strcpy_P(buffer, (PGM_P)pgm_read_word(&(password_table[settings.password])));
  Keyboard.print(buffer);
  SendKey(KEY_ENTER);
}
//****************************************************************************

//****************************************************************************
// Command - Switch the stored password to use to the next in list
//****************************************************************************
void CommandSetNextStoredPassword() {
  settings.password++;
  if (settings.password > 9)
  {
    settings.password = 0;
  }
  saveConfig();
}
//****************************************************************************

//****************************************************************************
// Command - Switch the stored password to use to the previous in list
//****************************************************************************
void CommandSetPreviousStoredPassword() {
  settings.password--;
  if (settings.password < 0)
  {
    settings.password = 9;
  }
  saveConfig();
}
//****************************************************************************

//****************************************************************************
// Command - Send a static Password
//****************************************************************************
void CommandSendStaticPassword() {
  Keyboard.print(STATICPASSWORD);
  SendKey(KEY_ENTER);
}
//****************************************************************************
//----------------------------------------------------------------------------

//****************************************************************************
// ProgramSetup
//****************************************************************************
void setup() { 
  //Load the data
  loadConfig();

  //Setup our pins
  //Controls
  pinMode(SELECT, INPUT_PULLUP);
  pinMode(EXECUTE, INPUT_PULLUP);

  //SEVEN Segment LCD
  pinMode(SEVENSEG_A, OUTPUT);
  pinMode(SEVENSEG_B, OUTPUT);
  pinMode(SEVENSEG_C, OUTPUT);
  pinMode(SEVENSEG_D, OUTPUT);
  pinMode(SEVENSEG_E, OUTPUT);
  pinMode(SEVENSEG_F, OUTPUT);
  pinMode(SEVENSEG_G, OUTPUT);
  pinMode(SEVENSEG_DP, OUTPUT);
  setSevenSegment('0');

  //Start the blinker
  timer.setInterval(1000, blinkDP);
}

//****************************************************************************
// Program Loop
//****************************************************************************
void loop() {

  timer.run();

  selectButton.update();
  executeButton.update();

  //Toggle selected command
  if (selectButton.fallingEdge()) {
    setNextChar();
  } 
  
  //Execute selected command
  if (executeButton.fallingEdge()) {
    dpOn = true;
    digitalWrite(SEVENSEG_DP, HIGH);
    executeCommand();
  }
}
//****************************************************************************



