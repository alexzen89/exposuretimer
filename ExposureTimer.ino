// include the library code:
#include <LiquidCrystal.h>
#include <CountUpDownTimer.h>

#define ENC_LIMIT_MAX 3599

// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(A5, A4, A3, A2, A1, A0);
CountUpDownTimer MyTimer(DOWN);

char timer1[8];
char timer2[] = {"        "};
char encoder[3];
char once = 1;
bool buttonPressed = 0;
byte timerHasStarted = 0;

byte menuLevel        = 0xFF;
byte menuLevelPrev    = 0xFF;

/***** DEBOUNCE STUFF HERE ******/
int lastButtonState = HIGH;   // the previous reading from the input pin
int buttonState;             // the current reading from the input pin
// the following variables are long's because the time, measured in miliseconds,
// will quickly become a bigger number than can be stored in an int.
long lastDebounceTime = 0;  // the last time the output pin was toggled
long debounceDelay = 50;    // the debounce time; increase if the output flickers
int buttonCnt, buttonRead;
/*******************************/

/***** ENCODER STUFF HERE ******/
static int pinA = 2; // Our first hardware interrupt pin is digital pin 2
static int pinB = 3; // Our second hardware interrupt pin is digital pin 3
volatile byte aFlag = 0; // let's us know when we're expecting a rising edge on pinA to signal that the encoder has arrived at a detent
volatile byte bFlag = 0; // let's us know when we're expecting a rising edge on pinB to signal that the encoder has arrived at a detent (opposite direction to when aFlag is set)
volatile int encoderPos; //this variable stores our current value of encoder position. Change to int or uin16_t instead of byte if you want to record a larger range than 0-255
volatile int oldEncPos; //stores the last encoder position value so we can compare to the current reading and see if it has changed (so we know when to print to the serial monitor)
volatile byte reading = 0; //somewhere to store the direct values we read from our interrupt pins before checking to see if we have moved a whole detent
/*******************************/

void setup() {
  pinMode(2, INPUT_PULLUP);
  pinMode(3, INPUT_PULLUP);
  pinMode(4, INPUT_PULLUP);
  attachInterrupt(0,PinA,RISING); // set an interrupt on PinA, looking for a rising edge signal and executing the "PinA" Interrupt Service Routine (below)
  attachInterrupt(1,PinB,RISING); // set an interrupt on PinB, looking for a rising edge signal and executing the "PinB" Interrupt Service Routine (below)
  // put your setup code here, to run once:
  //MyTimer.SetTimer(0,3,50);
//  MyTimer.SetTimer(0,0,10);
//  MyTimer.StartTimer();
  // set up the LCD's number of columns and rows:
  lcd.begin(12, 2);
  // Print a message to the LCD.
  lcd.setCursor(0, 0);
  lcd.print("  Exposure  ");
  lcd.setCursor(0, 1);
  lcd.print("    Timer   ");
  delay(2000);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("   Version  ");
  lcd.setCursor(0, 1);
  lcd.print("  v1.00b001 ");
  delay(2000);
  lcd.clear();
  sprintf(timer1,"%02d:%02d:%02d", 0, encoderPos/60, encoderPos%60);
}
void clearLcd()
{
  lcd.setCursor(0, 0);
  lcd.print("        ");
  lcd.setCursor(0, 1);
  lcd.print("        ");
}
void PinA(){
  cli(); //stop interrupts happening before we read pin values
  reading = PIND & 0xC; // read all eight pin values then strip away all but pinA and pinB's values
  if(reading == B00001100 && aFlag) { //check that we have both pins at detent (HIGH) and that we are expecting detent on this pin's rising edge
//    if (encoderPos < ENC_LIMIT_MAX)
      encoderPos ++; //increment the encoder's position count
//    else
//      encoderPos = 0;
    bFlag = 0; //reset flags for the next turn
    aFlag = 0; //reset flags for the next turn
  }
  else if (reading == B00000100) bFlag = 1; //signal that we're expecting pinB to signal the transition to detent from free rotation
  sei(); //restart interrupts
}

void PinB(){
  cli(); //stop interrupts happening before we read pin values
  reading = PIND & 0xC; //read all eight pin values then strip away all but pinA and pinB's values
  if (reading == B00001100 && bFlag) { //check that we have both pins at detent (HIGH) and that we are expecting detent on this pin's rising edge
//    if (encoderPos == 0)
//      encoderPos = ENC_LIMIT_MAX;
//    else  
      encoderPos --; //decrement the encoder's position count
    bFlag = 0; //reset flags for the next turn
    aFlag = 0; //reset flags for the next turn
  }
  else if (reading == B00001000) aFlag = 1; //signal that we're expecting pinA to signal the transition to detent from free rotation
  sei(); //restart interrupts
}

void loop() {
//  MyTimer.Timer(); // run the timer

  if(oldEncPos != encoderPos)
  {
    oldEncPos = encoderPos;
    clearLcd();
  }

  if (buttonPressed)
    menuLevel = menuLevelPrev;
  else
    menuLevel = encoderPos;    

  lcd.setCursor(0, 0);
  switch (menuLevel)
  {
    case 0:
      menuLevelPrev = 0;
      lcd.print("Set Timer   ");
      if (buttonPressed)
      {
        sprintf(timer1,"%02d:%02d:%02d", 0, encoderPos/60, encoderPos%60);
        MyTimer.SetTimer(0, encoderPos/60, encoderPos%60);
      }
      break;
    case 1:
      menuLevelPrev = 1;
      if (buttonPressed)
      {
        lcd.print("Stop        ");
        MyTimer.Timer(); // run the timer
        timerHasStarted = 1;
        if (once)
        {
          MyTimer.StartTimer();
          once = 0;
        }
      }
      else
      {             
        lcd.print("Start       ");
        MyTimer.StopTimer();
        MyTimer.ResetTimer();
        timerHasStarted = 0;
      }

      // print the number of seconds since reset:
      if (MyTimer.TimeHasChanged())
      {
        sprintf(timer1,"%02lu:%02lu:%02lu", MyTimer.ShowHours(), MyTimer.ShowMinutes(), MyTimer.ShowSeconds());
        clearLcd();
      }
      
      break;
    default:
      encoderPos = 0;
      break;
  }

/***** DEBOUNCE STUFF HERE ******/
  buttonRead = digitalRead(4);

  // check to see if you just pressed the button
  // (i.e. the input went from LOW to HIGH),  and you've waited
  // long enough since the last press to ignore any noise:

  // If the switch changed, due to noise or pressing:
  if (buttonRead != lastButtonState) 
  {
    // reset the debouncing timer
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) 
  {
    // whatever the reading is at, it's been there for longer
    // than the debounce delay, so take it as the actual current state:

    // if the button state has changed:
    if (buttonRead != buttonState) 
    {
      buttonState = buttonRead;

      // only toggle the LED if the new button state is HIGH
      if (buttonState == LOW) 
      {
        buttonCnt++;
        buttonPressed = !buttonPressed; 
        clearLcd();
      }
    }
  }

  // save the reading.  Next time through the loop,
  // it'll be the lastButtonState:
  lastButtonState = buttonRead; 
/********************************/

  // do something when the timer expires
  if (MyTimer.TimeCheck() && timerHasStarted)
  {
    clearLcd();
    lcd.setCursor(0, 0);
    lcd.print("  DONE  "); 
    lcd.setCursor(0, 1);
    lcd.print("PRESS A KEY"); 
  }
  else
  {
    lcd.setCursor(0, 1);
    lcd.print(timer1);
  }
}
