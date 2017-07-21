#include <LoRaShield.h>
#include <Servo.h>
#include <SoftwareSerial.h>

#define BlueRxD 3           //Bluetooth RX
#define BlueTxD 2           //Bluetooth TX
#define SERVOPIN 5          //servo Motor Pin
#define TriggerPin 8        //Trig Pin
#define EchoPin 9           //Echo Pin
#define Tx 10               //LoRa
#define Rx 11               //LoRa
#define PUSH 12             //pushButton Pin
#define OPEN 60          //angle of door open
#define CLOSE 0          //angle of door close

LoRaShield LoRa(Tx, Rx);
SoftwareSerial BTSerial(BlueRxD,BlueTxD); //(RX, TX)
Servo servo;

String s,m;              //LoRa.ReadLine(), GetMessage()
byte data;
long duration_val = 0;
boolean push_cnt = false;  //서보모터가 돌아가 있는지 여부
int cnt = 0;               //초음파 센서 알고리즘
boolean door_val = false; //문의 상태 TRUE:OPEN FALSE:CLOSE
boolean asButton = false; //버튼을 통해 문이 열렸는지 확인하는 변수

void setup() {
  BTSerial.begin(115200);
  Serial.begin(115200);
  LoRa.begin(38400);
  pinMode(PUSH, INPUT);
  servo.attach(SERVOPIN);
  servo.write(CLOSE);
  initializeBoardSetting();
}

void loop() {

  pushButton();
  DoorOpenState();
  
  while (LoRa.available())
  {
    s = LoRa.ReadLine();
    //Serial.print("LoRa.ReadLine() = ");
    //Serial.println(s);

    m = LoRa.GetMessage();
    if(m != ""){
    Serial.println("Recv from LoRa : " + m);
      if(m == "22"){    //command that open the door by LoRa
        openDoorByLoRa();
      }
    }
  }
}

void initializeBoardSetting(){
  
}

void pushButton()
{
  int i = digitalRead(PUSH);  // A5 아날로그 입력으로 전압을 읽어들임
  delay(100);
  if(!i){     // i 상태 == 1
    servo.write(OPEN);  //모터 60도 회전
    asButton = true;
    push_cnt = true;
  }
  else {
    servo.write(CLOSE);
    if(push_cnt)
    {
      LoRa.SendMessage("Going Out", HEX);
      Serial.println("Button Clicked");
      push_cnt = false;
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
 
  //duration_val = pulseIn(EchoPin, HIGH); // Waits for the echo pin to get high
  //returns the Duration in microseconds

  long Distance_mm = Distance(duration_val); // Use function to calculate the distance
  
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
    door_val = true;
    cnt = 11;
  }
  else if (cnt == 0)
    door_val = false;
  if (cnt < 0)
    cnt = 0; */
  
  
}
long Distance(long _time)
{
  // Calculates the Distance in mm
  // ((time)*(Speed of sound))/ toward and backward of object) * 10

  long DistanceCalc; // Calculation variable
  DistanceCalc = ((_time / 2.9) / 2); // Actual calculation in mm
  //DistanceCalc = time / 74 / 2; // Actual calculation in inches
  return DistanceCalc; // return calculated value
}

void openDoorByLoRa(void){
  //if(Closed){
  servo.write(OPEN);
  delay(100);
  servo.write(CLOSE);
  asButton = true;
  Serial.println("Open Success");
  LoRa.SendMessage("Open Success",HEX);
  return true;
  //}
  //if(Opened){
  //Serial.println("Open Fail");
  //LoRa.SendMessage("Open Fail",HEX);
  // return false;}
}
