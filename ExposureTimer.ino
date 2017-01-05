// include the library code:
#include <LiquidCrystal.h>
#include <CountUpDownTimer.h>

#define ENC_LIMIT_MAX 3599

// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(A5, A4, A3, A2, A1, A0);
CountUpDownTimer MyTimer(DOWN);

char message[16];
bool buttonPressed = 0;
byte timerHasStarted = 0;
char seconds, minutes, hours;
char updateDigit = 0;
bool bBlink = HIGH;
volatile unsigned char level = 0;

/***** DEBOUNCE STUFF HERE ******/
int lastButtonState = HIGH;   // the previous reading from the input pin
int buttonState;             // the current reading from the input pin
// the following variables are long's because the time, measured in miliseconds,
// will quickly become a bigger number than can be stored in an int.
unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled
unsigned long debounceDelay = 50;    // the debounce time; increase if the output flickers
int digitSelect, buttonRead;
/*******************************/

/***** ENCODER STUFF HERE ******/
//static int pinA = 2; // Our first hardware interrupt pin is digital pin 2
//static int pinB = 3; // Our second hardware interrupt pin is digital pin 3
volatile byte aFlag = 0; // let's us know when we're expecting a rising edge on pinA to signal that the encoder has arrived at a detent
volatile byte bFlag = 0; // let's us know when we're expecting a rising edge on pinB to signal that the encoder has arrived at a detent (opposite direction to when aFlag is set)
volatile char encoderPos; //this variable stores our current value of encoder position. Change to int or uin16_t instead of byte if you want to record a larger range than 0-255
volatile char oldEncPos; //stores the last encoder position value so we can compare to the current reading and see if it has changed (so we know when to print to the serial monitor)
volatile byte reading = 0; //somewhere to store the direct values we read from our interrupt pins before checking to see if we have moved a whole detent
volatile char modifyDigit = 0;
/*******************************/

void setup() {
  pinMode(2, INPUT_PULLUP);
  pinMode(3, INPUT_PULLUP);
  pinMode(4, INPUT_PULLUP);
  attachInterrupt(0, PinA, RISING); // set an interrupt on PinA, looking for a rising edge signal and executing the "PinA" Interrupt Service Routine (below)
  attachInterrupt(1, PinB, RISING); // set an interrupt on PinB, looking for a rising edge signal and executing the "PinB" Interrupt Service Routine (below)
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
  lcd.setCursor(0, 1);
  //  sprintf(message,"%02d:%02d:%02d", 0, encoderPos/60, encoderPos%60);
}
void clearLcd()
{
  lcd.setCursor(0, 0);
  lcd.print("            ");
  lcd.setCursor(0, 1);
  lcd.print("            ");
}
void PinA() {
  cli(); //stop interrupts happening before we read pin values
  reading = PIND & 0xC; // read all eight pin values then strip away all but pinA and pinB's values
  if (reading == B00001100 && aFlag) { //check that we have both pins at detent (HIGH) and that we are expecting detent on this pin's rising edge
    if (!updateDigit)
      level = !level;
    else
      modifyDigit ++;
    encoderPos ++; //increment the encoder's position count
    bFlag = 0; //reset flags for the next turn
    aFlag = 0; //reset flags for the next turn
  }
  else if (reading == B00000100) bFlag = 1; //signal that we're expecting pinB to signal the transition to detent from free rotation
  sei(); //restart interrupts
}

void PinB() {
  cli(); //stop interrupts happening before we read pin values
  reading = PIND & 0xC; //read all eight pin values then strip away all but pinA and pinB's values
  if (reading == B00001100 && bFlag) { //check that we have both pins at detent (HIGH) and that we are expecting detent on this pin's rising edge
    if (!updateDigit)
      level = !level;
    else
      modifyDigit --;
    encoderPos --; //decrement the encoder's position count
    bFlag = 0; //reset flags for the next turn
    aFlag = 0; //reset flags for the next turn
  }
  else if (reading == B00001000) aFlag = 1; //signal that we're expecting pinA to signal the transition to detent from free rotation
  sei(); //restart interrupts
}

