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

#define pushButton 8  // Pushbutton Pin
#define servoPin 4 // servo Motor Pin
#define TriggerPin 5 // Trig Pin
#define EchoPin 6 // Echo Pin
#define PiezoPin A1 // Piezo vibrator Pin

long Duration = 0;
float distance = 0;
boolean pushCnt = false; // 서보모터가 돌아가 있는지 여부
int cnt = 0;  //  문열림 상태체크용 카운트
int bcnt_1 = 0; // 진동감지 상태체크용 카운트
int bcnt_2 = 0;
boolean doorVal = false; // 문의 열림/닫힘상태
int angle = 0; // servo potision in degrees

void setup() {
  // put your setup code here, to run once:
  BTSerial.begin(9600);
  Serial.begin(115200);
  LoRa1.begin(38400);
  pinMode(pushButton, INPUT);
  //pinMode(led, OUTPUT);
  servo.attach(servoPin);
  servo.write(0);
}

void loop() {

  Pushing();
  DoorOpenState();
  Emergency();
  
  while (LoRa1.available())
    {
      String s = LoRa1.ReadLine();
      //Serial.print("LoRa.ReadLine() = ");
      //Serial.println(s);
 
      String m = LoRa1.GetMessage();
      Serial.print("LoRa.GetMessage() = ");
      Serial.println(m);

      if(m == "26") {
        //digitalWrite(led, HIGH);
        servo.write(60);
        delay(100);
        servo.write(0);
      }
      else if(m == "260011") {
        //digitalWrite(led, LOW);
    }
    }
}

void Pushing()
{
  int i = digitalRead(pushButton);  // 디지털입력 8번핀
  
  if(!i){             // i 상태 == 1
    servo.write(60);  //모터 60도 회전
    pushCnt = true;   //눌린상태 체크
  }
  else {
    servo.write(0);
    if(pushCnt)
    {
      //LoRa1.SendMessage("Click", HEX);  //눌렀다 뗐을 때 로라전송
      Serial.println("Click!");
      pushCnt = false;
    }
  }
}

void DoorOpenState()
{
  digitalWrite(TriggerPin, HIGH); // Trigger pin to HIGH
  delayMicroseconds(10); // 10us high
  digitalWrite(TriggerPin, LOW); // Trigger pin to HIGH
 
  // Duration = pulseIn(EchoPin, HIGH); // Waits for the echo pin to get high
  // returns the Duration in microseconds

  distance = ((float)(340*Duration)/10000)/2; // Use function to calculate the distance
  
  if(distance > 500 && distance < 30000) 
  {
    cnt++;
    //delay(100);
  }
  else
  {
    cnt--;
    //delay(100);
  }
  if(cnt > 10)
  {  
    doorVal = true;
    cnt = 11;
  }
  else if (cnt == 0)
    doorVal = false;
  if (cnt < 0)
    cnt = 0; 
  
  
}

void Emergency()
{
  int val;
  val = analogRead(PiezoPin);
  if(val > 1000)
      bcnt_1++;
  else
      bcnt_2++;

  if(bcnt_2 == 150)
  {
      bcnt_1 = 0;
      bcnt_2 = 0;
  }
  if(bcnt_1 > 300)
  {
      Serial.println("!!!");
      bcnt_1 = 0;
  }
}

