#include <mpu6050.h>
#include <Wire.h>
#include <Adafruit_NeoPixel.h>

// PINS
int pin = 11;
int button = 12;
uint8_t neo_pin = 6;

uint8_t neo_pixel_cnt = 18;


Adafruit_NeoPixel strip = Adafruit_NeoPixel(neo_pixel_cnt, neo_pin, NEO_GRB + NEO_KHZ800);

boolean saber = false;

int lred = 255;
int lgreen = 0;
int lblue = 0;

int slred = 255;
int slgreen = 200;
int slblue = 0;

int dred = 127;
int dgreen = 127;
int dblue = 127;
int daccz = 127;

int total = 127;
int foc_trigger = 48;
int debounce = 200;

void setup()
{
  int error;
  uint8_t c;
  pinMode(pin, OUTPUT);
  pinMode(neo_pin, INPUT);
  pinMode(button, INPUT_PULLUP);

  randomSeed(analogRead(0));

  strip.begin();
  Wire.begin();

  error = MPU6050_read (MPU6050_WHO_AM_I, &c, 1);
  error = MPU6050_read (MPU6050_PWR_MGMT_2, &c, 1);
  MPU6050_write_reg (MPU6050_PWR_MGMT_1, 0);
}

void loop()
{
  getAccData();

  if (digitalRead(button) == LOW)
  {
    saber = !saber;
    if (saber)
    {
      // debounce
      delay(debounce);
      start_sequence();
    }
    else
    {
      end_sequence();
      // debounce
      delay(debounce);
    }
  }

  if (saber)
  {
    if (daccz < foc_trigger)
    {
      flash_on_clash();
    }
    else
    {
      // saber_on();
      saber_on_unstable();
    }
  }
  else
  {
    saber_off();
  }
}

void flash_on_clash()
{
  tone(pin, 100);
  for (int i = 0; i < strip.numPixels(); i++)
  {
    strip.setPixelColor(i, 255, 255, 255);
  }
  strip.show();
  delay(100);
}

void start_sequence()
{
  for (int i = 0; i < strip.numPixels()/3; i++)
  {
    strip.setPixelColor(i, lred, lgreen, lblue);
    strip.setPixelColor(i+6, lred, lgreen, lblue);
    strip.setPixelColor(i+12, lred, lgreen, lblue);
    strip.show();
    tone(pin, 48 * i);
    delay(20);
  }
}

void saber_on()
{
  tone(pin, 50 + abs(19 * (total - 127) / 127));
  for (int i = 0; i < strip.numPixels(); i++)
  {
    strip.setPixelColor(i, lred, lgreen, lblue);
  }
  strip.show();
}

void saber_on_unstable()
{
  int r = random(neo_pixel_cnt);
  tone(pin, 50 + abs((19-r) * (total - 127) / 127));

  for (int i = 0; i < strip.numPixels(); i++)
  {
    if (r == i)
    {
      strip.setPixelColor(i, slred, slgreen, slblue);
    }
    else
    {
      strip.setPixelColor(i, lred, lgreen, lblue);
    }
  }
  strip.show();
}

void saber_off()
{
  noTone(11);
  for (int i = 0; i < strip.numPixels(); i++)
  {
    strip.setPixelColor(i, 0, 0, 0);
  }
  strip.show();
}

void end_sequence()
{
  saber_on(); // fix for unstable
  for (int i = 0; i < strip.numPixels()/3; i++)
  {
    strip.setPixelColor(5 - i, 0, 0, 0);
    strip.setPixelColor(5 - i + 6, 0, 0, 0);
    strip.setPixelColor(5 - i + 12, 0, 0, 0);
    strip.show();
    tone(pin, 48 * (5 - i));
    delay(20);
  }
  noTone(11);
}

void getAccData()
{

  int error;
  double dT;
  accel_t_gyro_union accel_t_gyro;

  error = MPU6050_read (MPU6050_ACCEL_XOUT_H, (uint8_t *) &accel_t_gyro, sizeof(accel_t_gyro));

  uint8_t swap;

#define SWAP(x,y) swap = x; x = y; y = swap

  SWAP (accel_t_gyro.reg.x_accel_h, accel_t_gyro.reg.x_accel_l);
  SWAP (accel_t_gyro.reg.y_accel_h, accel_t_gyro.reg.y_accel_l);
  SWAP (accel_t_gyro.reg.z_accel_h, accel_t_gyro.reg.z_accel_l);
  SWAP (accel_t_gyro.reg.t_h, accel_t_gyro.reg.t_l);
  SWAP (accel_t_gyro.reg.x_gyro_h, accel_t_gyro.reg.x_gyro_l);
  SWAP (accel_t_gyro.reg.y_gyro_h, accel_t_gyro.reg.y_gyro_l);
  SWAP (accel_t_gyro.reg.z_gyro_h, accel_t_gyro.reg.z_gyro_l);

  dred = map(accel_t_gyro.value.x_gyro, -32768, 32767, 0, 255);
  dgreen = map(accel_t_gyro.value.y_gyro, -32768, 32767, 0, 255);
  dblue = map(accel_t_gyro.value.z_gyro, -32768, 32767, 0, 255);
  daccz = map(accel_t_gyro.value.z_accel, -32768, 32767, 0, 255);

  total = (dred + dgreen + dblue) / 3;
}
