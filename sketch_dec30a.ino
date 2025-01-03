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
int seconds = 40;

int alarm1Hours = 14;
int alarm1Minutes = 0;

int alarm2Hours = 0;
int alarm2Minutes = 0;

int alarm3Hours = 0;
int alarm3Minutes = 0;

int selected_ringtone = 1;
bool active_buzzer = false;

struct Tone{
  public:
    int pitch;
    int duration;
};
 
struct Tones{
  public:
    char name;
    Tone pattern[2];
};
 
Tones ringtones[2] = {
  {"Ringtone 1", {{440, 500}, {600, 500}}},
  {"Ringtone 2", {{300, 200}, {400, 200}}}  
} ;



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


void callAlarm(){
  if(active_buzzer == false) return;
 
  for(int i = 0; i < 2; i++){
    Tone t = ringtones[selected_ringtone - 1].pattern[i];
    tone(piezo, t.pitch, t.duration);
    delay(t.duration + 50);
  }
  delay(50);
 
}

void loop() {
    // Clears the trigPin condition
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  //Sets the trigPin HIGH (ACTIVE) for 10 microseconds
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  callAlarm(); // Calls callAlarm function for buzzer

  duration = pulseIn(echoPin, HIGH);
  // Calculating the distance
  distance_cm = duration * 0.034 / 2; 
  distance_inch = duration * 0.0133 / 2;
  

  seconds++;

  if (seconds == 60){
    seconds = 0;
    minutes++;
    if(minutes == 60){
      minutes = 0;
      hours++;
      if(hours == 24){
        hours = 0;
      }
    }
  }

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

  lcd.setCursor(0, 1);

  lcd.print("Next:");

  if(alarm1Hours < 10){
    lcd.print("0");
  }
  lcd.print(alarm1Hours);

  lcd.print(":");

  if(alarm1Minutes < 10){
    lcd.print("0");
  }
  lcd.print(alarm1Minutes);

  lcd.setCursor(15,1);

int reading = analogRead(tempSensor);
float voltage = reading * (5.0 / 1024.0);
float temperatureC = (voltage - 0.5) * 100;

lcd.print(temperatureC);
lcd.print("C");

if(hours == alarm1Hours && minutes == alarm1Minutes){
  active_buzzer = true;

  if(distance_cm < 4){
     alarm1Hours = 0;
     alarm1Minutes = 0;
     active_buzzer = false;
   }
}



















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

  delay(1000);

}


  

