#include <WiFi.h>
#include <AccelStepper.h>
// #include <MultiStepper.h>

// initialize stepper motors
AccelStepper stepper_Y(AccelStepper::DRIVER, 5, 6); // initialise accelstepper for a two wire board (motor 1)
AccelStepper stepper_X1(AccelStepper::DRIVER, 0, 1); // initialise accelstepper for a two wire board (motor 3)
AccelStepper stepper_X2(AccelStepper::DRIVER, 18, 19); // initialise accelstepper for a two wire board (motor 2)

// initialize pin for limit switch
int limitPin = 2;

// initialize pins for LED control
int PIN_RED = 9;
int PIN_GREEN = 10;
int PIN_BLUE = 8;

// initialize variables for LED rainbow effect
int led_counter = 0;
int led_num_colors = 255;
int led_animation_timer = 0;
int led_animation_delay = 50;

// MultiStepper steppers;
// int state = 0;

// set Wifi Network parameters
const char* ssid     = "MAKERSPACE";
const char* password = "12345678";

WiFiServer server(80);


// function to zero out position of stepper motor Y
void zero() {
  Serial.println("zeroing the motor...");
  stepper_Y.setSpeed(-700);

  while(digitalRead(limitPin) == 1) {
    stepper_Y.runSpeed();
  }

  stepper_Y.setCurrentPosition(0);
  Serial.println("done zeroing.");
}


void setup() {

  Serial.begin(115200);
  delay(10);

  // We start by connecting to a WiFi network

  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  
  server.begin();

  delay(500);

  // now to set up the robot hardware

  // stepper motor setup:
  // Y motor moving the end effector
  stepper_Y.setMaxSpeed(1200.0);
  stepper_Y.setSpeed(700.0);
  stepper_Y.setAcceleration(1000.0);

  // X motors moving the wheels
  stepper_X1.setMaxSpeed(1200.0);
  stepper_X1.setSpeed(700.0); 
  stepper_X1.setAcceleration(0);
  stepper_X1.setCurrentPosition(0);

  stepper_X2.setMaxSpeed(1200.0);
  stepper_X2.setSpeed(700.0);
  stepper_X2.setAcceleration(0); 
  stepper_X2.setCurrentPosition(0);

  // steppers.addStepper(stepper_Y);
  // steppers.addStepper(stepper_X1);
  // steppers.addStepper(stepper_X2);

  // set up pin modes for limit switch and LED
  pinMode(limitPin, INPUT_PULLUP);

  pinMode(PIN_RED, OUTPUT);
  pinMode(PIN_BLUE, OUTPUT);
  pinMode(PIN_GREEN, OUTPUT);

  // make sure the LED is off
  analogWrite(PIN_RED, 0);
  analogWrite(PIN_GREEN, 0);
  analogWrite(PIN_BLUE, 0);

  delay(500);

  // zero out the Y motor
  zero();

  Serial.println("done setup.");

  analogWrite(PIN_RED, 0);
  analogWrite(PIN_GREEN, 255);
  analogWrite(PIN_BLUE, 255);
}

void loop() {
  WiFiClient client = server.available();   // listen for incoming clients
  
  if (client) {                             // if you get a client,
    Serial.println("New Client.");           // print a message out the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected()) {            // loop while the client's connected
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        if (c == '\n') {                    // if the byte is a newline character

          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println();

            Serial.println("debug");
            // the content of the HTTP response follows the header
            // add links to make the robot draw things
            client.print("Welcome to the Lite Brite 3000 Rover!<br>"); // we can pick a better name lol
            client.println();
            client.print("<a href=\"/S\">Draw a square!</a><br>"); // link to draw a square
            client.print("<a href=\"/C\">Draw a circle!</a><br>"); // link to draw a circle

            // The HTTP response ends with another blank line:
            client.println();
            // break out of the while loop:
            break;
          } else {    // if you got a newline, then clear currentLine:
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }

        // Logic to draw a shape based on what link a user has clicked on:
        if (currentLine.endsWith("GET /S")) {
          draw_box();
        }
        else if (currentLine.endsWith("GET /C")) {
          draw_circle();
        }
      }
    }
  
    // close the connection:
    client.stop();
    Serial.println("Client Disconnected.");
  }
}


