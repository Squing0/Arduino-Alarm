#include <LiquidCrystal.h>

LiquidCrystal lcd(12,11,10,9,8,7);

#define button1 5
#define button2 4
#define piezo 6
#define potentiometer A0
#define tempSensor A1
#define lightSensor A2
#define echoPin 3
#define trigPin 2 

#define encoderA 13  // Connect to CLK pin
#define encoderB 1  // Connect to DT pin
#define encoderBtn 0

int count = 1;  // Initial value, can be 1 to 60
int encoderAState;
int encoderALastState;
int encoderBState;
int encoderBLastState;
boolean encoderPressed;
boolean encoderRls;

int buttonState1, buttonState2 = 0;
int num = 0;
int potVal = 0;
int percentage = 0;
int menuNum = 10;

long duration; 
int distance_cm;
int distance_inch;

// int melody1[] = {262, 294, 330, 349};
// int duration1[] = {500, 500, 500, 500};

// int melody2[] = {392, 440, 494, 523};
// int duration2[] = {300, 300, 300, 300};

//char[] currentTime = "12:40:00";

int hours = 13;
int minutes = 59;
int seconds = 55;

int menuIndex = 0;
String previousScreen = "home";
String currentScreen = "home";




int selected_ringtone = 1; // Current selected ringtone 
bool active_buzzer = false; // Controls if buzzer is audible or not.


struct Tone{ // Individual Tone
  public:
    int pitch;
    int duration;
};
 
struct Tones{ // Tones container
  public:
    char name;
    Tone pattern[2];
};
 
Tones ringtones[2] = { // Ringtones along with tone and duration information
  {"Ringtone 1", {{440, 200}, {600, 200}}},
  {"Ringtone 2", {{300, 200}, {400, 200}}}  
} ;

struct Alarm{ // Alarm Object
  public:
    int hours;
    int minutes;
};


Alarm setAlarms[3] = { //All of the user set alarms
  {14,0},
  {18,0},
  {21,0},
};



void setup() {
  pinMode(button1, INPUT);
  pinMode(button2, INPUT);
  pinMode(potentiometer, INPUT); 
  pinMode(echoPin, INPUT);
  pinMode(tempSensor, INPUT);

  pinMode(trigPin, OUTPUT);
  pinMode(piezo, OUTPUT);
  lcd.begin(16, 2);

  pinMode(encoderA, INPUT);
  pinMode(encoderB, INPUT);
  pinMode(encoderBtn, INPUT_PULLUP);

  encoderALastState = digitalRead(encoderA);

}

int prevMs = 0; 
int toneIndex = 0; // Keep track of tone within ringtone
int toneDuration = 0; // Current tone duration
int toneGap = 0; 
unsigned long endTimeLast = 0;
unsigned long repeatStartTime = 0;
int currentAlarmHrs = 0;
int currentAlarmMins = 0;

void callAlarm() {
  static unsigned long lastToneTime = 0;  // Track the last time a tone was played

  if (!active_buzzer) return;  // Do not run if buzzer is not active

  unsigned long currentMs = millis();  // Get the current time in ms (non-blocking)

  // Check current end time
  if (toneIndex < 2 && currentMs - endTimeLast >= 100) {  // Add 100ms gap
    Tone t = ringtones[selected_ringtone - 1].pattern[toneIndex];  // Get the tone details (pitch, duration)

    // Play tone
    tone(piezo, t.pitch, t.duration); 
    if(currentScreen == "home"){
      lcd.setCursor(15,0);
      //lcd.print("A"); // Seperate alarm indicator, small one if full bottom row text is not desired
      lcd.setCursor(0,1);
      lcd.print("ALARM: ");
      lcd.print(currentAlarmHrs);
      lcd.print(":");
      if(currentAlarmMins < 10){
        lcd.print("0");
      }
      lcd.print(currentAlarmMins);
    };

    delay(100);

    if(currentScreen == "home"){
      lcd.setCursor(15,0);
      lcd.print("");
      lcd.setCursor(0,1);
      lcd.print("");
    };

    endTimeLast = currentMs + t.duration + 50; // Set end time of last note

    toneIndex++; // Move onto next note

    lastToneTime = currentMs; // Set end of last note
  }

  // Check if entire pattern played
  if (toneIndex >= 2) {
    if (currentMs - repeatStartTime >= 1000) {  // 1000ms gap between each tone playing
      toneIndex = 0;  // Reset to start
      repeatStartTime = currentMs;  // Ser time
    }
  }
}


int* getNextAlarm(){

  int convertCurrentHrs = hours * 60 + minutes; // Convert current time (hours and minutes) into minutes
  int minTimeDifference = 24 * 60; // Total minutes per day
  static int nextAlarm[2] = {-1,-1};

  for(int i = 0; i < 3; i++){ // Loop through alarms array
    int calcHours = setAlarms[i].hours;
    int calcMins = setAlarms[i].minutes;

    int convertCalcHrs = calcHours * 60 + calcMins; // Convert alarm array hours and minutes into only minutes

    int timeDifference = convertCalcHrs - convertCurrentHrs; // Calculate difference between the current and alarm
    if(timeDifference < 0){ //Handle cases where alarm is next day
        timeDifference += 24 * 60;
    };

    if (timeDifference > 0 && timeDifference < minTimeDifference) { //Ensure alarm is in the future and that the smallest time difference is used (first upcoming alarm)
      minTimeDifference = timeDifference; // Keep track of closest alarm
      nextAlarm[0] = calcHours;
      nextAlarm[1] = calcMins;
    };

  }

  return nextAlarm;
}

