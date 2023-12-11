// https://github.com/mathertel/OneButton
#include <OneButton.h>
#include <WiFi.h>    
#include <HTTPClient.h>
#include <UrlEncode.h>

const char* ssid = "MAKERSPACE";
const char* password = "12345678";

String phoneNumber = "insert number";
String apiKey = "insert API key";

// States of the machine
#define AWAIT 0
#define SETTING 1
#define PENDING 2
#define ALARM 3

// Pins for the button and cards
#define BUTTON_PIN D7

#define LIGHT_PIN D6

unsigned long curr_time = 0;
// Reads 1 when disconnected, 0 when connected
bool is_setting = false;
int card_pins[2] = {2, 3};
int reference[2] = {1, 1};
int states[2] = {AWAIT};
unsigned long missing_times[2] = {NULL};
#define array_size sizeof(card_pins)/sizeof(int)

// Button initialization
OneButton btn = OneButton(
  BUTTON_PIN,  // Input pin for the button
  false,      
  true
);

// Handler function for a single click:
static void handleClick() {
  is_setting = false;
}

static void doubleClick() {
  is_setting = true;
}

void sendMessage(String message){
  // Data to send with HTTP POST
  String url = "https://api.callmebot.com/whatsapp.php?phone=" + phoneNumber + "&apikey=" + apiKey + "&text=" + urlEncode(message);    
  HTTPClient http;
  http.begin(url);

  // Specify content-type header
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  
  // Send HTTP POST request
  int httpResponseCode = http.POST(url);

  // Free resources
  http.end();
}

void setup() {
  pinMode(BUTTON_PIN, INPUT);


  pinMode(LIGHT_PIN, OUTPUT);


  for (int i=0; i<array_size; i++) {
    pinMode(card_pins[i], INPUT_PULLUP);
  }

  // WiFi Setup
  WiFi.begin(ssid, password);
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
  // Single Click event attachment
  btn.attachClick(handleClick);

  // Double Click event attachment with lambda
  btn.attachDoubleClick(doubleClick);
}

void checkState(int i) {
  if ((digitalRead(card_pins[i]) != reference[i]) && (reference[i] != 1)) {
    if (states[i] == AWAIT) {
      states[i] = PENDING;
      missing_times[i] = millis();
    }
  }
  else {
    states[i] = AWAIT;
  }
}

void checkTime(int i) {
  if ((states[i] == PENDING) && (millis() - missing_times[i]) > 3000) {
    states[i] = ALARM;
    missing_times[i] = millis();
  }
}

void setState() {
  for (int i = 0; i < array_size; i++) {
    reference[i] = digitalRead(card_pins[i]);
  }
}

void soundAlarm(int id) {
  sendMessage("Your card in slot " + String(id) + " is missing!");
  states[id] = PENDING;
  missing_times[id] = millis();
}


void loop() {
  // Activate button
  btn.tick();

  if (is_setting) {
    setState();

    analogWrite(LIGHT_PIN,  255);
  }
  else {

    analogWrite(LIGHT_PIN,  0);
    for (int i = 0; i < array_size; i++ ) {
      if (states[i] == AWAIT) {
        checkState(i);
      }
      else if (states[i] == PENDING) {
        // Check whether we've detected the card yet
        checkState(i);
        // Check the time situation
        checkTime(i);
      }
      else if (states[i] == ALARM) {
        soundAlarm(i);
      }
    }
  }
}