void calibrate() {
  // set the LED to a different color
  analogWrite(PIN_RED, 0);
  analogWrite(PIN_GREEN, 0);
  analogWrite(PIN_BLUE, 0);
  
  // set X motors to 0 and move the end effector away from the limit switch
  stepper_Y.moveTo(700);
  stepper_X1.moveTo(0);
  stepper_X2.moveTo(0);
  stepper_Y.setSpeed(500);
  stepper_X1.setSpeed(500);
  stepper_X2.setSpeed(500);

  // run the motors towards their target positions
  while (stepper_Y.distanceToGo() != 0) {
    stepper_Y.runSpeedToPosition();
    stepper_X1.runSpeedToPosition();
    stepper_X2.runSpeedToPosition();
  }

  zero();
  delay(500);
}


// function to draw a circle with diameter = length of robot (roughly)
void draw_circle() {
  calibrate();

  analogWrite(PIN_RED, 255);
  analogWrite(PIN_GREEN, 0);
  analogWrite(PIN_BLUE, 100);

  // keep track of which quarter of the circle we're on
  int arc = 0;

  // define the circle with four points, each holding a coordinate for each motor
  long circle_coords[4][3] = {{1400, 350, 350}, {1400, -350, -350}, {0, -350, -350}, {0, 0, 0}};
  // long circle_speed[4][3] = {{800, 400, 400}, {800, -400, -400}, {-800, -400, -400}, {-800, 400, 400}};

  // set motor accelerations so that X and Y move proportionally
  stepper_Y.setAcceleration(600);
  stepper_X1.setAcceleration(400);
  stepper_X2.setAcceleration(400);

  delay(500);

  // draw the four quadrants of the circle
  while (arc < 4) {
    // change the LED color
    // updateColor();

    // Y motor only changes direction once
    if (arc == 0 || 2) {
      stepper_Y.moveTo(circle_coords[arc][0]);
    }
    
    // X motors change direction twice
    if (arc != 2)
    stepper_X1.moveTo(circle_coords[arc][1]);
    stepper_X2.moveTo(circle_coords[arc][2]);

    // run all the motors toward their target positions
    stepper_Y.run();
    stepper_X1.run();
    stepper_X2.run();

    // if the motors are at target positions, move on to the next quadrant
    if (stepper_Y.distanceToGo() == 0 ||
        stepper_X1.distanceToGo() == 0 ||
        stepper_X2.distanceToGo() == 0) {
      arc++;
    }
  }

  analogWrite(PIN_RED, 0);
  analogWrite(PIN_GREEN, 0);
  analogWrite(PIN_BLUE, 0);

  stepper_Y.moveTo(700);
  while (stepper_Y.distanceToGo() != 0) {
    stepper_Y.run();
  }

  analogWrite(PIN_RED, 0);
  analogWrite(PIN_GREEN, 255);
  analogWrite(PIN_BLUE, 255);
}


// function to draw a square of side length = length of robot (roughly)
void draw_box() {
  calibrate();

  analogWrite(PIN_RED, 0);
  analogWrite(PIN_GREEN, 100);
  analogWrite(PIN_BLUE, 250);

  // keep track of which side of the square we're on
  int side = 0;

  // define the circle with four points, each holding a coordinate for each motor
  long square_coords[4][3] = {{0, 524, 524}, {1400, 524, 524}, {1400, 0, 0}, {0, 0, 0}};
  long square_speed[4][3] = {{500, 500, 500}, {500, -500, -500}, {-500, -500, -500}, {-500, 500, 500}};

  // draw the four sides of the square
  while (side < 4) {
    // change the color of the LED
    // updateColor();

    // set the target positions of the motors to the appropriate point
    stepper_Y.moveTo(square_coords[side][0]);
    stepper_X1.moveTo(square_coords[side][1]);
    stepper_X2.moveTo(square_coords[side][2]);
    stepper_Y.setSpeed(square_speed[side][0]);
    stepper_X1.setSpeed(square_speed[side][1]);
    stepper_X2.setSpeed(square_speed[side][2]);

    // run all the motors toward their target positions
    stepper_Y.runSpeedToPosition();
    stepper_X1.runSpeedToPosition();
    stepper_X2.runSpeedToPosition();

    // if we're done with this side, move to the next one
    if (stepper_Y.distanceToGo() == 0 &&
        stepper_X1.distanceToGo() == 0 &&
        stepper_X2.distanceToGo() == 0) {
      side++;
    }
  }

  // set LED back to default color
  analogWrite(PIN_RED, 0);
  analogWrite(PIN_GREEN, 255);
  analogWrite(PIN_BLUE, 255);
}