boolean triggerAlarm(){
  for(int i = 0; i < 3; i++){ // Loop through alarms array
    if(setAlarms[i].hours == hours){ // If matching alarm hours
      if(setAlarms[i].minutes == minutes){ //If matching alarm minutes
        if(seconds < 5){ //If seconds are within 5 of the beginning (makes sure alarm does not repeat and responds to users cancelation)
          currentAlarmHrs = setAlarms[i].hours; //Set current alarm hours / minutes for LCD screen use
          currentAlarmMins = setAlarms[i].minutes;
          return true;
        }else{
          return false;
        }
      }else{
        return false;
      }
    }
  }
  return false;
}


unsigned long previousMs = 0;
void timeFetch(){
  unsigned long timeElapsedMs = millis();

  if(timeElapsedMs - previousMs >= 1000){
    seconds++;
    if(seconds >= 60){
      seconds -= 60;
      minutes++;
    }

    if(minutes >= 60){
      minutes -= 60;
      hours++;
    }

    if(hours >= 24){
      hours -= 24;
    }

    if(currentScreen == "home"){
      lcd.clear();
      lcd.print("Time: ");

      if(hours < 10){ // Way here is pretty similar to gpt and feels like it can be improved
        lcd.print(0);
      }
      lcd.print(hours);
      lcd.print(":");

      if(minutes < 10){
        lcd.print(0);
      }
      lcd.print(minutes);
      lcd.print(":");

      if(seconds < 10){
        lcd.print(0);
      }
      lcd.print(seconds);

      if(active_buzzer == false){
        int* nextAlarm = getNextAlarm();

        if(nextAlarm[0] != -1){ //If alarm exists
          lcd.setCursor(0, 1);
          
          lcd.print("Next:");

          lcd.print(nextAlarm[0]);
          lcd.print(":");
          if(nextAlarm[1] < 10){
            lcd.print("0");
          }
          lcd.print(nextAlarm[1]);

        }
      }




      int reading = analogRead(tempSensor);
      float voltage = reading * (5.0 / 1024.0);
      float temperatureC = (voltage - 0.5) * 100;

      if(temperatureC > 10 && temperatureC > 0){
        lcd.setCursor(13,1);
        lcd.print(temperatureC);
        lcd.setCursor(15,1);
        lcd.print("C");
      }else{
        lcd.setCursor(14,1);
        lcd.print(temperatureC);
        lcd.setCursor(15,1);
        lcd.print("C");
      }

    }

    previousMs = timeElapsedMs;

  }

}


void menuScreen(){
  char* menuOptions[5] = {
    "Back",
    "Menu 2",
    "Menu 3",
    "Menu 4",
    "Menu 5",
  };

  lcd.setCursor(0, 0);
  lcd.print("Scroll to select");
  lcd.setCursor(0, 1);
  lcd.print(">");
  lcd.print(menuOptions[menuIndex]);

   // Read the current state of rotary encoder
  int stateA = digitalRead(encoderA);
  int stateB = digitalRead(encoderB);

  // Detect changes in rotary encoder rotation
  if (stateA != encoderALastState) {
    if (stateB != stateA) {  // Clockwise rotation
      menuIndex++;
      if (menuIndex >= 5) {
        menuIndex = 0;  // Loop back to the first option
      }
    } else {  // Counterclockwise rotation
      menuIndex--;
      if (menuIndex < 0) {
        menuIndex = 5 - 1;  // Loop back to the last option
      }
    }

    // Clear and re-display the updated menu
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Scroll to select");
    lcd.setCursor(0, 1);
    lcd.print(">");
    lcd.print(menuOptions[menuIndex]);

    encoderALastState = stateA;  // Update last state of encoderA
  }

  encoderALastState = stateA;  // Update the last state of pinA
  encoderBLastState = stateB;  // Update the last state of pinB

 // Check if the button is pressed (button press will pull to LOW)
  if (digitalRead(encoderBtn) == LOW) {  // Button is pressed
    if (menuOptions[menuIndex] == "Back") {
      currentScreen = "home";  // Go back to the home screen
    }
    delay(200);  // Small delay to debounce button press (prevent multiple presses)
  }

  delay(50);  // Small delay to debounce encoder
}


// NOTE: Put setting time stuff here, store the hours,minutes,seconds as global variables that either the set alarm or set time functions can access
void setTimeScreen(){
  
}


