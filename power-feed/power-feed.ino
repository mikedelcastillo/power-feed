#define PIN_X_ENABLE 10
#define PIN_X_DIR 11
#define PIN_X_PULSE 12

#define ENCODER_A_BUTTON 2
#define ENCODER_A_DT 3
#define ENCODER_A_CLK 4

#define PULSE_LENGTH_US 10.0 // microseconds
#define MIN_CYCLE_LENGTH_US 800.0 // microseconds
#define SLOW_CYCLE_LENGTH_US 4000.0 // microseconds
#define ACCELERATION_CYCLE_LENGTH_US 250.0 // microseconds

typedef void (*CallbackFunction)();

class FeedEncoder {
  public:
    int dtPin;
    int clkPin;
    int value = 0;

    int clk = 0;
    int dt = 0;

    int increased = false;
    int decreased = false;
    FeedEncoder(int dtPin, int clkPin) {
      this->dtPin = dtPin;
      this->clkPin = clkPin;
    }
    void setup() {
      pinMode(this->dtPin, INPUT);
      pinMode(this->clkPin, INPUT);
    }
    void loop() {
      this->increased = false;
      this->decreased = false;
      int dt = digitalRead(this->dtPin);
      int clk = digitalRead(this->clkPin);
      if (this->clk != clk) {
        if (clk == dt) {
          this->value--;
          this->decreased = true;
        } else {
          this->value++;
          this->increased = true;
        }
        this->clk = clk;
        this->dt = dt;
      }
    }
    void debug() {
    }
};

class FeedStepper {
  public:
    int enablePin;
    int dirPin;
    int pulsePin;
    bool invertDir = false;

    bool enabled = false;
    bool running = false;
    unsigned long lastPulse = 0;
    float targetDelay = MIN_CYCLE_LENGTH_US;
    float currentDelay = SLOW_CYCLE_LENGTH_US;
    unsigned long lastAcceleration = 0;

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
      this->currentDelay = SLOW_CYCLE_LENGTH_US;
      this->targetDelay = SLOW_CYCLE_LENGTH_US;
    }
    void spin(float target) {
      this->targetDelay = target;
    }
    void loop() {
      accelerate();
      pulse();
    }
    void accelerate() {
      if (micros() - this->lastAcceleration >= ACCELERATION_CYCLE_LENGTH_US) {
        this->lastAcceleration = micros();
        float currentSign = this->currentDelay > 0 ? 1.0 : -1.0;
        float targetSign = this->targetDelay > 0 ? 1.0 : -1.0;

        float activeTarget = this->targetDelay;

        if (currentSign != targetSign) {
          if (abs(this->targetDelay) >= SLOW_CYCLE_LENGTH_US) {
            this->currentDelay = this->targetDelay;
          } else {
            activeTarget = currentSign * SLOW_CYCLE_LENGTH_US;
          }
        }

        this->currentDelay += (activeTarget - this->currentDelay) / 500.0;

        this->currentDelay = currentSign * max(abs(this->currentDelay), MIN_CYCLE_LENGTH_US);

        if (abs(this->currentDelay) >= abs(activeTarget) - 100) {
          this->currentDelay = targetSign * abs(this->currentDelay);
        }
      }
    }
    void pulse() {
      if (this->enabled) {
        if (this->running) {
          digitalWrite(this->enablePin, LOW);
          writeDirection();
          if (micros() - this->lastPulse >= round(abs(currentDelay))) {
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
      if (this->currentDelay >= 0) {
        digitalWrite(this->dirPin, this->invertDir ? HIGH : LOW);
      } else {
        digitalWrite(this->dirPin, this->invertDir ? LOW : HIGH);
      }
    }
};

FeedStepper stepperX = FeedStepper(PIN_X_ENABLE, PIN_X_DIR, PIN_X_PULSE);
FeedEncoder encoderA = FeedEncoder(ENCODER_A_DT, ENCODER_A_CLK);

void setup() {
  Serial.begin(115200);
  stepperX.setup();
  stepperX.enable();
  stepperX.run();

  encoderA.setup();
}

void loop() {
  debug();
  stepperX.loop();
  encoderA.loop();

  int sig = encoderA.value >= 0 ? 1.0 : -1.0;
  stepperX.targetDelay = sig * (MIN_CYCLE_LENGTH_US + pow(abs(encoderA.value) / 20.0, 2) * 50.0);

  if (Serial.available() != 0) {
    float targetDelay = Serial.parseFloat();
    if (abs(targetDelay) > MIN_CYCLE_LENGTH_US) {
      Serial.println(targetDelay);
      stepperX.spin(targetDelay);
    }
  }
}

unsigned long count = 0;
unsigned long lastDebug = 0;
unsigned long debugIntervalUs = 100000; // 100ms

void debug() {
  if (micros() - lastDebug >= debugIntervalUs) {
    lastDebug = micros();
    //    count++;
    //    if (count % 50 == 0) {
    //      float sig = (random(0, 2) == 0 ? -1 : 1);
    //      stepperX.targetDelay = 1 * random(3500, SLOW_CYCLE_LENGTH_US);
    //    }
    //    Serial.print(stepperX.targetDelay);
    //    Serial.print("\t");
    Serial.print(stepperX.targetDelay);
    Serial.print("\t");
    Serial.println(stepperX.currentDelay);
    encoderA.debug();
  }
}
