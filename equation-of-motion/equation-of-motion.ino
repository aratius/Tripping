#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
 #include <avr/power.h> // Required for 16 MHz Adafruit Trinket
#endif

#define PIN        3 // On Trinket or Gemma, suggest changing this to 1
#define NUMPIXELS 32 // Popular NeoPixel ring size

Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);
float body_size = 4;  // 体の大きさ
float g = 0;  // 重力加速度
float k = 1000;  // バネ定数
float c = 5;
float v = 0;
float time = 0;  // 時間
float last_time = 0;  // 前回の時間
float pos = 0;  // 現在の座標
float floor_pos = 0;  // 床の位置
float ceil_pos = NUMPIXELS;  // 天井の位置
int bound_cnt = 0;  // バウンド回数

float equation_of_motion(float m, float f){
  return f / m;
}

void setup() {
#if defined(__AVR_ATtiny85__) && (F_CPU == 16000000)
  clock_prescale_set(clock_div_1);
#endif

  Serial.begin(9600);

  pixels.begin(); // INITIALIZE NeoPixel strip object (REQUIRED)

  time = millis();
  initSimulation();
}

void loop() {
  pixels.clear(); // Set all pixel colors to 'off'

  time = (float)millis() / 1000.;
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
  pos = (float)NUMPIXELS - body_size - 1;
  floor_pos = 0;
  g = -9.8 * 10;
}

void simulate() {
  float delta_time = time - last_time;
  float f = g;
  float r = body_size/2;

  if (pos < r + floor_pos) {
    f += k * (r - (pos - floor_pos)) - c * v;
  } else if (pos > - r + ceil_pos) {
    f += k * (- r - (pos - ceil_pos)) - c * v;
  }

  float a = equation_of_motion(1, f);
  v += a * delta_time;
  pos += v * delta_time;
}

void reverse() {
  g *= -1;
}
 
float sign(float v) {
  if(v < 0) return -1;
  else return 1;
}
