#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
 #include <avr/power.h> // Required for 16 MHz Adafruit Trinket
#endif

#define PIN        3 // On Trinket or Gemma, suggest changing this to 1
#define NUMPIXELS 64 // Popular NeoPixel ring size

Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);
const float body_size = 2;
float g = -9.8 * 1;
float v0 = 0;
float t = 0;
float t_started = 0;
float pos = 0;
float floor_pos = 0;
float ceil_pos = NUMPIXELS;
int bound_cnt = 0;

float free_fall(float _v0, float _g, float _t) {
  return _v0 + _g * _t;
}

void setup() {
#if defined(__AVR_ATtiny85__) && (F_CPU == 16000000)
  clock_prescale_set(clock_div_1);
#endif

  pixels.begin(); // INITIALIZE NeoPixel strip object (REQUIRED)

  initSimulation();
}

void loop() {
  pixels.clear(); // Set all pixel colors to 'off'

  simulate();

  for(int i=0; i<NUMPIXELS; i++) { // For each pixel...
    if(
      (float)i < ceil(pos + body_size / 2) &&
      (float)i > floor(pos - body_size / 2)
    ) pixels.setPixelColor(
      i, 
      pixels.Color(
        ((float)i / (float)NUMPIXELS) * 255, 
        10, 
        (1 - (float)i / (float)NUMPIXELS) * 255
      )
    );
  }
  pixels.show();   // Send the updated pixel colors to the hardware.
  delay(10); // Pause before next pass through loop
}

void initSimulation() {
  t = millis();
  t_started = t;
  pos = (float)NUMPIXELS;
  floor_pos = 0;
  g = -9.8 * 1;
}

void simulate() {
  t = millis();
  float v = free_fall(v0, g, (t - t_started) / 1000);
  pos += v;
  if(
    (v <= 0 && pos - body_size / 2 < floor_pos) ||
    (v > 0 && pos + body_size / 2 > ceil_pos)
  ) bound(v);
}

void bound(float v) {
  v0 = -v * .8;
  t_started = t;
  pos = v < 0 ? floor_pos + body_size / 2 : ceil_pos - body_size / 2;
  bound_cnt++;
  
  if(bound_cnt % 15 == 0) reverse();
}

void reverse() {
  t_started = t;
  g *= -1;
}
 
float sign(float v) {
  if(v < 0) return -1;
  else return 1;
}