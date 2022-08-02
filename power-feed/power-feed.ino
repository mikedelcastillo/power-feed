#define PIN_X_ENABLE 10
#define PIN_X_DIR 11
#define PIN_X_PULSE 12

#define PULSE_LENGTH_US 50
#define MIN_CYCLE_LENGTH_US 1000

bool xDirection = HIGH;
bool xRunning = true;
unsigned long xDelayUs = MIN_CYCLE_LENGTH_US;
unsigned long lastXPulse = 0;

unsigned long count = 0;

void setup() {
  Serial.begin(115200);

  // Setup X stepper
  pinMode(PIN_X_ENABLE, OUTPUT);
  pinMode(PIN_X_DIR, OUTPUT);
  pinMode(PIN_X_PULSE, OUTPUT);

}

void loop() {
  debug();

  if (xRunning) {
    digitalWrite(PIN_X_ENABLE, LOW);
    if (micros() - lastXPulse >= xDelayUs) {
      digitalWrite(PIN_X_DIR, xDirection);
      digitalWrite(PIN_X_PULSE, HIGH);
      delayMicroseconds(PULSE_LENGTH_US);
      digitalWrite(PIN_X_PULSE, LOW);
      lastXPulse = micros();
    }
  } else {
    digitalWrite(PIN_X_ENABLE, HIGH);
  }
}

unsigned long lastDebug = 0;
unsigned long debugIntervalUs = 100000; // 100ms

void debug() {
  if (micros() - lastDebug >= debugIntervalUs) {
    count++;
    xDelayUs = ((sin((long) count / 20.0) + 1.0) / 2.0) * 4000.0 + MIN_CYCLE_LENGTH_US;
    Serial.println(xDelayUs);
    lastDebug = micros();
  }
}
