#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
 #include <avr/power.h> // Required for 16 MHz Adafruit Trinket
#endif

#define PI 3.14159265
#define PIN        3 // On Trinket or Gemma, suggest changing this to 1
#define NUMPIXELS 64 // Popular NeoPixel ring size
#define BALL_NUM 3

struct Color {
  float r;
  float g;
  float b;
};

struct Ball {
  float v = 0;
  float pos = 0;
  float m = 1;
  Color color = {0, 0, 0};
};

Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);
float body_size = 4;  // 体の大きさ
float g = 0;  // 重力加速度
float k = 1000;  // バネ定数
float c = 7;
struct Ball balls[BALL_NUM];
float time = 0;  // 時間
float last_time = 0;  // 前回の時間
float floor_pos = 0;  // 床の位置
float ceil_pos = NUMPIXELS;  // 天井の位置
int bound_cnt = 0;  // バウンド回数

void setup() {
#if defined(__AVR_ATtiny85__) && (F_CPU == 16000000)
  clock_prescale_set(clock_div_1);
#endif

  Serial.begin(9600);
  randomSeed(analogRead(0));
  pixels.begin(); // INITIALIZE NeoPixel strip object (REQUIRED)
  time = millis();
  initSimulation();
}

void loop() {
  pixels.clear(); // Set all pixel colors to 'off'

  time = (float)millis() / 1000;

  float seq_time = 15;  // シーケンス全体の時間
  // %使いたいがためにint化
  if(long(time * 1000) % long(seq_time * 1000) < long(5 * 1000)) {
    g = -9.8 * 30;
  } else if(long(time * 1000) % long(seq_time * 1000) < long(5.2 * 1000)) {
    g = 9.8 * 30;
  } else if(long(time * 1000) % long(seq_time * 1000) < long(5.4 * 1000)) {
    g = -9.8 * 20;
  } else {
    float power = 0;
    for(int j = 0; j < BALL_NUM; j++) {
      float pos = balls[j].pos;
      float dist_from_center_nomralized = (pos - (ceil_pos / 2)) / (ceil_pos / 2);
      power += dist_from_center_nomralized * (sin((float)j + time) + 1) * .5;
    }
    g = 9.8 * 6 * -power;
    g += (sin(time * .9) + cos(time * .95)) * 9.8 * 3;  // 収束しないようにsinで適当に揺らし続ける
  }

  simulate();

  for(int i=0; i<NUMPIXELS; i++) { // For each pixel...
    Color color = {0, 0, 0};
    for(int j = 0; j < BALL_NUM; j++) {
      float pos = balls[j].pos;
      float dist = abs(pos - (float)i);

      float fade_max = body_size / 2;
      float fade_min = body_size / 2 + 1;
      float a_max = 1;
      float a_min = 0;
      float a = ((a_max - a_min) / (fade_max - fade_min)) * dist + (fade_max * a_min - fade_min * a_max) / (fade_max - fade_min);
      a = clamp(a, a_min, a_max);
      if(a > 0) {
        Color c = balls[j].color;
        color.r += c.r * a;
        color.g += c.g * a;
        color.b += c.b * a;
      }
    }
    color.r = clamp(color.r, 0, 255);
    color.g = clamp(color.g, 0, 255);
    color.b = clamp(color.b, 0, 255);
    if(color.r > 0 || color.g > 0 || color.b > 0) pixels.setPixelColor(i, pixels.Color(color.r, color.g, color.b));
  }
  pixels.show();   // Send the updated pixel colors to the hardware.
  last_time = time;
}

void initSimulation() {
  float seed = (float)random(360) / 180 * (float)PI;
  for(int i = 0; i < BALL_NUM; i++) {
    // ずらして並べる
    balls[i].pos = (float)NUMPIXELS - body_size * (float)(i + 1) * 3 - 1;
    balls[i].color = getRandomColor((float)i + seed);
  }
  floor_pos = 0;
  g = -9.8 * 30;
}

void simulate() {
  float dt = time - last_time;
  for(int i = 0; i < BALL_NUM; i++) {
    Ball me = balls[i];
    float m = me.m;
    float v = me.v;
    float pos = me.pos;
    float f = m * g;  // F = ma
    float r = body_size / 2;

    // 地面や天井に沈み込んだときのF
    if (pos < r + floor_pos) {
      f += k * (r - (pos - floor_pos)) - c * v;
    } else if (pos > - r + ceil_pos) {
      f += k * (- r - (pos - ceil_pos)) - c * v;
    }

    // 他のボールと重なっている時のF
    for(int j = 0; j < BALL_NUM; j++) {
      if(i == j) continue;
      Ball other = balls[j];
      float dist = abs(me.pos - other.pos);
      if(dist < r * 2) {
        float pow_dir = me.pos > other.pos ? 1 : -1;
        f += k * (r * 2 - dist) * pow_dir - c * v * pow_dir;
      }
    }

    // 運動方程式F=ma
    float a = f / m;
    v += a * dt;
    v *= .99;  // 空気抵抗
    balls[i].v = v;
  }
  for(int i = 0; i < BALL_NUM; i++) {
    balls[i].pos += balls[i].v * dt;
  }
}

Color getRandomColor(float seed) {
    return {
      (sin(seed) + 1) * .5 * 255,
      (sin(seed + .4) + 1) * .5 * 255,
      (sin(seed + .8) + 1) * .5 * 255
    };
}

// --------------------- 
// Utility Methods -----
// ---------------------

float sign(float value) {
  if(value < 0) return -1;
  else return 1;
}

float clamp(float value, float minValue, float maxValue) {
  return max(min(value, maxValue), minValue);
}