// function to change the color of the LED (not working correctly)
void updateColor() {
  // increment the timer every loop...
  led_animation_timer++;

  // ...but only change the color once per given time interval
  if (led_animation_timer >= led_animation_delay){

    // This part takes care of displaying the
    // color changing in reverse by counting backwards if counter
    // is above the number of available colors  
    float color_number = led_counter > led_num_colors ? led_counter - led_num_colors: led_counter;
    
    // Play with the saturation and brightness values
    // to see what they do
    float saturation = 1.0; // Between 0 and 1 (0 = gray, 1 = full color)
    float brightness = 1.0; // Between 0 and 1 (0 = dark, 1 is full brightness)
    float hue = (color_number / float(led_num_colors)) * 360; // Number between 0 and 360
    long color = HSBtoRGB(hue, saturation, brightness); 
    
    // Get the red, blue and green parts from generated color
    int red = color >> 16 & 255;
    int green = color >> 8 & 255;
    int blue = color & 255;
    setColor(red, green, blue);
    
    // Counter can never be greater then 2 times the number of available colors
    // the colorNumber = line above takes care of counting backwards (nicely looping animation)
    // when counter is larger then the number of available colors
    led_counter = (led_counter + 1) % (led_num_colors * 2);
    
    // If you uncomment this line the color changing starts from the
    // beginning when it reaches the end (animation only plays forward)
    // counter = (counter + 1) % (numColors);
    led_animation_timer = 0;
  }
}


// helper function for updateColor()
void setColor (unsigned char red, unsigned char green, unsigned char blue) 
{        
    analogWrite(PIN_RED, red);
    analogWrite(PIN_GREEN, green);
    analogWrite(PIN_BLUE, blue);
} 


// helper function for updateColor()
long HSBtoRGB(float _hue, float _sat, float _brightness) {
    float red = 0.0;
    float green = 0.0;
    float blue = 0.0;
    
    if (_sat == 0.0) {
        red = _brightness;
        green = _brightness;
        blue = _brightness;
    } else {
        if (_hue == 360.0) {
            _hue = 0;
        }
        int slice = _hue / 60.0;
        float hue_frac = (_hue / 60.0) - slice;
        float aa = _brightness * (1.0 - _sat);
        float bb = _brightness * (1.0 - _sat * hue_frac);
        float cc = _brightness * (1.0 - _sat * (1.0 - hue_frac));
        
        switch(slice) {
            case 0:
                red = _brightness;
                green = cc;
                blue = aa;
                break;
            case 1:
                red = bb;
                green = _brightness;
                blue = aa;
                break;
            case 2:
                red = aa;
                green = _brightness;
                blue = cc;
                break;
            case 3:
                red = aa;
                green = bb;
                blue = _brightness;
                break;
            case 4:
                red = cc;
                green = aa;
                blue = _brightness;
                break;
            case 5:
                red = _brightness;
                green = aa;
                blue = bb;
                break;
            default:
                red = 0.0;
                green = 0.0;
                blue = 0.0;
                break;
        }
    }
    long ired = red * 255.0;
    long igreen = green * 255.0;
    long iblue = blue * 255.0;
    
    return long((ired << 16) | (igreen << 8) | (iblue));
}