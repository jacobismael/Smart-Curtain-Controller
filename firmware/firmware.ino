#include <WiFi.h>

#define LIMIT_SWITCH_0 18
#define LIMIT_SWITCH_1 17
#define A_PIN 39
#define B_PIN 32
#define ENCODER_A 25
#define ENCODER_B 27

#define ENCODER_LIMIT 255

volatile bool motor_shunt;             // 0: limit switch is TRIGGERED,    1: limit switch is OPEN
volatile bool motor_shunt_identifier;  // 0: limit switch 0 is TRIGGERED,    1: limit switch 0 is TRIGGERED


bool direction;  // 0: CW, 1: CCW
int counter = 0;

int target_pos = 0;

int encoder_state = 0;
int prev_encoder_state = encoder_state;

void trigger_motor_shunt_0() {
  motor_shunt = 1;
}

void trigger_motor_shunt_1() {
  motor_shunt = 1;
}

// read the encoder value, encode the readings into the state data
void read_encoder() {
  encoder_state = digitalRead(ENCODER_A);
  if (encoder_state != prev_encoder_state) {
    if (digitalRead(ENCODER_B) != encoder_state) {
      counter++;
    } else {
      counter--;
    }

    if (counter < 0) {
      counter = 0;
    } else if (counter > ENCODER_LIMIT) {
      counter = ENCODER_LIMIT;
    }
  }
  prev_encoder_state = encoder_state;
}

const char *ssid = "SETUP-7DD6";
const char *password = "boast2850canyon";

NetworkServer server(80);


void drive_motor(int target_pos) {
  read_encoder();
  direction = (counter < target_pos) ? 0 : 1;

  if (!motor_shunt) {
    while (abs(counter - target_pos) > 5) {
      if (direction) {
        digitalWrite(A_PIN, HIGH);
        digitalWrite(B_PIN, LOW);
      } else {
        digitalWrite(A_PIN, LOW);
        digitalWrite(B_PIN, HIGH);
      }
      delay(100);
    }
  } else {
    if (motor_shunt_identifier) {
      digitalWrite(A_PIN, HIGH);
      digitalWrite(B_PIN, LOW);
    } else {
      digitalWrite(A_PIN, LOW);
      digitalWrite(B_PIN, HIGH);
    }
  }
}


void setup() {
  Serial.begin(115200);
  // pin setup
  pinMode(LIMIT_SWITCH_0, INPUT);
  pinMode(LIMIT_SWITCH_1, INPUT);

  pinMode(A_PIN, OUTPUT);
  pinMode(B_PIN, OUTPUT);

  pinMode(ENCODER_A, INPUT);
  pinMode(ENCODER_B, INPUT);

  // attach interrupts to limit switches
  attachInterrupt(digitalPinToInterrupt(LIMIT_SWITCH_0), trigger_motor_shunt_0, CHANGE);
  attachInterrupt(digitalPinToInterrupt(LIMIT_SWITCH_1), trigger_motor_shunt_1, CHANGE);

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
}

void loop() {

  NetworkClient client = server.accept();  // listen for incoming clients

  if (client) {                     // if you get a client,
    Serial.println("New Client.");  // print a message out the serial port
    String currentLine = "";        // make a String to hold incoming data from the client
    while (client.connected()) {    // loop while the client's connected
      if (client.available()) {     // if there's bytes to read from the client,
        char c = client.read();     // read a byte, then
        Serial.write(c);            // print it out the serial monitor
        if (c == '\n') {            // if the byte is a newline character

          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println();

            // the content of the HTTP response follows the header:
            client.print("Click <a href=\"/H\">here</a> to open.<br>");
            client.print("Click <a href=\"/L\">here</a> to close.<br>");

            // The HTTP response ends with another blank line:
            client.println();
            // break out of the while loop:
            break;
          } else {  // if you got a newline, then clear currentLine:
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }

        // Check to see if the client request was "GET /H" or "GET /L":
        if (currentLine.endsWith("GET /H")) {
          Serial.println("open");
          drive_motor(255);
        }
        if (currentLine.endsWith("GET /L")) {
          Serial.println("close");
          drive_motor(0);
        }
      }
    }
    // close the connection:
    client.stop();
    Serial.println("Client Disconnected.");
  }
}