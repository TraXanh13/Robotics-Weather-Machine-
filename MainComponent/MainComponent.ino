/**
 * Components for this part:
 *    x1 Arduino Uno
 *    x1 Breadboard
 *    x1 RF24 (Receiver)
 *    x1 DS3231 (Real-Time-Clock module)
 *    x1 16x2 LCD screen
 *    x3 Button 
 *    x3 Resistor 
 * 
 * This part is for the indoor portion. 
 * This half of the project receives a package
 * with the RF24 sent by the transmitter (other RF24) 
 * and uses that data to display the temperature and 
 * humidity to the LCD screen. It also keeps track of 
 * the time and date with the DS3231 that is then 
 * displayed on the LCD screen.
 * 
 * It contains four different states taht is changed 
 * with the use of the buttons.
 * 
 *    1 (TiDa):    
 *        Displays the time on the top line and 
 *        the date on the second line.
 *    
 *    2 (TeHu):
 *        Displays the temperature on the top line 
 *        and the humidity on the second line.
 *    
 *    3 (TiTe):
 *        Displays the time on the top line and
 *        the temperature on the second line.
 *        
 *    4 (ChDaTi):
 *        Displays whats being changes (Time/Date) 
 *        on the top line and what is being 
 *        changed (Hour/Minute/Month/Day/Year) 
 *        on the second line.
 *        
 * For this specific project the buttons are:  
 *    button 1: Blue
 *    button 2: Green
 *    button 3: Red
 *    
 * State Conditions:
 *    1 (TiDa):
 *        1 -> 2: Press button 1 (blue)  
 *        1 -> 3: Press button 2 (green)
 *        1 -> 4: Press button 3 (red)
 *    2 (TeHu):  
 *        2 -> 3: Press button 1 (blue)
 *        2 -> 1: Press button 2 (green)
 *    3 (TiTe):
 *        3 -> 1: Press button 1 (blue)
 *        3 -> 2: Press button 2 (green)
 *        3 -> 4: Press button 3 (red)
 *    4 (ChDaTi):
 *        4 -> 1: Finish changing the time and date.
 *                Press button 3 (red) 5 times. 
 */

//------------------------------------------------------------------

// Libraries for transmitter
#include <SPI.h>
#include "RF24.h"

// Library for real time clock
#include <DS3231.h>

// Libraries for LCD screen
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

//------------------------------------------------------------------

// Pin for the first button (blue)
const int BUTT1_PIN = 2;

// Pin for the second button (green)
const int BUTT2_PIN = 3;

// Pin for the third button (red)
const int BUTT3_PIN = 4;

//------------------------------------------------------------------

/* 
 *  States for the weather machine
 *  
 *  Ti: Time
 *  Da: Date
 *  Te: Temperature
 *  Hu: Humidity
 *  Ch: Change
 */
enum State{
  TiDa,
  TeHu,
  TiTe,
  ChDaTi
};

// Initialize the current state
State currState = TiDa;

//------------------------------------------------------------------

// Initializing clock stuff
DS3231 clock;
RTCDateTime dt;

// Initializing LCD screen and size
// location, columns, rows
LiquidCrystal_I2C lcd(0x27,16,2);

//------------------------------------------------------------------

// Receiver pins
RF24 receiver (7, 8);
// Address at 0
byte addresses [][6] = {"0"};

// The information received
struct package
{
  float temp = 0.0;
  float humid = 0;
};

typedef struct package Package;
Package data;

//------------------------------------------------------------------

void setup() {
  // Setting up the receiver
  receiver.begin(); 
  receiver.setChannel(113); 
  receiver.setPALevel(RF24_PA_MAX);
  receiver.setDataRate(RF24_250KBPS) ; 
  receiver.openReadingPipe(1, addresses[0]);
  receiver.startListening();

  // Setting up the clock
  clock.begin();
  // Setting the date and time to match the computer
  clock.setDateTime(__DATE__, __TIME__);

  // Setting up the lcd screen
  Wire.begin();
  // Clearing everything on the LCD
  lcd.clear();
  lcd.init();
  // Turning the backlight on
  lcd.backlight();

  pinMode(BUTT1_PIN, INPUT);
  pinMode(BUTT2_PIN, INPUT);
  pinMode(BUTT3_PIN, INPUT);
}

//------------------------------------------------------------------

void loop() {
  // Retrieves transmission from the temperature and humidity sensor
  updatePackage();
  
  // Updates the date and time
  dt = clock.getDateTime();
  runStateMachine();
}

//------------------------------------------------------------------

// Moving through the different states
void runStateMachine(){
  switch(currState){
    case TiDa:
      runTiDa();
      break;
    case TeHu:
      runTeHu();
      break;
    case TiTe:
      runTiTe();
      break;
    case ChDaTi:
      runChDaTi();
      break;      
  }
}

//------------------------------------------------------------------

// Checks to see if new information is received and updates the data package
void updatePackage(){
  if ( receiver.available()) 
  {
    while (receiver.available())
    {
      receiver.read( &data, sizeof(data) );
    }
  }
}

