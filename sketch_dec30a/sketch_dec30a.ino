#include <LiquidCrystal.h>

LiquidCrystal lcd(2,3,4,5,6,7);

#define piezo 10
#define tempSensor A1
#define lightSensor A2
#define echoPin 13
#define trigPin 12 

#define encoderA 8  // Connect to CLK pin
#define encoderB 9  // Connect to DT pin
#define encoderBtn 11

int count = 1;  // Initial value, can be 1 to 60
int encoderAState;
int encoderALastState;
int encoderBState;
int encoderBLastState;
boolean encoderPressed;
boolean encoderRls;

int encoderBtnState; // May need to change way that have done this

int potVal = 0;
int percentage = 0;
int menuNum = 10;

long duration; 
int distance_cm;
int distance_inch;

int hours = 13;
int minutes = 59;
int seconds = 55;

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

void setCurrentTime() {
  int currentStep = 0; // 0: Set Hours, 1: Set Minutes
  bool timeSet = false;

  while (!timeSet) {
    encoderAState = digitalRead(encoderA);

    // Handle encoder rotation
    if (encoderAState != encoderALastState) {
      if (digitalRead(encoderB) != encoderAState) {
        if (currentStep == 0) {
          hours++;
          if (hours > 23) hours = 0;
        } else if (currentStep == 1) {
          minutes++;
          if (minutes > 59) minutes = 0;
        }
      } else {
        if (currentStep == 0) {
          hours--;
          if (hours < 0) hours = 23;
        } else if (currentStep == 1) {
          minutes--;
          if (minutes < 0) minutes = 59;
        }
      }

      encoderALastState = encoderAState;

      // Update LCD
      lcd.clear();
      if (currentStep == 0) {
        lcd.print("Set Hours: ");
        lcd.setCursor(0, 1);
        lcd.print(hours);
      } else if (currentStep == 1) {
        lcd.print("Set Minutes: ");
        lcd.setCursor(0, 1);
        lcd.print(minutes);
      }
    }

    // Check encoder button press
    encoderBtnState = digitalRead(encoderBtn);
    if (encoderBtnState == LOW) {
      delay(300); // Debounce delay
      if (currentStep == 0) {
        currentStep = 1; // Move to setting minutes
      } else if (currentStep == 1) {
        timeSet = true; // Exit loop after setting minutes
      }
    }
  }

  // Display confirmation
  lcd.clear();
  lcd.print("Time Set: ");
  lcd.setCursor(0, 1);
  lcd.print(hours < 10 ? "0" : "");
  lcd.print(hours);
  lcd.print(":");
  lcd.print(minutes < 10 ? "0" : "");
  lcd.print(minutes);
  delay(2000); // Display confirmation for 2 seconds
}



void setup() {
  pinMode(echoPin, INPUT);
  pinMode(tempSensor, INPUT);

  pinMode(trigPin, OUTPUT);
  pinMode(piezo, OUTPUT);
  lcd.begin(16, 2);

  pinMode(encoderA, INPUT);
  pinMode(encoderB, INPUT);
  pinMode(encoderBtn, INPUT_PULLUP);

  encoderALastState = digitalRead(encoderA);

  setCurrentTime();

}

int prevMs = 0; 
int toneIndex = 0; // Keep track of tone within ringtone
int toneDuration = 0; // Current tone duration
int toneGap = 0; 
unsigned long endTimeLast = 0;
unsigned long repeatStartTime = 0;

void callAlarm() {
  static unsigned long lastToneTime = 0;  // Track the last time a tone was played

  if (!active_buzzer) return;  // Do not run if buzzer is not active

  unsigned long currentMs = millis();  // Get the current time in ms (non-blocking)

  // Check current end time
  if (toneIndex < 2 && currentMs - endTimeLast >= 100) {  // Add 100ms gap
    Tone t = ringtones[selected_ringtone - 1].pattern[toneIndex];  // Get the tone details (pitch, duration)

    // Play tone
    tone(piezo, t.pitch, t.duration); 

    delay(100);

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

int menuIndex = 0;
String previousScreen = "home";
String currentScreen = "home";

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



      lcd.setCursor(15,1);

      int reading = analogRead(tempSensor);
      float voltage = reading * (5.0 / 1024.0);
      float temperatureC = (voltage - 0.5) * 100;

      lcd.print(temperatureC);
      lcd.print("C");

    }

    previousMs = timeElapsedMs;

  }

}

void menuScreen(){
  char* menuOptions[5] = {
    "Back",
    "Set current time",
    "Update alarm",
    "Delete alarm",
    "Set ringtone",
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
    if (menuOptions[menuIndex] == "Set current time"){
      setCurrentTime();
      currentScreen = "home";
    }
    if(menuOptions[menuIndex] == "Delete alarm"){
      deleteAlarm();
      currentScreen = "Home";
    }
    delay(200);  // Small delay to debounce button press (prevent multiple presses)
  }

  delay(50);  // Small delay to debounce encoder
}

