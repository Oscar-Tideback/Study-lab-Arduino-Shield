//  
//  Oscar Tidebäck
//  Labb 4. Ellära & Embedded
//
//    Något grovhuggen och kanske tjock :) 
//    Vikten kan minskas genom rader kod och design.
//    Koden fungerar som den är formulerad i labb 4.
//    Här finns också "fula" och osäkra lösnigar som bör göras bättre.
//    Dessutom finns troligen tankevurpor iform av buggar
//    Det blev Quik 'n' Dirty..
//    Inga felfall accepteras vid användning så.. :D
//

// Output
#define BLUE 9                                    // LED connected to digital pin 9, 10, 11
#define GREEN 10
#define RED 11

// Input
#define BUTTON_1 2                                // Buttons and other input device
#define BUTTON_2 12
#define POTENTIOMETER A0

const unsigned int MAX_MESSAGE_LENGTH = 10;       // max char of serial message

int globalState = 0;                              // state of "state machine"
int news = 0;                                     // global news

// Button 1 "interupt" 
int button1CurrentState = 0;                      // 0 = off, 1 = red, 2 = green, 3 = blue.
unsigned long button1LastChange;
bool interuptDeBounceIsActive = false;
// Button 2
int button2CurrentState = 0;
int button2LastState = 0;
unsigned long button2LastChange;
bool deBounceIsActive = false;

// Fade color values for rainbow effect on LED
int fadeToColor = 0;
int saturation = 0;
int blueValue = 255, greenValue = 255, redValue = 255;
int potDelay = 1;

// Timing
unsigned long lastFadeTime = 0;
unsigned long currentFadeTime;
typedef struct TimerStruct {
  unsigned long lastTime = 0;
  unsigned int delayTime = 10;
} timerStruct;

timerStruct *deBounceButton;

void setup() {
  Serial.begin(9600);
  Serial.println("Menu choice: red, green, blue, rainbow, off or menu");
  Serial.println("Set potentiometer values write pot-0 to pot-9");

  pinMode(RED, OUTPUT);
  pinMode(GREEN, OUTPUT);
  pinMode(BLUE, OUTPUT);

  pinMode(BUTTON_1, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(BUTTON_1), button1Pressed, FALLING);

  pinMode(BUTTON_2, INPUT);
  pinMode(POTENTIOMETER, INPUT);
}

void loop() {
  buttonHandler();                                           // Check for button1 actions
  if (interuptDeBounceIsActive == true) {                    // Check for button2 actions
    news = 2;
    interuptDeBounceIsActive = false;
  }
  potentionmeterHandler();                                    // Set new potentiometer value
  if (Serial.available() > 0) {                              // Check for UART/Serial messages
    startInputUART();
  }
  stateChanger();
  stateExecuter();
}

void buttonHandler() {
  int button2CurrentState = digitalRead(BUTTON_2);            // Get button2 pin state
  if (deBounceIsActive = false) {                             // Check if button is "not in change"
    if (button2CurrentState == 1 && button2LastState == 0) {  // Check if button has changed
      deBounceButton->lastTime = millis();                    // Set timestamp for bounce
      deBounceIsActive = true;                                // Set button to "is in change"
    }
  }
  else {
    if (timerDelay(*deBounceButton)) {                        // Wait for button to settle
      deBounceIsActive = false;                               // Set button to "not in change"
      button2CurrentState = digitalRead(BUTTON_2);            // Get button2 pin state
      if (button2CurrentState != button2LastState) {          // Check if button has changed
        news = 1;                                             // Post news to stateChanger
      }
    }
  }
  button2LastState = button2CurrentState;                      // Reset button state
}

void potentionmeterHandler() {
  if (potDelay != analogRead(POTENTIOMETER) && globalState == 1) {  // Check if golbalstate is in rainbow mode (not in UART mode)
    potDelay = analogRead(POTENTIOMETER);                           // Set new potentiometer value for rainbow fade
  }
}

bool timerDelay(timerStruct time) {                             // Check if then is now
  unsigned long now = millis();                                 // Set now to this time
  if ((now - time.lastTime) >= time.delayTime) {                // Check if the delay time has elapsed
    time.lastTime = now;                                        // Reset time
    return true;                                                // Delay time has elapsed
  }
  return false;                                                 // Delay time has not elapsed
}

void button1Pressed() {                                         // Interupt function for button1
  deBounceButton->lastTime = micros(); 
  if (timerDelay(*deBounceButton)) {
    interuptDeBounceIsActive = true;                            // Set to button has been pushed
  }
}

void stateChanger() {                                           // Check if some news has been posted by buttons and set the state of the "statemachine"
  if (news != 0) {
    if (news == 1) {                                            // Button1 has some news
      globalState = 1;                                          // Update change to statemachine
    }
    else if (news == 2) {                                       // Button2 has some news
      if (button1CurrentState < 3) {
        button1CurrentState++;                                  // Circle though 4 sets of color settings
      }
      else {
        button1CurrentState = 0;                                // Reset to first of 4 settings
      }
      globalState = 0;                                          // Update change to statemachine
    }
    else if (news == 3) {                                       // UART has some news for rainbow with potentiometer settings
      globalState = 2;
    }
    news = 0;                                                   // Reset news to none
  }
}

