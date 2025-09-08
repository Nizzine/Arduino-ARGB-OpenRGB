#define FASTLED_ALLOW_INTERRUPTS 1 // Prevents freezing of small microcontrollers after a while
#include <FastLED.h>

// === Pin definitions ===
#define PIN1 2
#define PIN2 3
#define PIN3 4
#define PIN4 5

// === LED definitions ===
#define NUM_LEDS1 10
#define NUM_LEDS2 10
#define NUM_LEDS3 5
#define NUM_LEDS4 5
#define NUM_TOTAL (NUM_LEDS1 + NUM_LEDS2 + NUM_LEDS3 + NUM_LEDS4)

#define BAUDRATE 115200
#define COLOR_ORDER RGB

// Packet size = RGB data for all LEDs + 3 bytes (header/checksum)
#define PACKET_SZ   ((NUM_TOTAL * 3) + 3)

// === LED arrays ===
CRGB leds1[NUM_LEDS1];
CRGB leds2[NUM_LEDS2];
CRGB leds3[NUM_LEDS3];
CRGB leds4[NUM_LEDS4];

// === Serial buffer ===
uint8_t serial_buffer[PACKET_SZ];
uint16_t head = 0;
uint16_t start;
uint16_t checksum_1;
uint16_t checksum_0;

void setup() {
  FastLED.addLeds<WS2812B, PIN1, COLOR_ORDER>(leds1, NUM_LEDS1);
  FastLED.addLeds<WS2812B, PIN2, COLOR_ORDER>(leds2, NUM_LEDS2);
  FastLED.addLeds<WS2812B, PIN3, COLOR_ORDER>(leds3, NUM_LEDS3);
  FastLED.addLeds<WS2812B, PIN4, COLOR_ORDER>(leds4, NUM_LEDS4);

  FastLED.clear();
  FastLED.show();

  Serial.begin(BAUDRATE);
}

// === Copy serial buffer into LED arrays ===
void updateLEDs() {
  int idx = start + 1;  // data starts right after 0xAA
  for (int i = 0; i < NUM_TOTAL; i++) {
    int pos = idx + (3 * i);
    if (pos >= (PACKET_SZ - 2)) pos -= PACKET_SZ;  // wrap around

    uint8_t g = serial_buffer[pos];
    uint8_t r = serial_buffer[pos + 1];
    uint8_t b = serial_buffer[pos + 2];

    if (i < NUM_LEDS1) {
      leds1[i].setRGB(r, g, b);
    } else if (i < NUM_LEDS1 + NUM_LEDS2) {
      leds2[i - NUM_LEDS1].setRGB(r, g, b);
    } else if (i < NUM_LEDS1 + NUM_LEDS2 + NUM_LEDS3) {
      leds3[i - NUM_LEDS1 - NUM_LEDS2].setRGB(r, g, b);
    } else {
      leds4[i - NUM_LEDS1 - NUM_LEDS2 - NUM_LEDS3].setRGB(r, g, b);
    }
  }
  FastLED.show();
}

void loop() {
  if (Serial.available()) {
    serial_buffer[head] = Serial.read();

    if (head >= (PACKET_SZ - 1)) {
      start = 0;
      checksum_1 = head;
      checksum_0 = head - 1;
      head = 0;
    } else {
      start = head + 1;
      checksum_1 = head;
      checksum_0 = (head == 0) ? (PACKET_SZ - 1) : (head - 1);
      head++;
    }

    if (serial_buffer[start] == 0xAA) {
      unsigned short sum = 0;
      for (int i = 0; i < checksum_0; i++) sum += serial_buffer[i];
      if (start > 0) {
        for (int i = start; i < PACKET_SZ; i++) sum += serial_buffer[i];
      }

      if ((((unsigned short)serial_buffer[checksum_0] << 8) | serial_buffer[checksum_1]) == sum) {
        updateLEDs();
      }
    }
  }
}
