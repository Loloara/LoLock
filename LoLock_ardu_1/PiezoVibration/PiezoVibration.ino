int piezoPin = A1; // Connect the sensor to A1

void setup() {
  Serial.begin(115200);
}

void loop() {
    int val;
    val = analogRead(piezoPin); 
    Serial.println(val, DEC);
    delay(100);
}