void stateExecuter() {                                          // Execut globalstate of statemachine
  switch (globalState) {
    case 0:                                                     // Set LED to red, green, blue or no light
      RGBLight(button1CurrentState);
      break;
    case 1:                                                     // Set LED in rainbow mode
      rainBow();
      break;
    case 2:                                                     // Set LED in rainbow mode with globalstate 2 that sets aside the physical potentiometer and sets pot value via UART
      rainBow();
      break;
  }
}
 
void rainBow() {                                                // LED in rainbow mode
  int localPotDelay = 1 + potDelay * 5;                         // Calibrate potentiometer value for timer
  currentFadeTime = micros();                                   // Timestamp for delay of the fadeing effect
  if ((currentFadeTime - lastFadeTime) >= localPotDelay) {      // Check if it's time to update LED
    if (fadeToColor == 0) {                                     // Color to fade to
      redValue--;
      greenValue++;
      analogWrite(RED, 255 - redValue);
      analogWrite(GREEN, 255 - greenValue);
      analogWrite(BLUE, 255);
      saturation++;
      if (saturation == 255) {                                  // Check if color is in full shine
        saturation = 0;                                         // Reset shine value
        fadeToColor = 1;                                        // Switch color to fade to
      }
    }
    if (fadeToColor == 1) {
      greenValue--;
      blueValue++;
      analogWrite(GREEN, 255 - greenValue);
      analogWrite(BLUE, 255 - blueValue);
      analogWrite(RED, 255);
      saturation++;
      if (saturation == 255) {
        saturation = 0;
        fadeToColor = 2;
      }
    }
    if (fadeToColor == 2) {
      redValue++;
      blueValue--;
      analogWrite(RED, 255 - redValue);
      analogWrite(BLUE, 255 - blueValue);
      analogWrite(GREEN, 255);
      saturation++;
      if (saturation == 255) {
        saturation = 0;
        fadeToColor = 0;
      }
    }
    lastFadeTime = currentFadeTime;
  }
}

void RGBLight(int state) {                                      // LED light. button1State = off, red, green or blue.
  switch (state) {
    case 0:                                                     // Check what color mode to set LED in
      digitalWrite(BLUE, 1);
      digitalWrite(GREEN, 1);
      digitalWrite(RED, 1);
      break;
    case 1:
      digitalWrite(BLUE, 1);
      digitalWrite(GREEN, 1);
      digitalWrite(RED, 0);
      break;
    case 2:
      digitalWrite(BLUE, 1);
      digitalWrite(GREEN, 0);
      digitalWrite(RED, 1);
      break;
    case 3:
      digitalWrite(BLUE, 0);
      digitalWrite(GREEN, 1);
      digitalWrite(RED, 1);
      break;
  }
}

void startInputUART() {                                               // Quik n Dirty 
  static char message[MAX_MESSAGE_LENGTH];                            // Create a place to hold the incoming message
  static unsigned int messagePos = 0;                                 // Index of message array
  while (Serial.available() > 0) {                                    // Check if message available
    char inByte = Serial.read();                                      // Read available byte in the serial receive buffer
    if ( inByte != '\n' && (messagePos < MAX_MESSAGE_LENGTH - 1) ) {  // Check message for null termination and length not to long
      message[messagePos] = inByte;                                   // Add the incoming byte to message
      messagePos++;                                                   // Next byte
    }
    else {                                                            // Full message received
      message[messagePos] = '\0';                                     // Null terminate string
      if (strcmp(message, "red") == 0) {                              // Find menu choices if availible in incoming message
        Serial.println("Red LED ON");
        button1CurrentState = 0;                                      // Set LED to red
        news = 2;                                                     // Post news to stateChanger
      }
      else if (strcmp(message, "green") == 0) {
        Serial.println("Green LED ON");
        button1CurrentState = 1;
        news = 2;
      }
      else if (strcmp(message, "blue") == 0) {
        Serial.println("Blue LED ON");
        button1CurrentState = 2;
        news = 2;
      }
      else if (strcmp(message, "off") == 0) {
        Serial.println("LED OFF");
        button1CurrentState = 3;
        news = 2;
      }
      else if (strcmp(message, "rainbow") == 0) {
        Serial.println("Rainbow effect ON");
        news = 1;
      } // Rainbow hack..
      else if (message[0] == 'p' && message[1] == 'o' && message[2] == 't' && message[3] == '-') { // Check for pot- message
        potDelay = message[4] - '0';                                // Convert fifth char to int
        potDelay = potDelay * 50;                                   // Calibrate to rainbow fade effect
        news = 3;                                                   // Post news about it
        Serial.println("Rainbow effect ON");
        Serial.println("Setting potentiometer to:");
        Serial.println(potDelay);
      }
      else if (strcmp(message, "menu") == 0) {
        Serial.println("Menu choice: red, green, blue, rainbow, off or menu");
        Serial.println("Set potentiometer values write pot-0 to pot-9");
      }
      else {
        Serial.println(message);
        Serial.println("is not a menu choice. please typ menu");
      }
      messagePos = 0;                                                // Reset UART/Serial message index
    }
  }
