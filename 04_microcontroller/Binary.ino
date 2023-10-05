#include <ShiftRegister.h>

const int ledPin0 = 10;
const int ledPin1 = 9;
const int buttonPin = 13;   // Replace with the actual pin your button is connected to
const int buzzerPin = 8;
int buttonState = 0;      // current state of the button
int lastButtonState = LOW;  // previous state of the button
unsigned long startTime = 0;  // the start time of the 10-second window
unsigned long currentTime = 0;  // current time
int buttonPressCount = 0;   // count of button presses
bool printed = true;


void printFullBin(int number)
{
  Serial.println(number);
  for (int bit = 7; bit >= 0; bit--)
  {
    Serial.print(bitRead(number, bit));
    if (bitRead(number, bit) == 0) {
      digitalWrite(ledPin0, HIGH);
    }
    else {
      digitalWrite(ledPin1, HIGH);
    }
    delay(1000);
    digitalWrite(ledPin0, LOW);
    digitalWrite(ledPin1, LOW);
    delay(500);
  }
  Serial.println();
}

void setup() {
  pinMode(buttonPin, INPUT); // Enable internal pull-up resistor
  Serial.begin(9600); // Initialize Serial communication (for debugging)
  delay(1000);
}

void loop() {
  currentTime = millis(); // Get the current time

  // Read the button state
  buttonState = digitalRead(buttonPin);
  // Serial.println(buttonState);

  // Check for a button press (low to high transition)
  if (buttonState == LOW && lastButtonState == HIGH) {
    // Button was pressed, start the timer if it's not already running
    if (currentTime - startTime >= 2000) {
      startTime = currentTime;
      buttonPressCount = 1; // Reset the button press count
      printed = false;
    } else {
      buttonPressCount++; // Increment the button press count
    }
  }

  // Update the last button state
  lastButtonState = buttonState;

  // Check if the 10-second window has elapsed
  if (currentTime - startTime >= 2000 && !printed) {
    // Play the buzzer
    tone(buzzerPin, 262);
    delay(500);
    noTone(buzzerPin);
    // Display the button press count and reset it
    Serial.println(buttonPressCount);
    printFullBin(buttonPressCount);
    buttonPressCount = 0;
    printed = true;
    // Flash the LED lights
    
  }
}

