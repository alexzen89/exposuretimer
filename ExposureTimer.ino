// include the library code:
#include <LiquidCrystal.h>
#include <CountUpDownTimer.h>

#define ENC_LIMIT_MAX 3599
#define DEBOUNCE 50  // button debouncer, how many ms to debounce, 5+ ms is usually plenty

enum
{
  BTN_UP,
  BTN_DOWN,
  BTN_ENTER,
  BTN_EXIT,
  BUTTONS_NO,
  BTN_NONE = 0xFF
};

typedef struct _stMenu
{
  char level;
  unsigned char * variable;
  char * msg_line;
}stMenu;

const stMenu ExposureMenu[2] = 
{
  {
    0x00,
    NULL,
    (char*)"Start    Set"    
  },
  {
    0x01,
    NULL,
    (char*)"Exit    Next"
  }
};

// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(A5, A4, A3, A2, A1, A0);
CountUpDownTimer MyTimer(DOWN);

unsigned char currentPressedButton = BTN_NONE;
char message[16];
bool buttonPressed = 0;
byte timerHasStarted = 0;
char seconds, minutes, hours;
char updateDigit = 0;
bool bBlink = HIGH;
volatile unsigned char level = 0;
int digitSelect = 0;
int modifyDigit = 0;

byte buttonArray[BUTTONS_NO] = {3, 4, 5, 6};
byte ledArray[BUTTONS_NO] = {7, 11, 12, 13};

// Variables will change:
byte ledState[4] = {HIGH, HIGH, HIGH, HIGH};           // the current state of the output pin
byte buttonState[4];                               // the current reading from the input pin
byte lastButtonState[4] = {LOW, LOW, LOW, LOW};    // the previous reading from the input pin

// the following variables are long's because the time, measured in miliseconds,
// will quickly become a bigger number than can be stored in an int.
unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled

void setup() {
  // Make input & enable pull-up resistors on switch pins
  //  for (byte i = 0; i < NUMBUTTONS; i++)
  //  {
  //    pinMode(buttons[i], INPUT_PULLUP);
  //    //    digitalWrite(buttons[i], HIGH);
  //  }
  for (byte i = 0; i < BUTTONS_NO; i++)
  {
    pinMode(buttonArray[i], INPUT_PULLUP);
    pinMode(ledArray[i], OUTPUT);
    // set initial LED state
    digitalWrite(ledArray[i], ledState[i]);
  }

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

void loop() {
  //  MyTimer.Timer(); // run the timer

  KeyManager();
  //  BlinkMng();

  if (currentPressedButton == BTN_ENTER)
  {
    level = !level;
    currentPressedButton = BTN_NONE;
  }
  WriteString(ExposureMenu[level].msg_line, 0, 1, 1);
//  WriteString(line2_msg, 0, 1, 1);


//  if (level)
//  {
//    WriteString((char*)"Start", 0, 1, 1);
//    WriteString((char*)">Set", 8, 1, 1);
//  }
//  else
//  {
//    WriteString((char*)"Start<", 0, 1, 1);
//    WriteString((char*)"Set", 9, 1, 1);
//  }
//
//  if (level && updateDigit)
//  {
//    switch (digitSelect)  // set timer
//    {
//      case 0:
//        seconds = modifyDigit;
//        if (seconds > 59)
//        {
//          seconds = 0;
//          modifyDigit = 0;
//        }
//        else if (seconds < 0)
//        {
//          seconds = 59;
//          modifyDigit = 59;
//        }
//        break;
//      case 1:
//        minutes = modifyDigit;
//        if (minutes > 59)
//        {
//          minutes = 0;
//          modifyDigit = 0;
//        }
//        else if (minutes < 0)
//        {
//          minutes = 59;
//          modifyDigit = 59;
//        }
//        break;
//      case 2:
//        hours = modifyDigit;
//        if (hours > 23)
//        {
//          hours = 0;
//          modifyDigit = 0;
//        }
//        else if (hours < 0)
//        {
//          hours = 23;
//          modifyDigit = 23;
//        }
//        break;
//      default:
//        break;
//    }
//  }
//
//  if (level)  // set timer is selected
//  {
//    if (buttonPressed)  // update timer
//    {
//      updateDigit = 1;
//      WriteString(message, 0, 0, (0x0030 << (digitSelect * 3)) | 2);
//    }
//    else  // display only
//    {
//      updateDigit = 0;
//      WriteString(message, 0, 0, 1);
//    }
//  }
//  else  // start timer is selected
//  {
//    if (buttonPressed && (hours || minutes || seconds)) // check to see if the timer is not zero
//    {
//      // set and start timer
//      MyTimer.SetTimer(hours, minutes, seconds);
//      MyTimer.StartTimer();
//      timerHasStarted = 1;
//    }
//    else
//      WriteString(message, 0, 0, 1);
//  }
//
//  if (MyTimer.TimeHasChanged())
//  {
//    sprintf(message, "  %02d:%02d:%02d  ", 0, (unsigned char)MyTimer.ShowMinutes(), (unsigned char)MyTimer.ShowSeconds());
//    clearLcd();
//    lcd.setCursor(0, 0);
//    lcd.print(message);
//  }
//  else
//    sprintf(message, "  %02d:%02d:%02d  ", hours, minutes, seconds);
//
//  if (MyTimer.TimeCheck())
//    timerHasStarted = 0;
}

void KeyManager(void)
{
byte key;

  for (key = 0; key < BUTTONS_NO; key++)
  {
    if(CheckPressed(buttonArray[key], ledArray[key], key))
      currentPressedButton = key;
  }
}

byte CheckPressed(byte key, byte led, byte index)
{
byte reading;
byte isButtonPressed = LOW;

  // read the state of the switch into a local variable:
  reading = digitalRead(key);

  // check to see if you just pressed the button
  // (i.e. the input went from LOW to HIGH),  and you've waited
  // long enough since the last press to ignore any noise:

  // If the switch changed, due to noise or pressing:
  if (reading != lastButtonState[index]) {
    // reset the debouncing timer
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > DEBOUNCE) {
    // whatever the reading is at, it's been there for longer
    // than the debounce delay, so take it as the actual current state:

    // if the button state has changed:
    if (reading != buttonState[index]) {
      buttonState[index] = reading;

      // only toggle the LED if the new button state is HIGH
      if (buttonState[index] == HIGH) {
        ledState[index] = !ledState[index];
        isButtonPressed = HIGH;
      }
    }
  }

  // set the LED:
  digitalWrite(led, ledState[index]);

  // save the reading.  Next time through the loop,
  // it'll be the lastButtonState:
  lastButtonState[index] = reading;

  return isButtonPressed;
}

/*
   Function Name:   WriteString
   Description:     Write a message at a specified possition on the LCD
   Paramters:
                    msg (in) - message to be displayed in the LCD
                    x_pos (in) - x posistion on the LCD
                    y_pos (in) - y posistion on the LCD
                    option (in) - sets the mode in whitch the message is shown on the LCD
                                - starting from the LSB: b0,b1 represent the status bits (ERASE, NORMAL, BLINK),
                                the next 12 bits are for blinking the characters individualy
   Return:          void
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