void setAlarm() {
  int currentStep = 0; // 0: Set Hours, 1: Set Minutes
  bool timeSet = false;

  while (!timeSet) {
    encoderAState = digitalRead(encoderA);

    // Handle encoder rotation
    if (encoderAState != encoderALastState) {
      if (digitalRead(encoderB) != encoderAState) {
        if (currentStep == 0) {
          hours++;
          if (hours > 23) hours = 0;
        } else if (currentStep == 1) {
          minutes++;
          if (minutes > 59) minutes = 0;
        }
      } else {
        if (currentStep == 0) {
          hours--;
          if (hours < 0) hours = 23;
        } else if (currentStep == 1) {
          minutes--;
          if (minutes < 0) minutes = 59;
        }
      }

      encoderALastState = encoderAState;

      // Update LCD
      lcd.clear();
      if (currentStep == 0) {
        lcd.print("Set Hours: ");
        lcd.setCursor(0, 1);
        lcd.print(hours);
      } else if (currentStep == 1) {
        lcd.print("Set Minutes: ");
        lcd.setCursor(0, 1);
        lcd.print(minutes);
      }
    }

    // Check encoder button press
    encoderBtnState = digitalRead(encoderBtn);
    if (encoderBtnState == LOW) {
      delay(300); // Debounce delay
      if (currentStep == 0) {
        currentStep = 1; // Move to setting minutes
      } else if (currentStep == 1) {
        timeSet = true; // Exit loop after setting minutes
      }
    }
  }

  // Display confirmation
  lcd.clear();
  lcd.print("Time Set: ");
  lcd.setCursor(0, 1);
  lcd.print(hours < 10 ? "0" : "");
  lcd.print(hours);
  lcd.print(":");
  lcd.print(minutes < 10 ? "0" : "");
  lcd.print(minutes);
  delay(2000); // Display confirmation for 2 seconds
}

// void deleteAlarm(){
//   int alarmMenuIndex = 0;
//   bool alarmSet = false;

//   while(!alarmSet){
//   lcd.setCursor(0, 0);
//   lcd.print("Scroll to select");
//   lcd.setCursor(0, 1);
//   lcd.print(">");
//   lcd.print(setAlarms[alarmMenuIndex].hours);
//   lcd.print(":");
//   lcd.print(setAlarms[alarmMenuIndex].minutes);

//    // Read the current state of rotary encoder
//   int stateA = digitalRead(encoderA);
//   int stateB = digitalRead(encoderB);

//   // Detect changes in rotary encoder rotation
//   if (stateA != encoderALastState) {
//     if (stateB != stateA) {  // Clockwise rotation
//       alarmMenuIndex++;
//       if (alarmMenuIndex >= 3) {
//         alarmMenuIndex = 0;  // Loop back to the first option
//       }
//     } else {  // Counterclockwise rotation
//       alarmMenuIndex--;
//       if (alarmMenuIndex < 0) {
//         alarmMenuIndex = 3 - 1;  // Loop back to the last option
//       }
//     }

//     // Clear and re-display the updated menu
//     lcd.clear();
//     lcd.setCursor(0, 0);
//     lcd.print("Scroll to select");
//     lcd.setCursor(0, 1);
//     lcd.print(">");
//     lcd.print(setAlarms[alarmMenuIndex].hours);
//     lcd.print(":");
//     lcd.print(setAlarms[alarmMenuIndex].minutes);

//     encoderALastState = stateA;  // Update last state of encoderA
//   }

//   encoderALastState = stateA;  // Update the last state of pinA
//   encoderBLastState = stateB;  // Update the last state of pinB

//  // Check if the button is pressed (button press will pull to LOW)
//   if (digitalRead(encoderBtn) == LOW) {  // Button is pressed
//     setAlarms[alarmMenuIndex].hours == 0;
//     setAlarms[alarmMenuIndex].minutes == 0;
//     alarmSet = true;
//     delay(200);  // Small delay to debounce button press (prevent multiple presses)
//   }

//   delay(50);  // Small delay to debounce encoder

//   }


  
// }

void deleteAlarm() {
  int alarmIndex = 0; // Start with the first alarm
  bool alarmDeleted = false;

  while (!alarmDeleted) {
    lcd.clear();
    lcd.print("Del Alarm: ");
    lcd.setCursor(0, 1);

    // Display the selected alarm
    if (setAlarms[alarmIndex].hours < 10) lcd.print("0");
    lcd.print(setAlarms[alarmIndex].hours);
    lcd.print(":");
    if (setAlarms[alarmIndex].minutes < 10) lcd.print("0");
    lcd.print(setAlarms[alarmIndex].minutes);

    // Read rotary encoder for navigation
    encoderAState = digitalRead(encoderA);
    if (encoderAState != encoderALastState) {
      if (digitalRead(encoderB) != encoderAState) { // Clockwise
        alarmIndex++;
        if (alarmIndex >= 3) alarmIndex = 0; // Wrap around
      } else { // Counterclockwise
        alarmIndex--;
        if (alarmIndex < 0) alarmIndex = 2; // Wrap around
      }
      encoderALastState = encoderAState;
    }

    // Check for button press to delete
    if (digitalRead(encoderBtn) == LOW) {
      delay(200); // Debounce
      // Shift alarms to remove the selected one
      for (int i = alarmIndex; i < 2; i++) {
        setAlarms[i] = setAlarms[i + 1];
      }
      // Clear the last alarm slot
      setAlarms[2].hours = -1;
      setAlarms[2].minutes = -1;

      lcd.clear();
      lcd.print("Alarm Deleted!");
      delay(2000);
      alarmDeleted = true;
    }
    delay(50); // Debounce
  }
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
    if(distance_cm < 4){
      active_buzzer = false;
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

  // int stateA = digitalRead(CLKPin);  // Read the current state of pinA
  // int stateB = digitalRead(DTPin);  // Read the current state of pinB

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