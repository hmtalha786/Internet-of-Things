#include <PZEM004Tv30.h>
#include <HardwareSerial.h>

//HardwareSerial MySerial(0);

PZEM004Tv30 pzem(&Serial1, 14, 15);

void setup() {
  Serial.begin(9600);
  Serial1.begin(9600, SERIAL_8N1, 14, 15);
//  MySerial.begin(9600, SERIAL_8N1, 32, 33);
}

void loop() {

  float voltage = pzem.voltage();
  if (voltage != NAN) {
    Serial.print("Voltage: "); Serial.print(voltage); Serial.println("V");
  } else {
    Serial.println("Error reading voltage");
  }

  float current = pzem.current();
  if (current != NAN) {
    Serial.print("Current: "); Serial.print(current); Serial.println("A");
  } else {
    Serial.println("Error reading current");
  }

  float power = pzem.power();
  if (current != NAN) {
    Serial.print("Power: "); Serial.print(power); Serial.println("W");
  } else {
    Serial.println("Error reading power");
  }

  float energy = pzem.energy();
  if (current != NAN) {
    Serial.print("Energy: "); Serial.print(energy, 3); Serial.println("kWh");
  } else {
    Serial.println("Error reading energy");
  }

  float frequency = pzem.frequency();
  if (current != NAN) {
    Serial.print("Frequency: "); Serial.print(frequency, 1); Serial.println("Hz");
  } else {
    Serial.println("Error reading frequency");
  }

  float pf = pzem.pf();
  if (current != NAN) {
    Serial.print("PF: "); Serial.println(pf);
  } else {
    Serial.println("Error reading power factor");
  }

  Serial.println();
  delay(1500);
}
