/*********
  Rui Santos
  Complete project details at http://randomnerdtutorials.com  
*********/

void setup() {
  Serial.begin(500000);
  Serial.print("setup() running on core ");
  Serial.println(xPortGetCoreID());
}

void loop() {
  Serial.print("loop() running on core ");
  Serial.println(xPortGetCoreID());
}
