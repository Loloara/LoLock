/*
    푸쉬버튼 아날로그입력 -> 디지털입력으로 변경
    (INPUT상태가 더 안정화됨)
*/
#include <LoRaShield.h>
#include <Servo.h>
#include <SoftwareSerial.h>

LoRaShield LoRa1(10, 11);
SoftwareSerial BTSerial(0,1); //(RX, TX)
Servo servo;

int led = 13;
int pushButton = 12;  // Pushbutton Pin
int servoPin = 5; // servo Motor Pin
int TriggerPin = 8; // Trig Pin
int EchoPin = 9; // Echo Pin
long Duration = 0;
boolean pushCnt = false; // 서보모터가 돌아가 있는지 여부
int cnt = 0;
boolean doorVal = false;
int angle = 0; // servo potision in degrees
void setup() {
  // put your setup code here, to run once:
  BTSerial.begin(9600);
  Serial.begin(115200);
  LoRa1.begin(38400);
  pinMode(pushButton, INPUT);
  pinMode(led, OUTPUT);
  servo.attach(servoPin);
  servo.write(0);
}

void loop() {

  PushButton();
  DoorOpenState();
  
  while (LoRa1.available())
    {
      String s = LoRa1.ReadLine();
      Serial.print("LoRa.ReadLine() = ");
      Serial.println(s);
 
      String m = LoRa1.GetMessage();
      Serial.print("LoRa.GetMessage() = ");
      Serial.println(m);

      if(m == "260010") {
        digitalWrite(led, HIGH);
        servo.write(60);
        delay(100);
        servo.write(0);
      }
      else if(m == "260011")
        digitalWrite(led, LOW);
    }
}

void PushButton()
{
  int i = digitalRead(pushButton);  // A5 아날로그 입력으로 전압을 읽어들임
  delay(100);
  if(!i){     // i 상태 == 1
    servo.write(60);  //모터 60도 회전
    pushCnt = true;
  }
  else {
    servo.write(0);
    if(pushCnt)
    {
      LoRa1.SendMessage("Click", HEX);
      Serial.println("Click!");
      pushCnt = false;
    }
  }
}

void DoorOpenState()
{
  digitalWrite(TriggerPin, LOW);
  delayMicroseconds(2);
  digitalWrite(TriggerPin, HIGH); // Trigger pin to HIGH
  delayMicroseconds(10); // 10us high
  digitalWrite(TriggerPin, LOW); // Trigger pin to HIGH
 
  //Duration = pulseIn(EchoPin, HIGH); // Waits for the echo pin to get high
  // returns the Duration in microseconds

  long Distance_mm = Distance(Duration); // Use function to calculate the distance
  
  /*if(Distance_mm > 500 && Distance_mm < 30000) 
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
    doorVal = false;
  if (cnt < 0)
    cnt = 0; */
  
  
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

