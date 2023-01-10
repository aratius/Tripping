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

  float seq_time = 15;  // シーケンス全体の時間
  // %使いたいがためにint化
  if(int(time * 1000) % int(seq_time * 1000) < int(5 * 1000)) {
    g = -9.8 * 30;
  } else if(int(time * 1000) % int(seq_time * 1000) < int(5.2 * 1000)) {
    g = 9.8 * 30;
  } else if(int(time * 1000) % int(seq_time * 1000) < int(5.4 * 1000)) {
    g = -9.8 * 20;
  } else {
    float dist_from_center_nomralized = (pos - (ceil_pos / 2)) / (ceil_pos / 2);
    g = 9.8 * 6 * -dist_from_center_nomralized;
    g += (sin(time * .9) + cos(time * .95)) * 9.8 * 2;  // 収束しないようにsinで適当に揺らし続ける
  }

  simulate();

  for(int i=0; i<NUMPIXELS; i++) { // For each pixel...
    float color[3] = {0, 0, 0};
    float dist = abs(pos - (float)i);
    
    float fade_max = body_size/2;
    float fade_min = body_size/2 + 2;
    float a_max = 255;
    float a_min = 0;
    float a = ((a_max - a_min) / (fade_max - fade_min)) * dist + (fade_max * a_min - fade_min * a_max) / (fade_max - fade_min);
    a = max(min(a, a_max), a_min);
    color[0] = a;
    color[1] = a;
    color[2] = a;
    if(color[0] > 0 || color[1] > 0 || color[2] > 0) pixels.setPixelColor(i, pixels.Color(color[0], color[1], color[2]));

  }
  pixels.show();   // Send the updated pixel colors to the hardware.
  last_time = time;
  delay(10); // Pause before next pass through loop
}

void initSimulation() {
  pos = (float)NUMPIXELS - body_size - 1;
  floor_pos = 0;
  g = -9.8 * 30;
}

void simulate() {
  float delta_time = time - last_time;
  float f = g;
  float r = body_size/2;

  // 地面や天井に沈み込んだときの演算
  if (pos < r + floor_pos) {
    f += k * (r - (pos - floor_pos)) - c * v;
  } else if (pos > - r + ceil_pos) {
    f += k * (- r - (pos - ceil_pos)) - c * v;
  }

  float a = equation_of_motion(1, f);
  v += a * delta_time;
  v *= .99;  // 空気抵抗
  pos += v * delta_time;
}

void reverse() {
  g *= -1;
}
 
float sign(float v) {
  if(v < 0) return -1;
  else return 1;
}
