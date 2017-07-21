#include <SoftwareSerial.h>
#include <LoRaShield.h>
#include <Servo.h>

LoRaShield LoRa1(10, 11);
SoftwareSerial BTSerial(0,1); // (TX, RX)
Servo servo;

int pushButton = A5; // push Button pin
const int servoPin = 5; // 서보모터 pin
const int TriggerPin = 8; //Trig pin
const int EchoPin = 9; //Echo pin
long Duration = 0;
int cnt = 0;
boolean doorVal = false;
int led = 13;
int angle = 0; // servo potision in degrees

void setup() {
  pinMode(TriggerPin, OUTPUT); // Trigger is an output pin
  pinMode(EchoPin, INPUT); // Echo is an input pin
  Serial.begin(9600); // Serial Output
  BTSerial.begin(9600); // Bluetooth Output
  LoRa1.begin(38400);
  pinMode(led, OUTPUT);
  servo.attach(servoPin);
  servo.write(0);
}

void loop() {

  int i = analogRead(5); // A5 아날로그 입력으로부터 전압을 읽어옴
  if(i > 1000) {  //전압값이 1000보다 크면 (약 4.88V)
    servo.write(60);  //모터 60도 회전
    digitalWrite(led, HIGH); // 내장 LED ON
  }
  else if(i < 1000) {
    servo.write(0);
    digitalWrite(led, LOW);
  }
  while (LoRa1.available())
  {
    String s = LoRa1.ReadLine();
    Serial.print("LoRa.ReadLine() = ");
    Serial.println(s);
 
    String m = LoRa1.GetMessage();
    Serial.print("LoRa.GetMessage() = ");
    Serial.println(m);

    if(m == "260100")
      digitalWrite(led, HIGH);
    else if(m == "260101")
      digitalWrite(led, LOW);
  }


  
  digitalWrite(TriggerPin, LOW);
  delayMicroseconds(2);
  digitalWrite(TriggerPin, HIGH); // Trigger pin to HIGH
  delayMicroseconds(10); // 10us high
  digitalWrite(TriggerPin, LOW); // Trigger pin to HIGH

  Duration = pulseIn(EchoPin, HIGH); // Waits for the echo pin to get high
  // returns the Duration in microseconds

  long Distance_mm = Distance(Duration); // Use function to calculate the distance
  
  if(Distance_mm > 500 && Distance_mm < 30000) 
  {
    cnt++;
    delay(100);
  }
  else
  {
    cnt--;
    delay(100);
  }
  if(cnt > 10)
  {
    doorVal = true;
    cnt = 11;
  }
  else if (cnt == 0)
  {
    doorVal = false;
  }
  if (cnt < 0)
  {
    cnt = 0;
  }
  /*if(doorVal)
    BTSerial.begin(9600);
  else
    BTSerial.end();
  Serial.print("Distance = "); // Output to serial
  Serial.print(Distance_mm);
  Serial.println(" mm");
  if(doorVal)
    Serial.println("Door Open!");
  else
    Serial.println("Door Closed");
  */
  delay(100); // Wait to do next measurement
}

long Distance(long time)
{
  // Calculates the Distance in mm
  // ((time)*(Speed of sound))/ toward and backward of object) * 10

  long DistanceCalc; // Calculation variable
  DistanceCalc = ((time / 2.9) / 2); // Actual calculation in mm
  //DistanceCalc = time / 74 / 2; // Actual calculation in inches
  return DistanceCalc; // return calculated value
}
