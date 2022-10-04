#include <LiquidCrystal.h>
#include <LiquidCrystal_I2C.h>
#include <Stepper.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);
bool is_day;
bool is_locked;

const byte lock_left[8] = {
  B00000,
  B00000,
  B00011,
  B00111,
  B01110,
  B01100,
  B01100,
  B01100
};
const byte lock_right[8] = {
  B00000,
  B00000,
  B11000,
  B11100,
  B01110,
  B00110,
  B00110,
  B00110
};
// const byte lock_mid = B10100000;
const byte lock_mid = B10011111;
const byte lock_body = B11111111;
const byte sun_nw[8] = {
  B00000,
  B00000,
  B00000,
  B00000,
  B00011,
  B00111,
  B01111,
  B01111
};
const byte sun_sw[8] = {
  B01111,
  B01111,
  B00111,
  B00011,
  B00000,
  B00000,
  B00000,
  B00000
};
const byte sun_ne[8] = {
  B00000,
  B00000,
  B00000,
  B00000,
  B11000,
  B11100,
  B11110,
  B11110
};
const byte sun_se[8] = {
  B11110,
  B11110,
  B11100,
  B11000,
  B00000,
  B00000,
  B00000,
  B00000
};
const byte moon_nw[8] = {
  B00000,
  B10000,
  B00010,
  B00000,
  B00011,
  B00111,
  B01111,
  B01111
};
const byte moon_ne1[8] = {
  B01000,
  B00000,
  B00000,
  B00001,
  B11000,
  B11100,
  B00100,
  B00000
};
const byte moon_ne2[8] = {
  B00000,
  B00001,
  B00000,
  B00000,
  B11000,
  B11100,
  B00100,
  B00000
};
const byte moon_se[8] = {
  B00000,
  B00100,
  B11100,
  B11000,
  B00000,
  B00000,
  B00000,
  B00000
};

String text1 = "Light: ";
String text2 = "Gabii na. ";
String text3 = "Buntag na.";
String text4 = "Door is ";
String text5 = "locked.  ";
String text6 = "unlocked.";

const int max = 1023;
Stepper st(100, 4, 5, 6, 7);
const int steps = 600;
volatile int live_pagenum = 3;
int pagenum = live_pagenum;
static bool frame = false;
unsigned long global_timestamp = millis()+200;

void setup() {
  pinMode(2, INPUT); //interrupt pin
  attachInterrupt(digitalPinToInterrupt(2), navigate, FALLING);
  pinMode(3, OUTPUT); //beep

  lcd.init();
  lcd.backlight();
  lcd.noAutoscroll();

  st.setSpeed(200);

  is_day = analogRead(3) > 600 ? true : false;

  Serial.begin(9600);

  // greet("AT");
  navigate();
}

void loop() {
  if(pagenum!=live_pagenum){
    lcd.clear();
    is_day=analogRead(3)<=600;
    if (live_pagenum==0) {
      lcd.setCursor(0, 0);
      lcd.createChar(0, sun_nw);
      lcd.createChar(1, sun_sw);
      lcd.createChar(2, sun_ne);
      lcd.createChar(3, sun_se);
      lcd.createChar(4, moon_nw);
      lcd.createChar(5, moon_ne1);
      lcd.createChar(6, moon_ne2);
      lcd.createChar(7, moon_se);
      if (is_day){
        lcd.write(4);
        lcd.write(5);
      }
      else {
        lcd.write(0);
        lcd.write(2);
      }
      lcd.setCursor(0,1);
      lcd.write(1);
      lcd.write(is_day?7:3);
      lcd.setCursor(3,0);
      lcd.print(text1);
      lcd.setCursor(3,1);
      lcd.print(is_day?text3:text2);

      is_day=!is_day;
    }
    else if (live_pagenum==1) {
      lcd.createChar(0, lock_left);
      lcd.createChar(1, lock_right);
      lcd.setCursor(0,0);
      lcd.write(0);
      lcd.write(1);
      lcd.setCursor(0,1);
      lcd.write(lock_body);
      lcd.write(lock_body);
      lcd.setCursor(3, 0);
      lcd.print(text4);
      lcd.setCursor(7, 1);
      lcd.print(text5);
    }
    else if (live_pagenum==2) {
      lcd.setCursor(0,0);
      lcd.print("SMS offline");
      lcd.setCursor(0,1);
      lcd.print("Nyss pinakagwapa sa tanan");
    }
    else if (live_pagenum==3) {
      lcd.setCursor(0,0);
      lcd.print("SD card");
      lcd.setCursor(0,1);
      lcd.print("0 Kb used.");
    }
    
    pagenum=live_pagenum;
  }
  // Serial.println(millis());
  // Serial.println(pagenum); 
  if (pagenum==0) {
    display_light();
  }
  else if (pagenum==1) {
    display_doorlock();
  }
  else if (pagenum==2) {
    display_sms();
  }
  else if (pagenum==3) {
    display_memory();
  }    
}

void greet(String message) {
  Serial.print(message + "\r");
  while (!Serial.available()) {}
  Serial.println(Serial.readString());
};

//steps is either 600 or -600
void toggle_lock() {
  st.step(is_locked ? steps : -steps);
  is_locked = !is_locked;
  analogWrite(3,123);
  delay(500);
  analogWrite(3,255);
  delay(500);
  digitalWrite(3,0);
}

void navigate() {
  if(millis()>global_timestamp){
  global_timestamp=millis()+200;
  live_pagenum=++live_pagenum%4;
  }
}

void display_light() {
  // Serial.println(millis());
  String light(float(max-analogRead(3))*100/max);
  light += '%';
  int len = 7 - light.length();
  for (; len != 0; len--) light += ' ';
  lcd.setCursor(10, 0);
  lcd.print(light);

  if (analogRead(3) > 600) {
    lcd.setCursor(0,0);
    if(frame==false){
      lcd.write(4);
      lcd.write(5);
      frame=true;
    }
    else{
      lcd.write(0);
      lcd.write(6);
      frame=false;
    }

    if (is_day) {
      lcd.setCursor(1,1);
      lcd.write(7);

      lcd.setCursor(3,1);
      lcd.print(text2);
      if (!is_locked) toggle_lock();
      is_day = !is_day;
    }
  }
  else {
    if (!is_day) {
      lcd.setCursor(0,0);
      lcd.write(0);
      lcd.write(2);
      lcd.setCursor(1,1);
      lcd.write(3);

      lcd.setCursor(3,1);
      lcd.print(text3);
      is_day = !is_day;
    }
  }

  long unsigned t = millis()+500;
  while(millis() < t){
    if(pagenum!=live_pagenum) return;
  }
}

void display_doorlock() {
}

void display_sms() {
}

void display_memory() {
}