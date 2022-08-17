#include <Arduino.h>

#define MOSFET_Pin PB5
#define Bat_Pin PA0
#define Shunt_Pin PA5

const unsigned long Vsup = 3310; //Опорное напряжение Arduino в мВ, относительно него мерим напряжение батареи
unsigned long mA = 0;
unsigned long Bat_Volt = 0;
unsigned long Bat_Volt_Prev = 0;
unsigned long Shunt_Volt = 0;
float Bat_Source = 0.0;
float Shunt_Source = 0.0;
unsigned long time = 0;
unsigned long time_Prev = 0;
const float R1 = 48.3;
const float R2 = 20.38;
const float k = R2 / (R1 + R2);
const float Rshunt = 1.0/4.0 + 0.073; //Сопротивление токоизмерительного резистора в цепи истока + сопротивление открытого канала коммутирующего мосфета
unsigned long Capacity = 0;
float EnergyHour = 0;
int data_start = 0;

void setup()
{
    SerialUSB.begin(115200);
    pinMode(MOSFET_Pin, OUTPUT);
    digitalWrite(MOSFET_Pin, LOW);
    //pinMode(Bat_Pin, INPUT_ANALOG);
    //pinMode(Shunt_Pin, INPUT_ANALOG);
    Serial.dtr(false); //Нужно, чтобы отображались UART-данные по USB
    while (!Serial);
    analogReadResolution(12);
}

void loop()
{
    for (int i = 0; i < 150; i++) { //Накапливаем измерения за 2мс*100 и делим на количество отсчетов: 150, т.е. усредняем
        Bat_Source = Bat_Source + analogRead(Bat_Pin);
        delay(2);
    }
    Bat_Source = Bat_Source / 150.0;
    Bat_Volt = map(round(Bat_Source), 0, 4095, 0, round(Vsup / k)); //Переводим данные из формата АЦП в мВ

    for (int i = 0; i < 150; i++) { //Накапливаем измерения за 2мс*100 и делим на количество отсчетов: 150, т.е. усредняем
        Shunt_Source = Shunt_Source + analogRead(Shunt_Pin);
        delay(2);
    }
    Shunt_Source = Shunt_Source / 150.0;
    Shunt_Volt = map(round(Shunt_Source), 0, 4095, 0, Vsup); //Переводим данные из формата АЦП в мВ
    
    time = millis();
    Capacity = (Shunt_Volt / Rshunt) * (time / 1000 / 60.0 / 60.0);
    EnergyHour += 0.5 * (Bat_Volt + Bat_Volt_Prev) / 1000 * (Shunt_Volt / Rshunt) * (time - time_Prev); //Интегрируем мгновенную мощность по времени методом трапеций
    int Energy = round (EnergyHour / 1000 / 60.0 / 60.0);
    time_Prev = time;
    Bat_Volt_Prev = Bat_Volt;

    if (Bat_Volt > 2500) {
      digitalWrite(MOSFET_Pin, HIGH);
    }
    else {
      digitalWrite(MOSFET_Pin, LOW);
      SerialUSB.println("Voltage < 2.5 Volt, Measure end");
      SerialUSB.print(Capacity); Serial.println(" mA⋅h");
      Serial.print(Energy); Serial.println(" mW⋅h");
      while(1);
    }
    SerialUSB.print(Bat_Volt); Serial.print(" ");
    Serial.print(int(round(Shunt_Volt / Rshunt))); Serial.print(" ");
    SerialUSB.print(time / 1000 / 60.0); Serial.print(" ");
    SerialUSB.print(Capacity); Serial.print(" ");
    Serial.print(Energy);
    SerialUSB.println(";");
    SerialUSB.print("$");

    delay(100);
    // Serial.print("Voltage: " + Bat_Volt);                    Serial.println(" V");
    // Serial.print("Shunt: " + round(Shunt_Volt / Rshunt));    Serial.println(" mA");
    // Serial.print("Time: " + time / 1000);                    Serial.println(" sec");
    // Serial.print("Capacity: " + Capacity);                   Serial.println(" mA⋅h");
    // Serial.print("Watt Hour: " + EnergyHour);                Serial.println(" W⋅h");
    // Serial.println("");

}