void loop() {
    // Clears the trigPin condition
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  //Sets the trigPin HIGH (ACTIVE) for 10 microseconds
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  duration = pulseIn(echoPin, HIGH);
  // Calculating the distance
  distance_cm = duration * 0.034 / 2; 
  distance_inch = duration * 0.0133 / 2;

  if(currentScreen != previousScreen){ //Check if the screen has updated
    lcd.clear();
    previousScreen = currentScreen;
  }

  timeFetch();
  if(currentScreen == "menu"){
    menuScreen();
  }else if(currentScreen == "setTime"){
    setTimeScreen();
  }

  int encoderBtnState = digitalRead(encoderBtn);  // Read the button state

  if (encoderBtnState == LOW && encoderRls) { // If the button is pressed and was released
      encoderPressed = true;  // Mark as pressed
      encoderRls = false;  // Prevent immediate response
      if (currentScreen == "home") {
          currentScreen = "menu";  // Go to menu
      } else if (currentScreen == "menu") {
          currentScreen = "home";  // Go back to home from menu
      }
      delay(200); // Debounce
  } else if (encoderBtnState == HIGH) {  // If the button released
      encoderPressed = false;  // Mark as released
      encoderRls = true;  // Allow button to be pressed
  };

  if(triggerAlarm() == true){ // Checks alarm status variable to set off alarm
    active_buzzer = true;

  }else if(active_buzzer == true){ //If trigger alarm is returning false (ie time has passed, and the buzzer is still active, then start responding to user actions to cancel etc)
    if(distance_cm < 4){ // Stop alarm
      active_buzzer = false;
      currentAlarmHrs = 0;
      currentAlarmMins = 0;
    };
  }

  callAlarm(); // Calls callAlarm function for buzzer





// // Displays the distance on the Serial Monitor
// Serial.print("Distance: ");
// Serial.print(distance_cm);
// Serial.println(" cm");
// lcd.setCursor(0, 0);
// lcd.print("Distance: ");
// lcd.print(distance_cm);
// lcd.print(" cm");
// lcd.setCursor(0, 1);
// lcd.print("Distance: ");
// lcd.print(distance_inch);
// lcd.print(" in");


  // int reading = analogRead(tempSensor);
  // float voltage = reading * (5.0 / 1024.0);
  // float temperatureC = (voltage - 0.5) * 100;

  // // Print the temperature in Celsius
  // lcd.setCursor(0, 1);
  // lcd.print("Temperature: ");
  // lcd.print(temperatureC);



// buttonState1 = digitalRead(button1);
//   buttonState2 = digitalRead(button2);

//   potVal = analogRead(potentiometer);
//   lcd.setCursor(0, 1);

//   percentage = map(potVal, 22, 1023, 1, 60);
//   int range = 100 / menuNum;

//   lcd.print(percentage);

  // for(int i = 1; i < range; i++){
  //   if(percentage <= i * range){
  //     lcd.print(i);
  //     break;
  //   }
  // }




  if (buttonState1 == HIGH || buttonState2 == HIGH) {
  num++;
  } 



  // tone(piezo, 85); //Set the voltage to high and makes a noise
  // delay(1000);//Waits for 1000 milliseconds
  // noTone(piezo);//Sets the voltage to low and makes no noise
  // delay(1000);//Waits for 1000 milliseconds

  // lcd.setCursor(0, 1);
  // lcd.print(potPal);


  // int lightSensorReading = 0;
  // lightSensorReading = analogRead(lightSensor);
  // lcd.setCursor(0, 1);
  // lcd.print(lightSensorReading);
  // delay(100);

  // int stateA = digitalRead(encoderA);  // Read the current state of pinA
  // int stateB = digitalRead(encoderB);  // Read the current state of pinB

  // // Detect changes in rotary encoder rotation
  // if (stateA != encoderALastState) {
  //   if (stateB != stateA) {  // Clockwise rotation
  //     value++;
  //   } else {  // Counterclockwise rotation
  //     value--;
  //   }

  //   // Limit the value to be between 1 and 60
  //   if (value > 60) {
  //     value = 1;
  //   }
  //   if (value < 1) {
  //     value = 60;
  //   }

  //   lcd.clear();
  //   lcd.print("Minutes: ");
  //   lcd.print(value);  // Display the current value
  // }

  // encoderALastState = stateA;  // Update the last state of pinA
  // encoderBLastState = stateB;  // Update the last state of pinB

  // delay(50);  // Small delay to debounce the encoder

//   encoderAState= digitalRead(encoderA);

//   if(encoderAState != encoderALastState){
//     if (digitalRead(encoderB) != encoderAState){
//       count++;
//     }
//     else{
//       count--;
//     }

//     if (count > 60){
//     count = 1;
//   }
//   else if (count < 1){
//     count = 60;
//   }

 

// lcd.clear();
//   lcd.print("Minutes: ");
//   lcd.print(count);  // Display the current value

//   encoderALastState = encoderAState;
//   }
  
  // if(digitalRead(encoderBtn) == HIGH){
  //   lcd.clear();
  //   lcd.print("YES");
  // }
  // else{
  //   lcd.clear();
  //   lcd.print("NO");
  // }

}


  

