#include <M5Core2.h>
#include <LovyanGFX.hpp>
#include <LGFX_AUTODETECT.hpp>
#include <HX711.h>
#include <AXP192.h>

static LGFX lcd;                       // LGFXのインスタンスを作成。
static LGFX_Sprite weightSprite(&lcd); // スプライトを使う場合はLGFX_Spriteのインスタンスを作成。
static LGFX_Sprite timeSprite(&lcd);

AXP192 power;

// HX711 related pin Settings.  HX711 相关引脚设置
#define LOADCELL_DOUT_PIN 36
#define LOADCELL_SCK_PIN 26
HX711 scale;

hw_timer_t *timer = NULL;
volatile uint32_t counter = 0;
volatile uint32_t current_time = 0;

int timeSec = 0;
int timeMin = 0;
char timeString[6];


void IRAM_ATTR onTimer()
{
  counter++;
  current_time = millis();
  Serial.printf("timeSec = %d, timeMin = %d, counter = %d \n", timeSec, timeMin, counter);
}

void vibration(){
  power.SetLDOEnable(3, true);
  delay(500);
  power.SetLDOEnable(3,false);
}

void setup()
{
  M5.begin(true, true, true, true); // Init M5Stack.  初始化M5Stack
  lcd.init();
  Serial.begin(115200); // シリアルポートを開き，通信速度を19200bpsに設定

  timer = timerBegin(0, 80, true);
  timerAttachInterrupt(timer, &onTimer, true);
  timerAlarmWrite(timer, 100000, true);

  lcd.setColorDepth(16);

  lcd.fillScreen(TFT_BLACK);
  lcd.setCursor(0, 0);
  lcd.setTextFont(&fonts::Font0);
  lcd.setTextColor(TFT_GREEN);
  lcd.setTextSize(5);

  lcd.drawString("WEIGHT", 0, 0);
  lcd.drawString("TIME", 0, 120);

  weightSprite.createSprite(240, 80);
  timeSprite.createSprite(320, 80);

  weightSprite.setTextColor(TFT_GREEN);
  timeSprite.setTextColor(TFT_GREEN);

  weightSprite.setTextFont(&fonts::Font0);
  timeSprite.setTextFont(&fonts::Font0);

  weightSprite.setTextSize(2);
  weightSprite.drawString("Calibration sensor....", 0, 40);
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  // The scale value is the adc value corresponding to 1g
  scale.set_scale(680.f); // set scale
  scale.tare();           // auto set offset
}

void loop()
{
  int weight = scale.get_units(10);
  // Serial.println(weight);
  weightSprite.setTextSize(5);
  weightSprite.fillSprite(TFT_BLACK);
  weightSprite.setCursor(0, 0);
  weightSprite.printf("%d g", weight);

  timeSec = int(counter * 0.1) % 60;
  timeMin = counter * 0.1 / 60;
  sprintf(timeString, "%02d:%02d", timeMin, timeSec);

  timeSprite.setTextSize(5);
  timeSprite.fillSprite(TFT_BLACK);
  timeSprite.setCursor(0, 0);
  timeSprite.printf(timeString);

  M5.update();
  if (M5.BtnA.wasPressed())
  {
    scale.set_scale(680.f);
    scale.tare();
    weightSprite.fillSprite(TFT_BLACK);
    weightSprite.setTextSize(2);
    weightSprite.drawString("Calibration!", 0, 0);
    vibration();
  }
  else if (M5.BtnB.wasPressed())
  {
    btnCFlg = false;
    counter = 0;
    timerAlarmEnable(timer);
    vibration();
  }
  else if (M5.BtnC.wasPressed())
  {
    if (timerStarted(timer))
    {
      timerStop(timer);
    }
    else
    {
      timerStart(timer);
    }

    vibration();
  }

  weightSprite.pushSprite(0, 40);
  timeSprite.pushSprite(0, 160);
}