//------------------------------------------------------------------

// Displays the time on top and the date on the bottom of the lcd
void runTiDa(){ 
  // Set the position of first char to col 0 and row 0 
  // setCursor([col], [row])
  lcd.setCursor(0, 0);
  printTime();

  // Set the position of first char to col 0 and row 1
  // setCursor([col], [row])
  lcd.setCursor(0, 1);
  printDate();

  // Check for a button press to change state
  if(digitalRead(BUTT1_PIN) == HIGH){
    currState = TeHu;
    delay(500);
    lcd.clear();
  } else if(digitalRead(BUTT2_PIN) == HIGH){
    currState = TiTe;
    delay(500);
    lcd.clear();
  } else if(digitalRead(BUTT3_PIN) == HIGH){
    currState = ChDaTi;
    delay(500);
    lcd.clear();
  }
}

//------------------------------------------------------------------

// Displays the temperature on the top and the humidity on the bottom of the lcd
void runTeHu(){
  // Set the position of first char to col 0 and row 0 
  // setCursor([col], [row])
  lcd.setCursor(0, 0);
  printTemp();

  // Set the position of first char to col 0 and row 1
  // setCursor([col], [row])
  lcd.setCursor(0, 1);
  printHumid();

  // Check for a button press to change state
  if(digitalRead(BUTT1_PIN) == HIGH){
    currState = TiTe;
    delay(500);
    lcd.clear();
  } else if(digitalRead(BUTT2_PIN) == HIGH){
    currState = TiDa;
    delay(500);
    lcd.clear();
  }
}

//------------------------------------------------------------------

// Displays the time on top and the temperature on the bottom of the lcd
void runTiTe(){
  // Set the position of first char to col 0 and row 0 
  // setCursor([col], [row])
  lcd.setCursor(0, 0);
  printTime();

  // Set the position of first char to col 0 and row 1
  // setCursor([col], [row])
  lcd.setCursor(0, 1);
  printTemp();

  // Check for a button press to change state
  if(digitalRead(BUTT1_PIN) == HIGH){
    currState = TiDa;
    delay(500);
    lcd.clear();
  } else if(digitalRead(BUTT2_PIN) == HIGH){
    currState = TeHu;
    delay(500);
    lcd.clear();
  } else if(digitalRead(BUTT3_PIN) == HIGH){
    currState = ChDaTi;
    delay(500);
    lcd.clear();
  }
}

//------------------------------------------------------------------

// Changes the date and time
void runChDaTi(){  
  // Date
  int day = dt.day;
  int month = dt.month;
  int year = dt.year;
  int maxDay = 1;
  
  // Time
  int hour = dt.hour;
  int mins = dt.minute;

  // Sets the cursor to column 0 and row 0
  lcd.setCursor(0, 0);
  lcd.print("Change time");

  //Setting the hour
  // Sets the lcd cursor to column 0 row 1
  lcd.setCursor(0, 1);
  lcd.print("Hour: ");
  // Keep allowing editing until button 3 is pressed
  while(digitalRead(BUTT3_PIN) == LOW){
    lcd.setCursor(7, 1);
    if(hour < 10){
      lcd.print("0");
      lcd.print(hour);
    }else{
      lcd.print(hour);
    }
    // If button 1 is pressed, increase the hour by 1
    if(digitalRead(BUTT1_PIN) == HIGH){
      hour++;
      if(hour > 23){
        hour=0;
      }
    // If button 2 is pressed, decrease the hour by 1
    } else if(digitalRead(BUTT2_PIN) == HIGH){
      hour--;
      if(hour < 0){
        hour=23;
      }
    }
    delay(250);
  }

  // Setting the minute
  lcd.setCursor(0, 1);
  lcd.print("Mins: ");
  delay(250);
  // Continue editing the minute until button 3 is pressed
  while(digitalRead(BUTT3_PIN) == LOW){
    lcd.setCursor(7, 1);
    if(mins < 10){
      lcd.print("0");
      lcd.print(mins);
    }else{
      lcd.print(mins);
    }
    // If button 1 is pressed, increase the minute by 1
    if(digitalRead(BUTT1_PIN) == HIGH){
      mins++;
      if(mins > 59){
        mins=0;
      }
    // If button 2 is pressed, decrease the minute by 1
    } else if(digitalRead(BUTT2_PIN) == HIGH){
      mins--;
      if(mins < 0){
        mins=59;
      }
    }
    delay(250);
  }

  // Changing the date
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Change date");
  delay(250);
  
  //Setting the month
  lcd.setCursor(0, 1);
  lcd.print("MM: ");
  // Continue editing the month until button 3 is pressed
  while(digitalRead(BUTT3_PIN) == LOW){
    lcd.setCursor(7, 1);
    if(month < 10){
      lcd.print("0");
      lcd.print(month);
    }else{
      lcd.print(month);
    }
    // If button 1 is pressed, increase the month by 1
    if(digitalRead(BUTT1_PIN) == HIGH){
      month++;
      if(month > 12){
        month=1;
      }
    // If button 2 is pressed, decrease the month by 1
    } else if(digitalRead(BUTT2_PIN) == HIGH){
      month--;
      if(month < 0){
        month=12;
      }
    }
    delay(250);
  }

  //Setting the day
  lcd.setCursor(0, 1);
  lcd.print("DD: ");
  delay(250);
  maxDay = getMaxDay(month);
  // Continue editing the day until button 3 is pressed
  while(digitalRead(BUTT3_PIN) == LOW){
    lcd.setCursor(7, 1);
    if(day < 10){
      lcd.print("0");
      lcd.print(day);
    }else{
      lcd.print(day);
    }
    // If button 1 is pressed, increase the day by 1
    if(digitalRead(BUTT1_PIN) == HIGH){
      day++;
      if(day > maxDay){
        day=1;
      }
    // If button 2 is pressed, decrease the day by 1
    } else if(digitalRead(BUTT2_PIN) == HIGH){
      day--;
      if(day < 1){
        day = maxDay;
      }
    }
    delay(250);
  }

  //Setting the year
  lcd.setCursor(0, 1);
  lcd.print("YYYY: ");
  delay(250);
  // Continue editing the year until button 3 is pressed
  while(digitalRead(BUTT3_PIN) == LOW){
    lcd.setCursor(7, 1);
    lcd.print(year);
    // If button 1 is pressed, increase the year by 1
    if(digitalRead(BUTT1_PIN) == HIGH){
      year++;
    // If button 2 is pressed, decrease the year by 1
    } else if(digitalRead(BUTT2_PIN) == HIGH){
      year--;
    }
    delay(250);
  }

  // Sets the date and time to the users change
  clock.setDateTime(year, month, day, hour, mins, dt.second);
  currState = TiDa;
  delay(500);
  lcd.clear();
}

