#include <LoRaShield.h>
#include <Servo.h>
#include<Wire.h>

#define SERVOPIN 9       //servo Motor Pin
#define Tx 10            //LoRa
#define Rx 11            //LoRa
#define PUSH 7           //pushButton Pin
#define OPEN 82          //angle of door open
#define CLOSE 180        //angle of door close
#define PIEZO A1         //Connect the sensor to A1

#define MAX_ACCEL_CNT 100
#define DELAY_ACCEL_FUNC 4000
#define ACCEL_THRESHOLD 10
#define MAX_FORCE 1750
#define MIN_FORCE 1600

#define MAX_PIEZO_COUNT 3
#define PIEZO_THRESHOLD 700
#define RESET_PIEZO_THRESHOLD 20

//로라 쉴드, 서보모터,초음파센서 선언부
LoRaShield LoRa(Tx, Rx);
String s,m;
Servo servo;
boolean push_cnt = false;  //서보모터가 돌아가 있는지 여부
boolean asButton = false; //버튼을 통해 문이 열렸는지 확인하는 변수

const int MPU=0x68;  //MPU 6050 의 I2C 기본 주소
const float gravity_earth = 9.80665f;
float AcX,AcY,AcZ, gForce;
int16_t Tmp,GyX,GyY,GyZ, gForce_int, preVal;
int accelFuncCnt = 0;
int diffCount = 0;
int movingCount = 0;
int cnt_initial = 0;
boolean isMoving = false;

//진동감지
int val_piezo;
int pre_piezo;
int piezo_count=0;
int normal_count=0;
boolean piezo_active = true;

void setup() 
{
  //로라 설정
  LoRa.begin(38400);
  Serial.begin(115200); //보드레이트 이격

  pinMode(PUSH, INPUT);
  pinMode(Rx, OUTPUT);
  
  //서보  
  servo.attach(SERVOPIN);
  servo.write(CLOSE);

  //가속도 센서
  Wire.begin();      //Wire 라이브러리 초기화
  Wire.beginTransmission(MPU); //MPU로 데이터 전송 시작
  Wire.write(0x6B);  // PWR_MGMT_1 register
  Wire.write(0);     //MPU-6050 시작 모드로
  Wire.endTransmission(true); 

  Serial.println("Run Success");
}
 
void loop() 
{  
  pushButton();   //푸시버튼, 문 열림상태 감지 함수.
    
  if(accelFuncCnt == DELAY_ACCEL_FUNC && push_cnt == false){
    if(piezo_active)
      checkPiezo();
    doorCheckByAccel();
    accelFuncCnt = 0;
  }else{
    accelFuncCnt++;
  }
 
  //로라 실행구간.
   while (LoRa.available())
   {
    s = LoRa.ReadLine();
    Serial.print("LoRa.ReadLine() = ");
    Serial.println(s);

    m = LoRa.GetMessage();
    if(m != ""){
      Serial.println("Recv from LoRa : " + m);
      if(m == "26"){
        openDoorByLoRa();
      }
     }
   }
}


void pushButton(){
  int i = digitalRead(PUSH);  // 12번 디지털 입력으로 전압을 읽어들임

  if(i==0 && push_cnt==false){     
    servo.write(OPEN);  //모터 90도 회전
    asButton = true;
    push_cnt = true;
  }
  else if(i==1 && push_cnt==true){
    delay(100);     
    servo.write(CLOSE);      
    Serial.println("button pushed");
    push_cnt = false;
    piezo_count=0;
  }
}

void openDoorByLoRa(){
  servo.write(OPEN);
  delay(100);  
  servo.write(CLOSE);
  Serial.println("Open Success");
}

