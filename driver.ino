#define LIMIT_SWITCH_0 0
#define LIMIT_SWITCH_1 1
#define A_PIN 2
#define B_PIN 3
#define C_PIN 4
#define ENCODER_A 8
#define ENCODER_B 9
#define ERROR_FLAG 7
#define EN_1 9
#define EN_2 8

#define ENCODER_LIMIT 255

volatile bool motor_shunt;  // 0: limit switch is TRIGGERED,    1: limit switch is OPEN
volatile bool motor_shunt_identifier;  // 0: limit switch 0 is TRIGGERED,    1: limit switch 0 is TRIGGERED


bool direction; // 0: CW, 1: CCW
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
  if (encoder_state != prev_encoder_state){     
    if (digitalRead(ENCODER_B) != encoder_state) { 
      counter++;
    } else {
      counter--;
    }

    if ( counter < 0) {
      counter = 0;
    } else if (counter > ENCODER_LIMIT) {
      counter = ENCODER_LIMIT;
    }
  } 
  prev_encoder_state = encoder_state;
}


void setup() {

  // pin setup
  pinMode(LIMIT_SWITCH_0, INPUT);
  pinMode(LIMIT_SWITCH_1, INPUT);

  pinMode(A_PIN, OUTPUT);
  pinMode(B_PIN, OUTPUT);
  pinMode(C_PIN, OUTPUT);

  pinMode(ENCODER_A, INPUT);
  pinMode(ENCODER_B, INPUT);

  pinMode(ERROR_FLAG, INPUT); 

  pinMode(EN_1, OUTPUT);
  pinMode(EN_2, OUTPUT);

  // attach interrupts to limit switches
  attachInterrupt(digitalPinToInterrupt(LIMIT_SWITCH_0), trigger_motor_shunt_0, CHANGE);
  attachInterrupt(digitalPinToInterrupt(LIMIT_SWITCH_1), trigger_motor_shunt_1, CHANGE);

  digitalWrite(EN_1, HIGH); // enable motor channel
}

void loop() {
  read_encoder();

  direction = (counter < target_pos) ? 0 : 1;

  if(!motor_shunt) {
    while(abs(counter - target_pos) > 5) {
      if(direction) {
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