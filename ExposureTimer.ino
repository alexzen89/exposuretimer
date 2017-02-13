// include the library code:
#include <LiquidCrystal.h>
#include <CountUpDownTimer.h>

#define ENC_LIMIT_MAX 3599
#define DEBOUNCE 50  // button debouncer, how many ms to debounce, 5+ ms is usually plenty

enum
{
  BTN_CUSTOM_1,
  BTN_CUSTOM_2,
  BTN_UP,
  BTN_DOWN,
  BUTTONS_NO,
  BTN_NONE = 0xFF
};

// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(A5, A4, A3, A2, A1, A0);
CountUpDownTimer MyTimer(DOWN);

char message[16];
byte timerHasStarted = 0;
byte timerHasStoped = 0;
char seconds, minutes, hours;
bool bBlink = HIGH;
volatile unsigned char level = 0;
int nextDigit = 0;
int modifyDigit = 0;
char processDone = 0;
byte ctrlPin = 4; // light pin

byte buttonArray[BUTTONS_NO] = {0, 1, 2, 3};
byte ledArray[BUTTONS_NO] = {7, 11, 12, 13};

// Variables will change:
byte ledState[4] = {HIGH, HIGH, HIGH, HIGH};           // the current state of the output pin
volatile byte buttonState[4] = {HIGH, HIGH, HIGH, HIGH};                               // the current reading from the input pin
volatile byte lastButtonState[4] = {LOW, LOW, LOW, LOW};    // the previous reading from the input pin

// the following variables are long's because the time, measured in miliseconds,
// will quickly become a bigger number than can be stored in an int.
unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled

void setup() {
  // Make input & enable pull-up resistors on switch pins
  for (byte i = 0; i < BUTTONS_NO; i++)
  {
    pinMode(buttonArray[i], INPUT_PULLUP);
    pinMode(ledArray[i], OUTPUT);
    // set initial LED state
    digitalWrite(ledArray[i], ledState[i]);
  }

  // init the controll pin, which controls the neon lamps
  pinMode(ctrlPin, OUTPUT);
  digitalWrite(ctrlPin, LOW);
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

void loop()
{
  MyTimer.Timer(); // run the timer

  KeyManager();
  BlinkMng();

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
  if (level)
  {
    WriteString(message, 0, 0, (0x0030 << (nextDigit * 3)) | 2);

    switch (nextDigit)  // set timer
    {
      case 0:
        //        seconds = modifyDigit;
        if (seconds > 59)
        {
          seconds = 0;
          //          modifyDigit = 0;
        }
        else if (seconds < 0)
        {
          seconds = 59;
          //          modifyDigit = 59;
        }
        break;
      case 1:
        //        minutes = modifyDigit;
        if (minutes > 59)
        {
          minutes = 0;
          //          modifyDigit = 0;
        }
        else if (minutes < 0)
        {
          minutes = 59;
          //          modifyDigit = 59;
        }
        break;
      case 2:
        //        hours = modifyDigit;
        if (hours > 23)
        {
          hours = 0;
          //          modifyDigit = 0;
        }
        else if (hours < 0)
        {
          hours = 23;
          //          modifyDigit = 23;
        }
        break;
      default:
        break;
    }

    sprintf(message, "  %02d:%02d:%02d  ", hours, minutes, seconds);
    WriteString((char*)"Exit    Next", 0, 1, 1);
  }
  else
  {
    if (timerHasStarted)
    {
      if (MyTimer.TimeHasChanged())
      {
        sprintf(message, "  %02d:%02d:%02d  ", 0, (unsigned char)MyTimer.ShowMinutes(), (unsigned char)MyTimer.ShowSeconds());
        clearLcd();
      }
      if (timerHasStoped)
        WriteString((char*)"Resume   Set", 0, 1, 1);
      else
        WriteString((char*)"Stop     Set", 0, 1, 1);
    }
    else
    {
      sprintf(message, "  %02d:%02d:%02d  ", hours, minutes, seconds);
      if (processDone)
      {
        processDone = 0;
        clearLcd();
        WriteString((char*)"Process Done", 0, 0, 1);
        WriteString((char*)" Remove PCB ", 0, 1, 1);
        delay(3000);
      }
      WriteString((char*)"Start    Set", 0, 1, 1);
    }
    WriteString(message, 0, 0, 1);
  }

  if (MyTimer.TimeCheck() && timerHasStarted)  // timer has expired, do something usefull
  {
    // turn lights off
    digitalWrite(ctrlPin, LOW);
    // reset start
    processDone = 1;
    timerHasStarted = 0;
  }
}

/*
   Function Name:   KeyManager
   Description:     State machine which controlls what the buttons do after they are pressed
   Paramters:       none
   Return:          void
*/
void KeyManager(void)
{
  byte key;

  for (key = 0; key < BUTTONS_NO; key++)
  {
    if (ButtonPressed(buttonArray[key], ledArray[key], key))
    {
      switch (key)
      {
        case BTN_CUSTOM_1:
          if (level)
            level = 0;
          else
          {
            if (!timerHasStarted) // the timer was not running
            {
              if (hours || minutes || seconds) // check to see if the timer is not zero)
              {
                // set and start timer
                MyTimer.SetTimer(hours, minutes, seconds);
                MyTimer.StartTimer();
                timerHasStarted = 1;

                // turn on the lamps
                digitalWrite(ctrlPin, HIGH);
              }
            }
            else  // the timer is running, you can stop it
            {
              if (timerHasStoped)
              {
                MyTimer.ResumeTimer();
                timerHasStoped = 0;
              }
              else
              {
                MyTimer.PauseTimer();
                timerHasStoped = 1;
              }  
            }
          }
          break;
        case BTN_CUSTOM_2:
          if (level)
          {
            if (nextDigit < 2)
              nextDigit++;
            else
              nextDigit = 0;
          }
          else
            level = 1;
          break;
        case BTN_UP:
          // update timer value
          if (level)
          {
            switch (nextDigit)
            {
              case 0:
                seconds++;
                break;
              case 1:
                minutes++;
                break;
              case 2:
                hours++;
                break;
            }
          }
          break;
        case BTN_DOWN:
          // update timer value
          if (level)
          {
            switch (nextDigit)
            {
              case 0:
                seconds--;
                break;
              case 1:
                minutes--;
                break;
              case 2:
                hours--;
                break;
            }
          }
          break;
        default:
          break;
      }
    }
  }
}

/*
   Function Name:   ButtonPressed
   Description:     Check if the buttons are pressed and what buttons are pressed
   Paramters:
                    key (in) - hardware pin on the uC for the buttons
                    byte (in) - hardware pin on the uC for the LEDs, for debuging only
                    index (in) - index of the button being pressed
   Return:          state of the buttons, pressed or not
*/
byte ButtonPressed(byte key, byte led, byte index)
{
  byte reading;
  byte isButtonPressed = LOW;

  // TO DO: add multimple press
  
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

/*
   Function Name:   BlinkMng
   Description:     A little state machine used for the blinking of the LCD messages
                    which replaces the use of the "delay" function
   Paramters:       none
   Return:          void
*/
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

