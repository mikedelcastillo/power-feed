#define PIN_X_ENABLE 10
#define PIN_X_DIR 11
#define PIN_X_PULSE 12

#define PULSE_LENGTH_US 3 // microseconds
#define MIN_CYCLE_LENGTH_US 800 // microseconds
#define START_CYCLE_LENGTH_US 5000 // microseconds

class FeedStepper {
  private:
    int enablePin;
    int dirPin;
    int pulsePin;
    bool invertDir = false;

    bool forward = true;
    bool enabled = false;
    bool running = false;
    unsigned long lastPulse = 0;
    unsigned long targetDelay = MIN_CYCLE_LENGTH_US;
    unsigned long currentDelay = START_CYCLE_LENGTH_US;
  public:
    FeedStepper(int enablePin, int dirPin, int pulsePin, bool invertDir = false) {
      this->enablePin = enablePin;
      this->dirPin = dirPin;
      this->pulsePin = pulsePin;
      this->invertDir = invertDir;
    }
    void setup() {
      pinMode(this->enablePin, OUTPUT);
      pinMode(this->dirPin, OUTPUT);
      pinMode(this->pulsePin, OUTPUT);
    }
    void enable() {
      this->enabled = true;
    }
    void disable() {
      this->enabled = false;
    }
    void run() {
      this->running = true;
    }
    void stop() {
      this->running = false;
    }
    void loop() {
      if (this->enabled || this->running) {
        if (this->running) {
          digitalWrite(this->enablePin, LOW);
          writeDirection();
          if (micros() - this->lastPulse >= currentDelay) {
            this->lastPulse = micros();
            digitalWrite(this->pulsePin, HIGH);
            delayMicroseconds(PULSE_LENGTH_US);
            digitalWrite(this->pulsePin, LOW);
          }
        }

      } else {
        digitalWrite(this->enablePin, HIGH);
      }
    }
    void writeDirection() {
      if (this->forward) {
        digitalWrite(this->dirPin, this->invertDir ? HIGH : LOW);
      } else {
        digitalWrite(this->dirPin, this->invertDir ? LOW : HIGH);
      }
    }
};

FeedStepper stepperX = FeedStepper(PIN_X_ENABLE, PIN_X_DIR, PIN_X_PULSE);

void setup() {
  Serial.begin(115200);
  stepperX.setup();
  stepperX.enable();
  stepperX.run();
}

void loop() {
  debug();
  stepperX.loop();
}

unsigned long count = 0;
unsigned long lastDebug = 0;
unsigned long debugIntervalUs = 100000; // 100ms

void debug() {
  if (micros() - lastDebug >= debugIntervalUs) {
    count++;
    lastDebug = micros();
  }
}
