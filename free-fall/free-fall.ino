#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
 #include <avr/power.h> // Required for 16 MHz Adafruit Trinket
#endif

#define PIN        3 // On Trinket or Gemma, suggest changing this to 1
#define NUMPIXELS 64 // Popular NeoPixel ring size

Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);
float body_size = 2;  // 体の大きさ
float g = -9.8 * 1;  // 重力加速度
float v0 = 0;  // 初速
float time = 0;  // 時間
float last_time = 0;  // 前回の時間
float simulation_time_started = 0;  // 演算のための
float pos = 0;  // 現在の座標
float floor_pos = 0;  // 床の位置
float ceil_pos = NUMPIXELS;  // 天井の位置
int bound_cnt = 0;  // バウンド回数

float free_fall(float _v0, float _g, float _t) {
  return _v0 + _g * _t;
}

void setup() {
#if defined(__AVR_ATtiny85__) && (F_CPU == 16000000)
  clock_prescale_set(clock_div_1);
#endif

  pixels.begin(); // INITIALIZE NeoPixel strip object (REQUIRED)

  time = millis();
  initSimulation();
}

void loop() {
  pixels.clear(); // Set all pixel colors to 'off'

  time = millis();
  simulate();

  for(int i=0; i<NUMPIXELS; i++) { // For each pixel...
    if(
      (float)i < ceil(pos + body_size / 2) &&
      (float)i > floor(pos - body_size / 2)
    ) pixels.setPixelColor(i, pixels.Color(255, 255, 255));
  }
  pixels.show();   // Send the updated pixel colors to the hardware.
  last_time = time;
  delay(1); // Pause before next pass through loop
}

void initSimulation() {
  simulation_time_started = time;
  pos = (float)NUMPIXELS;
  floor_pos = 0;
  g = -9.8 * 1;
}

void simulate() {
  float v = free_fall(v0, g, (time - simulation_time_started) / 1000);
  pos += v;
  if(
    (v <= 0 && pos - body_size / 2 < floor_pos) ||
    (v > 0 && pos + body_size / 2 > ceil_pos)
  ) bound(v);
}

void bound(float v) {
  v0 = -v * .8;
  simulation_time_started = time;
  pos = v < 0 ? floor_pos + body_size / 2 : ceil_pos - body_size / 2;
  bound_cnt++;
}

void reverse() {
  g *= -1;
}
 
float sign(float v) {
  if(v < 0) return -1;
  else return 1;
}