#include <FastLED.h>
#include "BluetoothSerial.h"
BluetoothSerial SerialBT;

#define LED_PIN 4
#define NUM_LEDS 5
#define STEPS 14
#define BRIGHTNESS 127
#define LED_TYPE WS2811
#define COLOR_ORDER GRB
#define A 25
#define B 26

CRGB leds[NUM_LEDS * STEPS];

// Motion detection state
enum MotionState { NONE, ENTERED_A, ENTERED_B };
volatile MotionState motionState = NONE;
volatile bool motionDetected_A = false;
volatile bool motionDetected_B = false;
volatile bool direction = false;
volatile bool on_steps = false;
volatile bool timeoutFlag = false;

TimerHandle_t motionTimer;

void IRAM_ATTR readPIR_A() {
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;

  if (motionState == NONE) {
    motionState = ENTERED_A;
    motionDetected_A = true;
  } else if (motionState == ENTERED_B) {
    motionDetected_A = true;
  }

  if (xTimerIsTimerActive(motionTimer) == pdFALSE) {
    xTimerStartFromISR(motionTimer, &xHigherPriorityTaskWoken);
  } else {
    xTimerResetFromISR(motionTimer, &xHigherPriorityTaskWoken);
  }

  portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void IRAM_ATTR readPIR_B() {
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;

  if (motionState == NONE) {
    motionState = ENTERED_B;
    motionDetected_B = true;
  } else if (motionState == ENTERED_A) {
    motionDetected_B = true;
  }

  if (xTimerIsTimerActive(motionTimer) == pdFALSE) {
    xTimerStartFromISR(motionTimer, &xHigherPriorityTaskWoken);
  } else {
    xTimerResetFromISR(motionTimer, &xHigherPriorityTaskWoken);
  }

  portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void BluetoothTask(void *pvParameters) {
  while (1) {
    if (SerialBT.available()) {
      char received = SerialBT.read();

      if (received == 'A') {
        readPIR_A(); // simulate interrupt
      }
      else if (received == 'B') {
        readPIR_B();
      }
    }
    vTaskDelay(10 / portTICK_PERIOD_MS);
  }
}


void motionTimeoutCallback(TimerHandle_t xTimer) {
  timeoutFlag = true;
}

void LEDTask(void *pvParameters) {
  while (1) {
        if (!on_steps) {
      if (motionState == ENTERED_A && motionDetected_A) {
        direction = true;
        on_steps = true;
        motionDetected_A = false;
        FastLED.setBrightness(BRIGHTNESS);

        for (int i = 0; i < STEPS; i++) {
          for (int j = i * NUM_LEDS; j < (i + 1) * NUM_LEDS; j++) {
            leds[j] = CRGB(255, 255, 255);
          }
          FastLED.show();

          if (i < 5) {
            vTaskDelay(100 / portTICK_PERIOD_MS);
          } else {
            vTaskDelay(500 / portTICK_PERIOD_MS);
          }
        }
      }

      else if (motionState == ENTERED_B && motionDetected_B) {
        direction = false;
        on_steps = true;
        motionDetected_B = false;
        FastLED.setBrightness(BRIGHTNESS);

        for (int i = STEPS; i > 0; i--) {
          for (int j = i * NUM_LEDS - 1; j >= (i - 1) * NUM_LEDS; j--) {
            leds[j] = CRGB(255, 255, 255);
          }
          FastLED.show();

          if (i > STEPS - 5) {  // last 4 steps in reverse
            vTaskDelay(100 / portTICK_PERIOD_MS);
          } else {
            vTaskDelay(500 / portTICK_PERIOD_MS);
          }
        }
      }
    }

    // Turn off: second sensor confirms direction
    if (on_steps) {
       if (direction && motionState == ENTERED_A && motionDetected_B) {
        for (int i = 0; i < STEPS; i++) {
          for (int j = i * NUM_LEDS; j < (i + 1) * NUM_LEDS; j++) {
            leds[j] = CRGB(0, 0, 0);
          }
          FastLED.show();
          vTaskDelay(500 / portTICK_PERIOD_MS);
        }

        motionDetected_A = motionDetected_B = false;
        on_steps = false;
        timeoutFlag = false;
        motionState = NONE;
        xTimerStop(motionTimer, 0);
      }

      else if (!direction && motionState == ENTERED_B && motionDetected_A)  {
        for (int i = STEPS; i > 0; i--) {
          for (int j = i * NUM_LEDS - 1; j >= (i - 1) * NUM_LEDS; j--) {
            leds[j] = CRGB(0, 0, 0);
          }
          FastLED.show();
          vTaskDelay(500 / portTICK_PERIOD_MS);
        }

        motionDetected_A = motionDetected_B = false;
        on_steps = false;
        timeoutFlag = false;
        motionState = NONE;
        xTimerStop(motionTimer, 0);
      }
    }

    // Timeout case
    if (timeoutFlag && on_steps) {
  for (int i = 0; i < 32; i++) { // more fade steps
    fadeToBlackBy(leds, NUM_LEDS * STEPS, 10); // slower fade
    FastLED.show();
    vTaskDelay(50 / portTICK_PERIOD_MS);
  }

  // force to fully off
  fill_solid(leds, NUM_LEDS * STEPS, CRGB::Black);
  FastLED.show();

  motionDetected_A = motionDetected_B = false;
  on_steps = false;
  motionState = NONE;
  timeoutFlag = false;
  xTimerStop(motionTimer, 0);
}

  vTaskDelay(1 / portTICK_PERIOD_MS);
  }
}

void setup() {
  SerialBT.begin("LEDController"); // Bluetooth device name
  Serial.println("Bluetooth Server Ready. Waiting for connection...");

  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS * STEPS);
  FastLED.setBrightness(BRIGHTNESS);
  xTaskCreatePinnedToCore(BluetoothTask, "Bluetooth Task", 4096, NULL, 2, NULL, 1);  // priority 2

  motionTimer = xTimerCreate("MotionTimer", pdMS_TO_TICKS(15000), pdFALSE, (void *)0, motionTimeoutCallback);

  xTaskCreatePinnedToCore(LEDTask, "LED Task", 10000, NULL, 3, NULL, 0); // priority 3

   attachInterrupt(digitalPinToInterrupt(B), readPIR_A, FALLING);

}

void loop() {}