void loop() {
  MyTimer.Timer(); // run the timer

  if (oldEncPos != encoderPos)
  {
    clearLcd();
    oldEncPos = encoderPos;
  }

  KeyManager();
  BlinkMng();

  if (level)
  {
    WriteString((char*)"Start", 0, 1, 1);
    WriteString((char*)">Set", 8, 1, 1);
  }
  else
  {
    WriteString((char*)"Start<", 0, 1, 1);
    WriteString((char*)"Set", 9, 1, 1);
  }

  if (level && updateDigit)
  {
    switch (digitSelect)  // set timer
    {
      case 0:
        seconds = modifyDigit;
        if (seconds > 59)
        {
          seconds = 0;
          encoderPos = 0;
          modifyDigit = 0;
        }
        else if (seconds < 0)
        {
          seconds = 59;
          encoderPos = 59;
          modifyDigit = 59;
        }
        break;
      case 1:
        minutes = modifyDigit;
        if (minutes > 59)
        {
          minutes = 0;
          encoderPos = 0;
          modifyDigit = 0;
        }
        else if (minutes < 0)
        {
          minutes = 59;
          encoderPos = 59;
          modifyDigit = 59;
        }
        break;
      case 2:
        hours = modifyDigit;
        if (hours > 23)
        {
          hours = 0;
          encoderPos = 0;
          modifyDigit = 0;
        }
        else if (hours < 0)
        {
          hours = 23;
          encoderPos = 23;
          modifyDigit = 23;
        }
        break;
      default:
        break;
    }
  }

  if (level)  // set timer is selected
  {
    if (buttonPressed)  // update timer
    {
      updateDigit = 1;
      WriteString(message, 0, 0, (0x0030 << (digitSelect * 3)) | 2);
    }
    else  // display only
    {
      updateDigit = 0;
      WriteString(message, 0, 0, 1);
    }
  }
  else  // start timer is selected
  {
    if (buttonPressed && (hours || minutes || seconds)) // check to see if the timer is not zero
    {
      // set and start timer
      MyTimer.SetTimer(hours, minutes, seconds);
      MyTimer.StartTimer();
      timerHasStarted = 1;
    }
    else
      WriteString(message, 0, 0, 1);
  }

  if (MyTimer.TimeHasChanged())
  {
    sprintf(message, "  %02d:%02d:%02d  ", 0, (unsigned char)MyTimer.ShowMinutes(), (unsigned char)MyTimer.ShowSeconds());
    clearLcd();
    lcd.setCursor(0, 0);
    lcd.print(message);
  }
  else
    sprintf(message, "  %02d:%02d:%02d  ", hours, minutes, seconds);

  if (MyTimer.TimeCheck())
    timerHasStarted = 0;
}

void KeyManager(void)
{
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
        if (level && updateDigit)  // set timer mode active
        {
          if (digitSelect >= 2)
            digitSelect = 0;
          else
            digitSelect++;
          switch (digitSelect)  // get the last value of the encoder
          {
            case 0:
              encoderPos = seconds;
              modifyDigit = seconds;
              break;
            case 1:
              encoderPos = minutes;
              modifyDigit = minutes;
              break;
            case 2:
              encoderPos = hours;
              modifyDigit = hours;
              break;
            default:
              break;
          }
        }
        buttonPressed = !buttonPressed;
        clearLcd();
      }
    }
  }

  // save the reading.  Next time through the loop,
  // it'll be the lastButtonState:
  lastButtonState = buttonRead;
}

/*
   Function Name:  WriteString
   Description: Write a message at a specified possition on the LCD
   Paramters:
        msg (in) - message to be displayed in the LCD
        x_pos (in) - x posistion on the LCD
        y_pos (in) - y posistion on the LCD
        option (in) - sets the mode in whitch the message is shown on the LCD
                    - starting from the LSB: b0,b1 represent the status bits (ERASE, NORMAL, BLINK),
                    the next 12 bits are for blinking the characters individualy
   Return: void
*/
void WriteString(char * msg, char x_pos, char y_pos, int option)
{
  char blank[] = "            "; // string used to clear the line
  int positions;  // variable to store the blinking bits from the 'option' varible
  int mask; // masking the individiual bits of the variable 'positions' which represents the position that is blinking of the 12 positions on the LCD
  unsigned char blink_position = 0;  // current blink position

  // check if there is a message to be displayed and
  // that the positions aren't over the limits
  if ((msg == NULL) || (x_pos > 11) || (y_pos > 1))
    return;

  positions = ((option & 0x3FFC) >> 2);

  switch (option & 0x0003)
  {
    case 0: // erase LCD line
      lcd.setCursor(x_pos, y_pos);
      lcd.print(blank);
      break;
    case 1: // write the message on the LCD line
      lcd.setCursor(x_pos, y_pos);
      lcd.print(msg);
      break;
    case 2: // blink the message on the LCD line
      if (bBlink)
      {
        lcd.setCursor(x_pos, y_pos);
        lcd.print(msg);
        //        delay(250);
      }
      else
      {
        if (option != 2)
        {
          mask = 0x0800;  // start from the first position
          for (unsigned char i = 0; i < 12; i++)
          {
            if (mask & positions) // if the bit is set then that position will blink
            {
              // blink the selected position on the LCD
              lcd.setCursor(x_pos + blink_position, y_pos);
              lcd.print(blank[blink_position]);
            }
            mask = mask >> 1; // mask the next bit
            blink_position++; // increment the current blink position
          }
        }
        else
        {
          for (unsigned char i = x_pos; i < (x_pos + strlen(msg)); i++)
          {
            // blink the selected position on the LCD
            lcd.setCursor(i, y_pos);
            lcd.print(blank[i]);
          }
        }
        //      delay(250);
      }
      break;
    default:
      break;
  }
}

void BlinkMng()
{
  static unsigned long timer;
  static char state = 0;

  switch (state)
  {
    case 0: // start
      timer = millis();
      state = 1;
      break;
    case 1: // wait
      if ((millis() - timer) >= 250UL)
      {
        state = 0;
        bBlink = !bBlink;
      }
      break;
    default:
      break;
  }
}