//------------------------------------------------------------------

/**
 * Checks to see if the month is in an array thus determining if the
 * month has 30 or 31 days. Otherwise is it February with a max of
 * 29 days. Then it returns to max number of days for that month.
 * 
 * month: the month selected by the user.
 * returns: max days in the selected month
 */
int getMaxDay(int month){
  // Array of days with 30 or 31 days
  int months30[] = {4, 6, 9, 11};
  int months31[] = {1, 3, 5, 7, 8, 10, 12};

  // The maximum number of days in the month
  int maxDay = 29;
  
  // Checks to see if the month is in the list of months with 30
  for(int i = 0; i < sizeof(months30)/sizeof(months30[0]); i++){
    if(months30[i] == month){
      maxDay = 30;
      return maxDay;
    }
  }
  
  // Checks to see if the month is in the list of months with 31 days
  for(int i = 0; i < sizeof(months31)/sizeof(months31[0]); i++){
    if(months31[i] == month){
      maxDay = 31;
      return maxDay;
    }
  }

  // If the month is February
  return maxDay;
}

//------------------------------------------------------------------

// Prints the hour, minute, and seconds to the LCD
void printTime(){  
  lcd.print("Time: ");
  // Checks if the hour is less than 10, it will then print 0[hour]
  if(dt.hour < 10){
    lcd.print("0");
    lcd.print(dt.hour);  
  }else{
    lcd.print(dt.hour); 
  }  
  
  lcd.print(":");

  // Checks if the minute is less than 10, it will then print 0[minute]
  if(dt.minute < 10){
    lcd.print("0");
    lcd.print(dt.minute);  
  }else{
    lcd.print(dt.minute); 
  }

  lcd.print(":");

  // Checks if the seconds is less than 10, it will then print 0[seconds]
  if(dt.second < 10){
    lcd.print("0");
    lcd.print(dt.second);  
  }else{
    lcd.print(dt.second); 
  }
}

//------------------------------------------------------------------

// Prints the date in numeric value (mm/dd/yyyy) on the lcd
void printDate(){
  lcd.print("Date: ");

  // Checks if the month is less than 10, it will then print 0[month]
  if(dt.month < 10){
    lcd.print("0");
    lcd.print(dt.month);
  }else{
    lcd.print(dt.month);
  }
  lcd.print("/");

  // Checks if the day is less than 10, it will then print 0[day]
  if(dt.day < 10){
    lcd.print("0");
    lcd.print(dt.day);
  }else{
    lcd.print(dt.day);
  }
  lcd.print("/");
  lcd.print(dt.year);
}

//------------------------------------------------------------------

// Prints the temperature contents of the package to the lcd
void printTemp(){
  lcd.print("Temp: ");
  lcd.print(data.temp);
  lcd.print((char)223);
  lcd.print("C");
}

//------------------------------------------------------------------

// Prints the humidity contents of the package to the lcd
void printHumid(){
  lcd.print("Humid:");
  lcd.print(data.humid);
  lcd.print("%");
}
