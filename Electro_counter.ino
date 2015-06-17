/* 
Перед загрузкой данной программы, неоходимо обнулмть eeprom, т.е. записать во все ячейки eeprom 0.
*/
#include <EEPROM.h>

volatile unsigned long counter = 746 ;  // Начальные показания
volatile unsigned long newcounter = 0;
unsigned long counter_max = 70000;  //Максимальное значения которое хранится в одном банке памяти
#define pin1 2   // 2-й цифровой пин, нулевое прерывание Электросчетчика
int counter_byte = 0; //Ячейка eeprom для хранения данных Электросчетчика
int adress_byte = 128; //Адрес хранения адреса хранения данных электросчетчика))
int adress = 0;
int impuls = 0;
unsigned long power = 0;
unsigned long old_power = 0;

unsigned long cur_tm = millis();
unsigned long pre_tm = cur_tm;
unsigned int tm_diff = 0; 
 
unsigned long cur_tmS = millis();
unsigned long pre_tmS = cur_tmS;
float diffS = 0;
float tm_diffS = 0;  

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
  void setup() {
    Serial.begin(115200);
    delay(2);
            if (EEPROM.read(150)==0){ // Если первый запуск то запишем в ячейки адреса где хранятся данные
               if (counter >= counter_max && counter<=(counter_max*2)-1){
                  counter_byte = 4; 
               }
               if (counter>=counter_max*2 && counter<=(counter_max*3)-1){
                  counter_byte = 8; 
               }
               if (counter>=counter_max*3 && counter<=(counter_max*4)-1){
                  counter_byte = 12; 
               }         
               EEPROM.write(adress_byte, counter_byte);
               EEPROMWriteInt(counter_byte, counter);  //Пишем показания счетчика в eeprom из переменной
               EEPROM.write(150, 1); //Бит первого запуска
             }
     counter_byte = EEPROM.read(adress_byte);
     counter = EEPROMReadInt(counter_byte);
                   
    //Настраиваем цифровой вход
    pinMode(13, OUTPUT);
    pinMode(2, INPUT);      // Сюда будем подключать подтягивающий резюк
    digitalWrite(2, HIGH);  // "Подключаем" подтягивающий резистор
    attachInterrupt(0, count_1, FALLING); // задаём обработчик прерывания 0 (2-й пин). 
                                        // прерывание будет при изменении уровня с HIGHT на LOW
                                        // вызывать функицю count
  }
///////////////////////////////////ОСНОВНОЙ ЦИКЛ/////////////////////////////////////////////  
void loop(){
          if (Serial.available() > 0) {
             if (Serial.find("Status")) { //Ищем на входе символ "C" Если нашли отправляем данные 
                 for (int i=0; i <= 1; i++){
                     Serial.print("C");
                     Serial.print(counter); // выводим в консоль значение счётчика
                     Serial.print("W");
                     Serial.print(power); // выводим в консоль значение счётчика
                     Serial.print("$");
                 }
             }
              if (Serial.find("CHG")) { //Ищем на входе строку "CHG" Если нашли получаем данные 
                 unsigned long change_counter = Serial.parseInt(); // Парсинг числа во входящем потоке
                 counter = change_counter;
                   if (counter > 0){
                      if (counter >= counter_max && counter<=(counter_max*2)-1){
                        counter_byte = 4; 
                      }
                      if (counter>=counter_max*2 && counter<=(counter_max*3)-1){
                        counter_byte = 8; 
                      }
                      if (counter>=counter_max*3 && counter<=(counter_max*4)-1){
                        counter_byte = 12; 
                      }
                      EEPROM.write(adress_byte, counter_byte);
                      EEPROMWriteInt(counter_byte, counter);  //Пишем показания счетчика в eeprom из переменной
                    }  
                  }
           }
               if (counter != newcounter){  
                  detachInterrupt (0);
                  for (int i=0; i <= 1; i++){
                        Serial.print("C");
                        Serial.print(counter); // выводим в консоль значение счётчика
                        Serial.print("W");
                        Serial.print(power); // выводим в консоль значение счётчика
                        Serial.print("$");
                        newcounter = counter;
                      }
                  attachInterrupt(0, count_1, FALLING);  
                  }
        if (impuls >= 3200){ 
          detachInterrupt (0);
            impuls = 0;
            counter = counter + 1;
              if (counter>=counter_max && counter<=(counter_max*2)-1){
                counter_byte = counter_byte + 4; 
                EEPROM.write(adress_byte, counter_byte);
              }
              if (counter>=counter_max*2 && counter<=(counter_max*3)-1){
                counter_byte = counter_byte + 8; 
                EEPROM.write(adress_byte, counter_byte);
              }
              if (counter>=counter_max*3 && counter<=(counter_max*4)-1){
                counter_byte = counter_byte + 12; 
                EEPROM.write(adress_byte, counter_byte);
              } 
                EEPROMWriteInt(counter_byte, counter); //Пишем данные счетчика в энергонезависимую память 
       attachInterrupt(0, count_1, FALLING);    
        }
}
//////////////////////////////////ФУНКЦИИ СЧЕТЧИКА//////////////////////////////////////
void count_1(){
    detachInterrupt (0);
  // вызывается прерыванием 0 от 2-го цифрового входа
     unsigned long millis_prev;
        if(millis()-10 > millis_prev) { //Защита от дребезга контаков
             pre_tmS = cur_tmS;
             cur_tmS = millis();
               if( cur_tmS > pre_tmS ) {
                 tm_diffS = cur_tmS - pre_tmS;
               }
             impuls++;  // Инкриментируем счетчик импульсов    

             diffS = (tm_diffS / 1000);
             power = 3600000 / (3200 * diffS);

         if (power > (old_power + 4000)){
            power = old_power;
         }
             Serial.print("C");
             Serial.print(counter);
             Serial.print("W");
             Serial.print(power);
             Serial.print("$");
             old_power = power;
             millis_prev = millis();
           }
            attachInterrupt(0, count_1, FALLING);
}
////////////////////////////////////ФУНКЦИИ ПАМЯТИ///////////////////////////////////////////
//кушаем аж 4 байта EEPROM
void EEPROMWriteInt(int p_address, unsigned long p_value){
    byte four = (p_value & 0xFF);
    byte three = ((p_value >> 8) & 0xFF);
    byte two = ((p_value >> 16) & 0xFF);
    byte one = ((p_value >> 24) & 0xFF);
    EEPROM.write(p_address, four);
    EEPROM.write(p_address + 1, three);
    EEPROM.write(p_address + 2, two);
    EEPROM.write(p_address + 3, one);
}
// считаем нашы 4 байта
unsigned long EEPROMReadInt(int p_address){
   long four = EEPROM.read(p_address);
   long three = EEPROM.read(p_address + 1);
   long two = EEPROM.read(p_address + 2);
   long one = EEPROM.read(p_address + 3);
   return ((four << 0) & 0xFF) + ((three << 8) & 0xFFFF) + ((two << 16) & 0xFFFFFF) + ((one << 24) & 0xFFFFFFFF);
 }
