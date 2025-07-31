#include <LedControl.h>
#include <Servo.h>

#define DISPLAY_DIN 7
#define DISPLAY_CS 4
#define DISPLAY_CLK 2
const int BLUE_OUT  = 11;
const int RED_OUT = 10;
const int GREEN_OUT = 9;
const int DIAL_IN = A0;
const int BUTTON_IN = 13;
const int SERVO_OUT = 6;

LedControl segment_display = LedControl(DISPLAY_DIN, DISPLAY_CLK, DISPLAY_CS);
Servo servo;

float dialPosition = 0;
bool buttonPressed = false;
float lastButtonPress = 0;
float countdownInSeconds = 0;
bool countdown = false;
bool boxLocked = false;

class Colour {
  private:
    int red;
    int green;
    int blue;
  public:
    char* name;
    Colour(int red, int green, int blue) {
      this->red = red;
      this->green = green;
      this->blue = blue;
    }
    Colour(char* name, int red, int green, int blue) {
      this->name = name;
      this->red = red;
      this->green = green;
      this->blue = blue;
    }
    void display() {
      analogWrite(RED_OUT, this->red);
      analogWrite(GREEN_OUT, this->green);
      analogWrite(BLUE_OUT,  this->blue);
    }
};

const Colour WHITE = Colour("white", 255, 255, 255);
const Colour RED = Colour("red", 0, 255, 255);
const Colour GREEN = Colour("green", 255, 0, 255);
const Colour ORANGE = Colour("orange", 0, 0, 255);

Colour currentColour = WHITE;

unsigned int displayUpdateDelay = 250;

void setup() {
  // Inputs
  pinMode(DIAL_IN, INPUT);
  pinMode(BUTTON_IN, INPUT);

  // Outputs
  pinMode(RED_OUT,   OUTPUT);
  pinMode(BLUE_OUT, OUTPUT);
  pinMode(GREEN_OUT,  OUTPUT);

  digitalWrite(BUTTON_IN, HIGH);
  servo.attach(SERVO_OUT);

  segment_display.shutdown(0,false);
  segment_display.setIntensity(0,6);
  segment_display.clearDisplay(0);

  unlockBox();

  Serial.begin(9600);
}

void displayColour(Colour colour) {
  if (currentColour.name != colour.name) {
    currentColour = colour;
    currentColour.display();
  }
}

void loop() {
  buttonPressed = digitalRead(BUTTON_IN);
  if (buttonPressed) {
    beginCountdown();
  } else if (!buttonPressed && !countdown) {
    displayColour(GREEN);
    float newCountdownInSeconds = convertDialPositionToSeconds();
    if (newCountdownInSeconds != countdownInSeconds) {
      countdownInSeconds = newCountdownInSeconds;
      setDisplay(countdownInSeconds);
    }
  }

  while (countdown) {
    delay(1000 - displayUpdateDelay);
    countdownInSeconds -= 1;
    setDisplay(countdownInSeconds);
    if (countdownInSeconds <= 0) {
      endCountdown();
    }
  }
}

void beginCountdown() {
  if (!boxLocked) {
    lockBox();
  } 
  countdown = true;
  Serial.println("begin countdown");
  delay(500);
  displayColour(RED);
}

void endCountdown() {
  if (boxLocked) {
    unlockBox();
  }
  countdown = false;
  Serial.println("end countdown");

  for (int i = 0; i < 3; i++) {
    delay(500);
    segment_display.clearDisplay(0);
    delay(500);
    setDisplay(0);
  }
}

int MAX_SERVO = 85;
int MIN_SERVO = 40;

void lockBox() {
  displayColour(ORANGE);
  Serial.println("lock box");
  servo.write(MAX_SERVO);
  delay(1000);
  boxLocked = true;
}

void unlockBox() {
  displayColour(ORANGE);
  Serial.println("unlock box");
  servo.write(MIN_SERVO);
  delay(1000);
  boxLocked = false;
}

float SECOND = 1;
float MINUTE = SECOND*60;
float HOUR = MINUTE*60;

int convertDialPositionToSeconds() {
  float MIN_DIAL_POS = 0;
  float MAX_DIAL_POS = 1023;

  float MIN_TIME = 0;
  float MAX_TIME = HOUR*2;
  
  dialPosition = analogRead(DIAL_IN);

  float dialPositionPercentage = dialPosition/MAX_DIAL_POS;

  float roundedTime = int(dialPositionPercentage*MAX_TIME/(MINUTE*5))*(MINUTE*5);
  
  return roundedTime;
}

void setDisplay(int timeInSeconds) {

  unsigned int hours_digit_0;
  unsigned int hours_digit_1;
  unsigned int minutes_digit_0;
  unsigned int minutes_digit_1;
  unsigned int seconds_digit_0;
  unsigned int seconds_digit_1;

  if (timeInSeconds == 0) {
    hours_digit_0 = 0;
    hours_digit_1 = 0;
    minutes_digit_0 = 0;
    minutes_digit_1 = 0;
    seconds_digit_0 = 0;
    seconds_digit_1 = 0;
  } else {
    // Get hours digits
    int hours = timeInSeconds/HOUR;
    hours_digit_0 = (hours/10) % 10;
    hours_digit_1 = hours % 10;
    
    // Get minutes digits
    int minutes = (timeInSeconds - hours*HOUR)/MINUTE;
    minutes_digit_0 = (minutes/10) % 10;
    minutes_digit_1 = minutes % 10;

    // Get seconds digits
    int seconds = (timeInSeconds - hours*HOUR - minutes*MINUTE)/SECOND;
    seconds_digit_0 = (seconds/10) % 10;
    seconds_digit_1 = seconds % 10;
  }

  // Set display
  segment_display.setDigit(0, 5, hours_digit_0, false);
  segment_display.setDigit(0, 4, hours_digit_1, true);
  segment_display.setDigit(0, 3, minutes_digit_0, false);
  segment_display.setDigit(0, 2, minutes_digit_1, true);
  segment_display.setDigit(0, 1, seconds_digit_0, false);
  segment_display.setDigit(0, 0, seconds_digit_1, false);
  delay(displayUpdateDelay);
}