void doorCheckByAccel(){
  Wire.beginTransmission(MPU);    //데이터 전송시작
  Wire.write(0x3B);               // register 0x3B (ACCEL_XOUT_H), 큐에 데이터 기록
  Wire.endTransmission(false);    //연결유지
  Wire.requestFrom(MPU,14,true);  //MPU에 데이터 요청
  //데이터 한 바이트 씩 읽어서 반환
  AcX=Wire.read()<<8|Wire.read();  // 0x3B (ACCEL_XOUT_H) & 0x3C (ACCEL_XOUT_L)    
  AcY=Wire.read()<<8|Wire.read();  // 0x3D (ACCEL_YOUT_H) & 0x3E (ACCEL_YOUT_L)
  AcZ=Wire.read()<<8|Wire.read();  // 0x3F (ACCEL_ZOUT_H) & 0x40 (ACCEL_ZOUT_L)
  Tmp=Wire.read()<<8|Wire.read();  // 0x41 (TEMP_OUT_H) & 0x42 (TEMP_OUT_L)
  GyX=Wire.read()<<8|Wire.read();  // 0x43 (GYRO_XOUT_H) & 0x44 (GYRO_XOUT_L)
  GyY=Wire.read()<<8|Wire.read();  // 0x45 (GYRO_YOUT_H) & 0x46 (GYRO_YOUT_L)
  GyZ=Wire.read()<<8|Wire.read();  // 0x47 (GYRO_ZOUT_H) & 0x48 (GYRO_ZOUT_L)

  AcX = AcX / gravity_earth;
  AcY = AcY / gravity_earth;
  AcZ = AcZ / gravity_earth;
  gForce = AcX* AcX + AcY* AcY + AcZ* AcZ;
  gForce = sqrt(gForce);
  gForce_int = (int16_t)gForce;
  /*
  if(gForce_int != preVal){
    diffCount++;
  }else{
    diffCount=0;
    piezo_active = true;
  }
  preVal = gForce_int;

  if(diffCount > (ACCEL_THRESHOLD/2) && !isMoving){
    piezo_active = false;
  }
  
  if(diffCount > ACCEL_THRESHOLD && !isMoving){
  */
  if((gForce_int > MAX_FORCE || gForce_int < MIN_FORCE) && gForce != 0 && isMoving == false){
    piezo_active = false;
    movingCount++;
    isMoving = true;
    if(asButton){
      Serial.println("Door is open inside");
      LoRa.SendMessage("0",HEX);
    }else{
      Serial.println("Door is opened outside");
      LoRa.SendMessage("1",HEX);
    }
    asButton = false;
  }else if(isMoving){
    cnt_initial++;
  }

  if(cnt_initial == MAX_ACCEL_CNT){
    cnt_initial = 0;
    isMoving = false;
    diffCount = 0;
    piezo_active = true;
    normal_count=0;
    piezo_count=0;
  }
  Serial.print("gForce: ");
  Serial.print(gForce_int);
  Serial.print("          movingCount: ");
  Serial.print(movingCount);
  Serial.print("          cnt_initial: ");
  Serial.println(cnt_initial);
}

void checkPiezo(){    //진동감지가 RESET_PIEZO_THRESHOLD(50) 사이로 연달아 PIEZO_THRESHOLD(3)개가 나타나면 서버에 알린다.
  val_piezo = analogRead(PIEZO);
  if(val_piezo > PIEZO_THRESHOLD){
    piezo_count++;
    normal_count=0;
  }else if(pre_piezo <= PIEZO_THRESHOLD){
    normal_count++; 
  }
  pre_piezo = val_piezo;

  if(normal_count == RESET_PIEZO_THRESHOLD){
    normal_count=0;
    piezo_count=0;
  }
  
  if(piezo_count == MAX_PIEZO_COUNT){ //PIEZO_THRESHOLD 5
    Serial.println("Ring Ring");
    LoRa.SendMessage("2",HEX);
    piezo_count++;
  }
  Serial.print("val_piezo: ");
  Serial.print(val_piezo);
  Serial.print("   piezo_count: ");
  Serial.print(piezo_count);
  Serial.print("   normal_count: ");
  Serial.println(normal_count);
}

