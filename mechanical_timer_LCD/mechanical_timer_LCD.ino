#include <LiquidCrystal.h>

// http://www.pjrc.com/teensy/td_libs_Time.html
// http://playground.arduino.cc/Code/Time
#include <Time.h> 
#include <Wire.h>

// http://www.pjrc.com/teensy/td_libs_TimeAlarms.html
#include <TimeAlarms.h>

// for RTC see http://www.hobbyist.co.nz/?q=real_time_clock
#include <DS1307RTC.h> 

/*******************************************************

Giorgio Gilestro, 2015

********************************************************/



// pin for mosfet
const int mosfetA = 16; //A1
const int mosfetB = 17; //A2

//SD_PULSE regulates the duration of the shaking pulse 
const int MIN_SD_PULSE = 5; //in seconds
const int MAX_SD_PULSE = 20; //in seconds

//SD_PAUSE regulates the pause between pulses
const int MIN_SD_PAUSE = 1; //in minutes
const int MAX_SD_PAUSE = 7; //in minutes


// select the pins used on the LCD panel
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);

// define some values used by the panel and buttons
int lcd_key     = 0;
int adc_key_in  = 0;

int lastDay = 0;
int lastMonth = 0;
int lastYear = 0;
int lastHour = 0;
int lastMinute = 0;
int menuOptions = 3;
int menuOption = 0;
bool resetClock = false;

int SD_start_hours = 10;
int SD_start_minutes = 0;
long SD_length_hours = 12;
bool SDSet = 0;
bool SD_running = false;
bool SD_vibrate = false;
long SD_started = 0;
long SD_A = 0;
long SD_B = 0;
long next_action = 0;
int SD_AB = 0;
int SD_BC = 0;

#define beeper A1
#define shortBeep 100
#define longBeep  500

// define constants
const int backLight = 10; 
//const int pirPin = 16;
#define btnRIGHT  0
#define btnUP     1
#define btnDOWN   2
#define btnLEFT   3
#define btnSELECT 4
#define btnNONE   5

// read the buttons
int read_LCD_buttons()
{
 adc_key_in = analogRead(0);      // read the value from the sensor 
 // my buttons when read are centered at these valies: 0, 144, 329, 504, 741
 // we add approx 50 to those values and check to see if we are close
 if (adc_key_in > 1000) return btnNONE; // We make this the 1st option for speed reasons since it will be the most likely result
 // For V1.1 us this threshold
 if (adc_key_in < 50)   return btnRIGHT;  
 if (adc_key_in < 250)  return btnUP; 
 if (adc_key_in < 450)  return btnDOWN; 
 if (adc_key_in < 650)  return btnLEFT; 
 if (adc_key_in < 850)  return btnSELECT;  

 // For V1.0 comment the other threshold and use the one below:
/*
 if (adc_key_in < 50)   return btnRIGHT;  
 if (adc_key_in < 195)  return btnUP; 
 if (adc_key_in < 380)  return btnDOWN; 
 if (adc_key_in < 555)  return btnLEFT; 
 if (adc_key_in < 790)  return btnSELECT;   
*/

 return btnNONE;  // when all others fail, return this...
}

void setup()
{
 Serial.begin(115200);
 lcd.begin(16, 2);              // start the library
 lcd.setCursor(0,0);
 lcd.print("Welcome"); // print a simple message
 resetClock = true;
 pinMode(mosfetA, OUTPUT);
 pinMode(mosfetB, OUTPUT);
 
 setTime(13,56,0,20,8,15); // set time to arbitrary date
 setSyncProvider(RTC.get);  // get the time from the RTC
 
}
 
void selectMenu()
{
  int button = 0; 
  menuOption = 1;
  lcdClear();
  lcd.print("Toggle SD");  

  while (menuOption <= menuOptions)
  {
    button = read_LCD_buttons();
    if (button == btnSELECT)
    {
      timedBeep(shortBeep,1);  
      menuOption++;

      if (menuOption == 2)
      {
        lcdClear();
        lcd.print("Set/Clear SD");
      }
      if (menuOption == 3)
      {
        lcdClear();
        lcd.print("Set Date & Time");            
      }
    } 

    if (button == btnRIGHT)
    {
      if (menuOption == 1)
      {
        timedBeep(shortBeep,1);
        //minuteTimer();
        toggleSD();
        return;
      }
      if (menuOption == 2)
      {
        timedBeep(shortBeep,1);
        //check for existing alarm
        if (SDSet)
        {
          clearSD();
        }
        else
        {
          setSD();
        }
        return;
      }
      if (menuOption == 3)
      {
        timedBeep(shortBeep,1);
        setDateTime();
        return;
      } 
    }
  }
}    

