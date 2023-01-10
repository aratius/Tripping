#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
 #include <avr/power.h> // Required for 16 MHz Adafruit Trinket
#endif

#define PIN        3 // On Trinket or Gemma, suggest changing this to 1
#define NUMPIXELS 64 // Popular NeoPixel ring size
#define BALL_NUM 3

struct Ball {
  float v;
  float pos;  
};

Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);
float body_size = 3;  // 体の大きさ
float g = 0;  // 重力加速度
float k = 1000;  // バネ定数
float c = 10;
struct Ball balls[BALL_NUM] = {{0, 0}, {0, 0}, {0, 0}};
float time = 0;  // 時間
float last_time = 0;  // 前回の時間
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
  if(long(time * 1000) % long(seq_time * 1000) < long(5 * 1000)) {
    g = -9.8 * 30;
  } else if(long(time * 1000) % long(seq_time * 1000) < long(5.2 * 1000)) {
    g = 9.8 * 30;
  } else if(long(time * 1000) % long(seq_time * 1000) < long(5.4 * 1000)) {
    g = -9.8 * 20;
  } else {
    float pos = balls[0].pos;
    float dist_from_center_nomralized = (pos - (ceil_pos / 2)) / (ceil_pos / 2);
    g = 9.8 * 6 * -dist_from_center_nomralized;
    g += (sin(time * .9) + cos(time * .95)) * 9.8 * 2;  // 収束しないようにsinで適当に揺らし続ける
  }

  simulate();

  for(int i=0; i<NUMPIXELS; i++) { // For each pixel...
    float color[3] = {0, 0, 0};
    for(int j = 0; j < BALL_NUM; j++) {
      float pos = balls[j].pos;
      float dist = abs(pos - (float)i);

      float fade_max = body_size/2;
      float fade_min = body_size/2 + 2;
      float a_max = 255;
      float a_min = 0;
      float a = ((a_max - a_min) / (fade_max - fade_min)) * dist + (fade_max * a_min - fade_min * a_max) / (fade_max - fade_min);
      a = max(min(a, a_max), a_min);
      color[j % 3] += a;
    }
    color[0] = max(min(color[0], 255), 0);
    color[1] = max(min(color[1], 255), 0);
    color[2] = max(min(color[2], 255), 0);
    if(color[0] > 0 || color[1] > 0 || color[2] > 0) pixels.setPixelColor(i, pixels.Color(color[0], color[1], color[2]));

  }
  pixels.show();   // Send the updated pixel colors to the hardware.
  last_time = time;
  delay(10); // Pause before next pass through loop
}

void initSimulation() {
  for(int i = 0; i < BALL_NUM; i++) {
    balls[i].pos = (float)NUMPIXELS - body_size * (float)(i + 1) * 3 - 1;
  }
  floor_pos = 0;
  g = -9.8 * 30;
}

void simulate() {
  float delta_time = time - last_time;
  for(int i = 0; i < BALL_NUM; i++) {
    Ball me = balls[i];
    float f = g;
    float r = body_size/2;
    float v = me.v;
    float pos = me.pos;

    // 地面や天井に沈み込んだときの演算
    if (pos < r + floor_pos) {
      f += k * (r - (pos - floor_pos)) - c * v;
    } else if (pos > - r + ceil_pos) {
      f += k * (- r - (pos - ceil_pos)) - c * v;
    }

    for(int j = 0; j < BALL_NUM; j++) {
      if(i == j) continue;
      Ball other = balls[j];
      float dist = abs(me.pos - other.pos);
      if(dist < r * 2) {
        float pow_dir = me.pos > other.pos ? 1 : -1;
        f += k * (r * 2 - dist) * pow_dir - c * v * pow_dir;
      }
    }

    float a = equation_of_motion(1, f);
    v += a * delta_time;
    v *= .99;  // 空気抵抗
    balls[i].v = v;
  }
  for(int i = 0; i < BALL_NUM; i++) {
    balls[i].pos += balls[i].v * delta_time;
  }
}

void reverse() {
  g *= -1;
}
 
float sign(float value) {
  if(value < 0) return -1;
  else return 1;
}
