
// пин кнопки
#define btn_pin 2

// пин геркона
#define gerkon_pin 3

// пин баззера
#define buzzer_pin 4

// пин релюшки
#define relay_pin 5

#define _LCD_TYPE 1 //Тип подключения дисплея: 1 - по шине I2C scl - a5, sdl - a4

// адрес памяти
#define addr_current_timer 1

#include <LCD_1602_RUS_ALL.h>
#include <Button.h>
#include <EEPROM.h>

LCD_1602_RUS lcd(0x27, 16, 2);
Button btn(btn_pin);
Button gerkon(gerkon_pin);

byte mode = 2; // флаг переключающий состояния
int timers[6] = {5, 10, 15, 30, 45, 60}; // таймеры в минутах
int current_timer = 0; // текущее положение таймера
int min;
int sec = 0;
bool trigger = true; // триггер для очищения экрана (костыль)
long tick = 0; // таймер задержек
bool blink = false; // флаг мигания

void setup () {

  // просто красивая инициализация экрана
  lcd.init();
  lcd.backlight();
  for (int i = 0; i < 2; i++) {
    for (int j = 0; j < 16; j++) {
      lcd.setCursor(j, i);
      lcd.write(255);
      delay(20);
    }
  }
  lcd.clear();
  delay(50);

  // вход в режим настроек
  if (!digitalRead(btn_pin)){
    mode = 1;
  }

  // инициализация кнопки
  btn.begin();

  // инициализация выводов
  pinMode(buzzer_pin, OUTPUT);
  pinMode(relay_pin, OUTPUT);

}

void loop () {

  switch (mode) { // главное меню - переключение между состояниями
    case 1: setting(); break; // настройка таймера
    case 2: waiting(); break; // ожидание установки капсулы
    case 3: counting(); break; // обратный отсчет
    case 4: cocking(); break; // засвет капсулы
  }
}

void setting () {

  if (trigger) { // обновить информация на экране по флагу
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("УСТАНОВИ  ТАЙМЕР");
    lcd.setCursor(6, 1);
    lcd.print(timers[current_timer], DEC);
    lcd.setCursor(9, 1);
    lcd.print("мин");
    trigger = !trigger;
  }

  if (btn.pressed()) {
    current_timer ++ ;
    if (current_timer > 5) current_timer = 0;
    EEPROM.write(addr_current_timer, current_timer);
    trigger = true;
    Serial.println(current_timer);
  }
}

void waiting () {

  if (trigger) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("УСТАНОВИ КАПСУЛУ");
    lcd.setCursor(2, 1);
    lcd.print("ЧТОБЫ НАЧАТЬ");
    trigger = !trigger;
  }

  if (gerkon.pressed()){ // капсула установлена
    mode = 3;
    min = timers[EEPROM.read(addr_current_timer)];
    sec = 0;
    trigger = true;
    tick = millis();
  } 

}

void counting (){

  if (millis() - tick > 1000){ // обратный отсчет
    tick = millis();
    blink = !blink;
    sec --;
    trigger = true;
    digitalWrite(buzzer_pin, 1);
  }

  if (millis() - tick > 1){ // выключаем звук
    digitalWrite(buzzer_pin, 0);  
  }

  if (sec < 0){
    sec = 59;
    min --;
  }

  if (trigger) { // индикация
    lcd.clear();
    lcd.setCursor(6, 0);
    if (min < 10) lcd.setCursor(7, 0); // смещение для однозначних чисел
    lcd.print(min, DEC);
    if (blink) {
      lcd.setCursor(8, 0);
      lcd.write(58);
    }
    lcd.setCursor(9, 0);
    if (sec < 10) {
      lcd.print(0, DEC);
      lcd.setCursor(10, 0);
    }
    lcd.print(sec, DEC);

    // строка прогресса 
    int val = map(min*100 + sec, 0, timers[EEPROM.read(addr_current_timer)]*100 - 50, 15, 0);

    for (int j = 0; j <= val; j++) {
      lcd.setCursor(j, 1);
      lcd.write(255);
    }

    trigger = !trigger;
  }

  if (min < 0){
    mode = 4;
    trigger = true;
    tick = millis();
  }

  if (gerkon.released()){ // отключение режима
    mode = 2;
    trigger = true;
    digitalWrite(buzzer_pin, 0);
  } 
}

void cocking (){

  if (trigger) {
    lcd.clear();
    lcd.setCursor(4, 0);
    lcd.print("КАПСУЛА");
    lcd.setCursor(4, 1);
    lcd.print("АКТИВНА");
    trigger = !trigger;
  }

  if (millis() - tick < 3000) { // звуковая индикация и засветка
    digitalWrite(buzzer_pin, 1);
    digitalWrite(relay_pin, 1);
  }
  else {
    digitalWrite(buzzer_pin, 0);
    digitalWrite(relay_pin, 0);
  }

  if (gerkon.released()){ // отключение режима
    mode = 2;
    trigger = true;
    digitalWrite(buzzer_pin, 0);
    digitalWrite(relay_pin, 0);
  } 
}