void toggleSD()
{
  if ( SD_running )
  {
    Serial.println("Toggling stop");
    stopSD();
  }
  else
  {
    Serial.println("Toggling start");
    startSD();
  }
}
  
void startSD()
{
  SD_running = true;
  SD_started = now();
  random_pulse(SD_running);
}

void stopSD()
{
  SD_running = false;
  random_pulse(SD_running);
}

void random_pulse(bool status)
{
  if ( status ) 
  {
    SD_vibrate = true;
    SD_A = now();
    SD_AB = random (MIN_SD_PULSE, MAX_SD_PULSE);
    next_action = now() + SD_AB;

    lcd.setCursor(13,0);
    lcd.print("[*]");

    digitalWrite(mosfetB, HIGH);
    Serial.println("Start duty cycle");
  }
  else
  {
    SD_vibrate = false;
    SD_B = now();
    SD_BC = random (MIN_SD_PAUSE, MAX_SD_PAUSE ) * 60; 
    next_action = now() + SD_BC;

    lcd.setCursor(13,0);
    lcd.print("[.]");

    digitalWrite(mosfetB, LOW);
    Serial.println("End duty cycle");
      
  }
}
 

void setSD()
{
  int button = 0;
  char *okno= "OK";
  SD_start_hours = getTimerMinutes("Set SD Hour", SD_start_hours, 23);  
  if (SD_start_hours > 0 && SD_start_hours < 23)
  {
    SD_start_minutes = getTimerMinutes("Set SD Minutes", SD_start_minutes, 59);
    //if (SD_start_minutes > 0)
    if (SD_start_minutes < 60)
    {
        
      SD_length_hours = getTimerMinutes("Set durat. HH", SD_length_hours, 240);
      
      lcdClear();
      lcd.print("SD starts at ");
      lcd.setCursor(0,1);
      //display alarm time
      lcd.print(SD_start_hours);       
      lcd.print(":");
      if (SD_start_minutes < 10)
        lcd.print("0");
      lcd.print(SD_start_minutes);
      
      lcd.setCursor(6,1);
      lcd.print("for ");
      lcd.print(SD_length_hours);

      lcd.setCursor(14,1);
      lcd.print("OK");
      button = 6;
      while (button != btnSELECT && button != btnRIGHT)
      {
        button = read_LCD_buttons();
        if (button == btnUP || button == btnDOWN)
        {
          timedBeep(shortBeep,1);
          if (okno == "OK")
          {
            okno = "NO";
          }
          else
          {
            okno = "OK";
          }
          lcd.setCursor(6,1);
          lcd.print(okno);  
        }
      }

      if (button == btnRIGHT)
      {
        timedBeep(shortBeep,1);
        SDSet = true;
        Alarm.alarmRepeat(SD_start_hours,SD_start_minutes,0 , startSD);  // repeat every day
        
        lcdClear();
        lcd.setCursor(0,0);
        lcd.print("SD Set");
        delay(1000);
        return;       
      }
      else
      {
        timerCancelled("SD");
        return;  
      }   
    }
    else
    {
      timerCancelled("SD");     
    }    
  }     
  else
  {
    timerCancelled("SD");       
  }
}

