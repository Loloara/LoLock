#include <LoRaShield.h>
#include <Servo.h>
#include<Wire.h>

#define SERVOPIN 9       //servo Motor Pin
#define Tx 10            //LoRa
#define Rx 11            //LoRa
#define PUSH 7           //pushButton Pin
#define OPEN 80          //angle of door open
#define CLOSE 180        //angle of door close

#define MAX_ACCEL_CNT 100 //when the moving is detected, stop function for a while
#define DELAY_ACCEL_FUNC 4000
#define MAX_FORCE 1750    //max force value of normal range
#define MIN_FORCE 1600    //min force value of normal range

LoRaShield LoRa(Tx, Rx);//Rx, Tx (define inverse pin)
String s,m;     //String for LoRa.ReadLine(), LoRa.GetMessage()
Servo servo;
boolean push_cnt = false;  //is pushing?
boolean asButton = false; //was the button pressed before the door opened?

const int MPU=0x68;  //I2C base address of MPU 6050
const float gravity_earth = 9.80665f;
float AcX,AcY,AcZ, gForce;
int16_t Tmp,GyX,GyY,GyZ, gForce_int, preVal;
int accelFuncCnt = 0;

int movingCount = 0;
int cnt_initial = 0;
boolean isMoving = false;
int moving_interval_x = 0;
int moving_interval_y = 0;
int piezo_count=0;

void setup() 
{
  LoRa.begin(38400);    //set LoRa
  Serial.begin(115200); //board rate

  pinMode(PUSH, INPUT); //set button
  pinMode(Rx, OUTPUT);  //set LoRa output
  
  //서보  
  servo.attach(SERVOPIN);  //set servo
  servo.write(CLOSE);   //init servo

  //가속도 센서
  Wire.begin();      //init Wire library
  Wire.beginTransmission(MPU); //start transmit data to MPU
  Wire.write(0x6B);  // PWR_MGMT_1 register
  Wire.write(0);     //mode to MPU-6050
  Wire.endTransmission(true); 

  Serial.println("setup complete");
}
 
void loop() 
{  
  pushButton();   //detect the button is pressing
    
  if(accelFuncCnt == DELAY_ACCEL_FUNC && push_cnt == false){    //delay 4000 cnt && no button pressed
    doorCheckByAccel();   //check door state[opened inside(0), opened outside(1), kicked(2)]
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
        openDoorByLoRa(); //activate servo for opening the door by LoRa
      }
     }
   }
}


void pushButton(){
  int i = digitalRead(PUSH);  // read digital pin

  if(i==0 && push_cnt==false){     
    servo.write(OPEN);
    asButton = true;      //true <- opened inside, false <- opened outside
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
  delay(300);  
  servo.write(CLOSE);
  Serial.println("Open Success");
}

void doorCheckByAccel(){
  Wire.beginTransmission(MPU);    //start transmit data
  Wire.write(0x3B);               //register 0x3B (ACCEL_XOUT_H), record data to queue
  Wire.endTransmission(false);    //keep connecting
  Wire.requestFrom(MPU,14,true);  //request data to MPU
  
  //read eight bytes of data one byte at a time
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

  if(isMoving){         //detected door moving
    cnt_initial++;      //stop calculating movingCount for a while
    if(cnt_initial % 10 == 0 && cnt_initial != 100 || cnt_initial == 1){  //print log for waiting
      Serial.print(MAX_ACCEL_CNT/10 - cnt_initial/10);
      Serial.println(" sec to Ready");
    }
  }else if((gForce_int > MAX_FORCE || gForce_int < MIN_FORCE) && gForce != 0){    //Over Threshold
    movingCount++;
    if(movingCount==3){
      if(moving_interval_x + moving_interval_y < 2){  //short interval vibes
        piezo_count++;
        if(piezo_count == 3){
          Serial.println("Kick the door");
          LoRa.SendMessage("2",HEX);
          
          isMoving=true;          
          movingCount=0;
          asButton=false;
          piezo_count=0;          
          moving_interval_x=0;
          moving_interval_y=0;
          cnt_initial=0;
        }
      }else{                                      //long interval vibes
        if(asButton){
          Serial.println("Door is open inside");
          LoRa.SendMessage("0",HEX);
        }else{
          Serial.println("Door is opened outside");
          LoRa.SendMessage("1",HEX);
        }
        
        isMoving = true;        
        movingCount = 0;
        asButton = false;
        piezo_count=0;
        moving_interval_x=0;
        moving_interval_y=0;
        cnt_initial=0;
      }
    }
  }else if(movingCount>0){    //gForce_int is out of threshold range and movingCount is 1 or 2
    if(movingCount==1)
      moving_interval_x++;
    else if(movingCount==2)
      moving_interval_y++;
      
    if(moving_interval_x == 10 || moving_interval_y == 10){ //over interval
      moving_interval_x=0;
      moving_interval_y=0;
      piezo_count=0;
      movingCount=0;
    }
  }else if(movingCount==0){   //init moving interval and piezo count
    cnt_initial++;
    moving_interval_x=0;
    moving_interval_y=0;
    if(cnt_initial==5){
      cnt_initial=0;
      piezo_count=0;
    }
  }

  if(cnt_initial == MAX_ACCEL_CNT){ //when the moving is detected, stop function for a while
    cnt_initial = 0;
    isMoving = false;
    Serial.println("Ready for accelometer detection");
  }
//  Serial.print("gForce: ");
//  Serial.print(gForce_int);
//  Serial.print("  movingCount: ");
//  Serial.print(movingCount);
//  Serial.print("  piezo_count: ");
//  Serial.print(piezo_count);
//  Serial.print("  cnt_initial: ");
//  Serial.println(cnt_initial);
}

