#pragma once
#include <Arduino.h>
#include "config.h"

// PPM Receiver State
volatile uint16_t ppmRawChannels[PPM_CHANNEL_COUNT];
volatile unsigned long lastPpmFrameMicros = 0;
volatile bool ppmSignalDetected = false;

static unsigned long isrLastEdgeMicros = 0;
static uint8_t isrCurrentChannel = 0;

// Hardware Interrupt Service Routine for PPM decoding
void IRAM_ATTR ppmInterruptHandler() {
  unsigned long now = micros();
  unsigned long pulse = now - isrLastEdgeMicros;
  isrLastEdgeMicros = now;

  if (pulse > PPM_SYNC_MIN_US) {
    // Sync pulse detected (> 3000us) -> reset to channel 0
    isrCurrentChannel = 0;
    lastPpmFrameMicros = now;
    ppmSignalDetected = true;
  } else if (pulse >= PPM_PULSE_MIN_US && pulse <= PPM_PULSE_MAX_US) {
    if (isrCurrentChannel < PPM_CHANNEL_COUNT) {
      ppmRawChannels[isrCurrentChannel] = (uint16_t)pulse;
      isrCurrentChannel++;
    }
  } else {
    // Glitch detected -> reset channel pointer
    isrCurrentChannel = PPM_CHANNEL_COUNT;
  }
}

class PpmReceiver {
public:
  void begin(uint8_t pin) {
    for (int i = 0; i < PPM_CHANNEL_COUNT; i++) {
      ppmRawChannels[i] = (i == 2) ? 1000 : 1500; // Throttle at 1000, others 1500
    }
    pinMode(pin, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(pin), ppmInterruptHandler, FALLING);
  }

  // Returns true if a valid PPM frame was received within the last 150ms
  bool isConnected() {
    if (!ppmSignalDetected) return false;
    unsigned long age = (micros() - lastPpmFrameMicros) / 1000;
    return (age < PPM_TIMEOUT_MS);
  }

  // Read channel value (1-based index: 1 to 8)
  uint16_t getChannel(uint8_t ch, uint16_t defaultVal = 1500) {
    if (ch < 1 || ch > PPM_CHANNEL_COUNT) return defaultVal;
    uint16_t val = ppmRawChannels[ch - 1];
    if (val < PPM_PULSE_MIN_US || val > PPM_PULSE_MAX_US) return defaultVal;
    return val;
  }
};