void setDateTime()
{
  int button = 0;
  char *okno = "OK";

  //get month
  int setMonth = getTimerMinutes("Set Month", lastMonth, 12);
  if (setMonth > 0 && setMonth < 13)
  {
    //get day
    int setDay = getTimerMinutes("Set Day", lastDay, 31);
    if (setDay > 0 && setDay < 32)
    {
      //get year
      int setYear = getTimerMinutes("Set Year", lastYear, 2030);
      if (setYear > 2000 && setYear < 2031)
      {
        //get hour
        int setHour = getTimerMinutes("Set Hour", lastHour, 23);
        if (setHour > 0 && setHour < 24)
        {
          //get minutes
          int setMinute = getTimerMinutes("Set Minute", lastMinute, 59);
          if (setMinute < 60)
          {
            //get ampm
            lcdClear();
            lcd.setCursor(0,1);
            //display alarm time
            lcd.print(setHour);       
            lcd.print(":");
            if (setMinute < 10)
              lcd.print("0");
            lcd.print(setMinute);

            lcd.setCursor(6,1);
            lcd.print(okno);
            button = 6;
            while (button != btnSELECT && button != btnRIGHT)
            {
              button = read_LCD_buttons();
              if (button == btnUP || button == btnDOWN)
              {
                timedBeep(shortBeep,1);
                if (okno == "OK")
                {
                  okno = "NO";
                }
                else
                {
                  okno = "OK";
                }
                lcd.setCursor(6,1);
                lcd.print(okno);         
              }
            }
            if (button == btnRIGHT)
            {
              timedBeep(shortBeep,1);

              //RTC.adjust(DateTime(setYear,setMonth,setDay,setHour,setMinute));
              setTime(setHour,setMinute,0,setDay,setMonth,setYear);
              RTC.set(now());

              lcd.setCursor(0,0);
              lcd.print("Saving...     ");
              delay(1000);
              return;       
            }
            else
            {
              timerCancelled("");
              return;  
            }  
          }
          else
          {
            timerCancelled("");     
          }    
        }     
        else
        {
          timerCancelled("");       
        }
      }
      else
      {
        timerCancelled("");     
      }    
    }     
    else
    {
      timerCancelled("");       
    }
  }
  else
  {
    timerCancelled("");       
  }

}

void minuteTimer()
{
  // 8/1/2012 Pass maxCount to getTimerMinutes
  int timerMinutes = getTimerMinutes("Set Minutes", 0, 60);
  if (timerMinutes > 0)
  {
    timedCountDown(timerMinutes*60, "Minute Timer");
  }
  else
  {
    timerCancelled("Timer");       
  }
  return;
}

void timedCountDown(int secondCount, char countLabel[])
{
  long seconds = 0;
  long minutes = 0; 

  lcdClear();
  lcd.print(countLabel);
  for (int i = secondCount; i >= 0; i--)
  {
    seconds = i;
    minutes = i / 60;
    if (minutes > 0)
    {
      seconds = seconds - (minutes * 60);  
    }     

    if (minutes > 0)
    {
      lcd.setCursor(0,1);
      lcd.print(minutes);
      lcd.print(" min ");
    }
    else
    {
      lcd.setCursor(0,1);
    }
    if (seconds < 10) lcd.print("0");
    lcd.print(seconds);
    lcd.print(" sec remaining");
    if (seconds > 0) delay(1000); 
    if (read_LCD_buttons() == btnSELECT) //cancel
    {
      timerCancelled("Timer");
      i = 0;
      return;
    }
  }
  lcd.setCursor(6,1);
  timedBeep(longBeep,3);
}

// 8/1/2012 Pass maxCount to getTimerMinutes
int getTimerMinutes(char timerText[], int startNum, int maxCount)
{
  int minutes = startNum;
  int button = 0;
  lcdClear();
  lcd.print(timerText);
  lcd.setCursor(0,1);
  lcd.print(minutes);   

  while (button != btnSELECT)
  {
    button = read_LCD_buttons();
    Serial.println(button);
    // 8/1/2012 Pass maxCount to getTimerMinutes
    if (button == btnLEFT)
    {
      if ((minutes + 10) <= maxCount)
      {
        timedBeep(shortBeep,1);
        minutes = minutes + 10;
      }
      else
      {
        timedBeep(shortBeep,2); 
      }
    }
    // 8/1/2012 Pass maxCount to getTimerMinutes
    if (button == btnUP)
    {
      if (minutes < maxCount)
      {
        timedBeep(shortBeep,1);
        minutes++;
      }
      else
      {
        timedBeep(shortBeep,2); 
      }
    }
    if (button == btnDOWN)
    {
      if (minutes > 0)
      {
        timedBeep(shortBeep,1);
        minutes--;
      }
      else
      {
        timedBeep(shortBeep,2); 
      }   
    } 
    if (button == btnRIGHT)
    {
      timedBeep(shortBeep,1);
      return minutes; 
    }
    lcd.setCursor(0,1);
    lcd.print(minutes); 
    lcd.print("   ");
  }
  return 0;
}

void timedBeep(int beepTime, int beepCount)
{
  for (int i = 0; i < beepCount; i ++)
  {
    digitalWrite(beeper, HIGH);
    delay(beepTime);
    digitalWrite(beeper, LOW);
    delay(beepTime);
  }
}

void lcdClear(){
  resetClock = true;
  lcd.clear();
  lcd.begin(16,2);
  lcd.setCursor(0,0); 
}

void clearSD()
{
  int button = 0;
  bool clearIt = true;

  lcdClear();
  lcd.print("SD Starting at");
  lcd.setCursor(0,1);
  lcd.print(SD_start_hours);   
  lcd.print(":");
  lcd.print(SD_start_minutes);
  lcd.print(" for ");
  lcd.print(SD_length_hours);
  lcd.print("H ");

  delay(2000);
  lcdClear();
  lcd.print("Clear SD?");
  lcd.setCursor(0,1);
  lcd.print("Yes");  

  while (button != btnSELECT)
  {
    button = read_LCD_buttons();
    if (button == btnUP)
    {
      timedBeep(shortBeep,1);
      clearIt = !clearIt;
    }
    if (button == btnDOWN)
    {
      timedBeep(shortBeep,1);
      clearIt = !clearIt;
    }
    if (button == btnRIGHT)
    {
      timedBeep(shortBeep,1);
      SDSet = !clearIt;
      if (clearIt)
      {
        lcdClear();
        timedBeep(shortBeep,2);
        lcd.print("SD Cleared!");
        delay(2000);
      }
      return; 
    }
    lcd.setCursor(0,1);
    if (clearIt)
    {
      lcd.print("Yes"); 
    }
    else{
      lcd.print("No ");
    }
  }   
}


void timerCancelled(char message[])
{
  lcdClear();
  lcd.print(message);
  lcd.print(" Cancelled");
  timedBeep(shortBeep,3);    
}

void button_loop()
{
  int button = read_LCD_buttons();
  if (button == btnSELECT)
  {
    timedBeep(shortBeep,1); 
    selectMenu();
  }
}

void loop () {
  
  arduinoClockDisplay();
  
  for (int i = 0; i < 10000; i++)
  {
    button_loop(); //check for button pushed
  }

if ( SD_running )
  {
   long tot_sleep_length =  ( now() - SD_started);
   long time_from_last_A = ( now() - SD_A );
   long time_from_last_B = ( now() - SD_B );
   long SD_total = ( SD_length_hours * 3600 );

   if ( tot_sleep_length > SD_total)
   {
     Serial.print("SD is finished: ");
     Serial.print(tot_sleep_length);
     Serial.print("  ");
     Serial.println(SD_total);
     stopSD();
   }
   else
   {
      if ( SD_vibrate && ( time_from_last_A > ( SD_AB ) ) )
      {
        random_pulse(false); //stop random pulse and enter wait time
      }
      
      else if ( !SD_vibrate && ( time_from_last_B > (SD_BC ) ) )
      {
        random_pulse(true); //start random pulse
      }
      
      lcd.setCursor(11,1);
      int d = next_action - now();
      printDigits(d/60);
      lcd.print(":");
      printDigits(d % 60);
      Serial.println(d);

   }
  
  }
else
  {
   lcd.setCursor(13,0);
   lcd.print("[ ]");
  }   

  Alarm.delay(100); // wait 100ms between clock display


}

void printDigits(byte digits)
{
  // utility function for digital clock display: prints preceding colon and leading 0
  //lcd.print(":");
  if(digits < 10)
    lcd.print('0');
  lcd.print(digits,DEC);
}

void arduinoClockDisplay()
{

  if (day() != lastDay || resetClock == true)
  {
    lcd.begin(16,2);
    lcd.setCursor(0,0);

    printDigits(month());
    lcd.print("/");

    printDigits(day());
    lcd.print("/");

    int thisYear = year();
    lcd.print(thisYear, DEC);
  }
  // 8/1/2012 Fixed default day and hour settings on set date/time
  if (minute() != lastMinute || resetClock == true)
  {
    lcd.setCursor(0,1);
    printDigits(hour());
    lcd.print(":");
    printDigits(minute());
  }

  resetClock = false;

  lastDay = day();
  lastMonth = month();
  lastYear = year();
  lastHour = hour();
  lastMinute = minute();

}
 
