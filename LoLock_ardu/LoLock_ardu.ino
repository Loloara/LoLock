#include <LoRaShield.h>
#include <CurieBLE.h>
#include <Servo.h>
#include <CurieIMU.h>

#define SERVOPIN 4          //servo Motor Pin
#define Tx 10               //LoRa
#define Rx 11               //LoRa
#define PUSH 12             //pushButton Pin
#define OPEN 90          //angle of door open
#define CLOSE 0          //angle of door close
#define PIEZO A1         //piezo vibration sensor Pin

//BLE 서비스 설정을 위한 선언구간
BLEService service = BLEService("FEAA"); //UUID -> 0xFEAA로 뜸.
BLECharacteristic characteristic( "FEAA", BLEBroadcast, 50 );
//uint8 설정이 많은 이유-> HEX값으로 URL지정함.
const uint8_t FRAME_TYPE_EDDYSTONE_UID = 0x00;
const uint8_t FRAME_TYPE_EDDYSTONE_URL = 0x10;
const uint8_t FRAME_TYPE_EDDYSTONE_TLM = 0x20;
const uint8_t FRAME_TYPE_EDDYSTONE_EID = 0x40;
const uint8_t URL_PREFIX_HTTP_WWW_DOT = 0x00;
const uint8_t URL_PREFIX_HTTPS_WWW_DOT = 0x01;
const uint8_t URL_PREFIX_HTTP_COLON_SLASH_SLASH = 0x02;
const uint8_t URL_PREFIX_HTTPS_COLON_SLASH_SLASH = 0x03;
const uint8_t URL_EXPANSION_COM = 0x07;

const int8_t TX_POWER_DBM = -29; // (-70 + 41);Tx 송출시 기기와 측정장비 사이의 거리가 0m일 때 -29dBm

//로라 쉴드, 서보모터,초음파센서 선언부
LoRaShield LoRa(Tx, Rx);
String s,m;
Servo servo;
boolean push_cnt = false;  //서보모터가 돌아가 있는지 여부
boolean asButton = false; //버튼을 통해 문이 열렸는지 확인하는 변수

int accelFuncCnt = 0;
float ax, ay, az;   //scaled accelerometer values
const float gravity_earth = 9.80665f;
float gForce;
float preVal;
int diffCount = 0;
int movingCount = 0;
int cnt_initial = 0;
boolean isMoving = false;

void setup() 
{
  //로라 설정
  LoRa.begin(38400);
  Serial.begin(115200); //보드레이트 이격
  // enable if you want to log values to Serial Monitor



  //BLE 설정, 실행
  // begin initialization
  BLE.begin();

  // No not set local name 
  //만약 local name을 지정해주면 비콘이 탐지 되지 않음, 설정이 부족해서 안 뜨는 것 같기도 함.
  //BLE.setLocalName("LOLO-LOCK");

  // set service
  BLE.setAdvertisedService(service);
  
  // add the characteristic to the service
  service.addCharacteristic( characteristic );

  // add service
  BLE.addService( service );

  // call broadcast otherwise Service Data is not included in advertisement value
  characteristic.broadcast();

  // characteristic.writeValue *after* calling characteristic.broadcast
  uint8_t advdatacopy[] =
  {   
    FRAME_TYPE_EDDYSTONE_URL,
    (uint8_t) TX_POWER_DBM, // Tx Power. Cast to uint8_t or you get a "warning: narrowing conversion"
    // I suppose it's doing 2s Complement conversion to convert from a negative integer to a hex value.  
    URL_PREFIX_HTTP_WWW_DOT, // http://www.
    0x67,0x6f,0x6f,0x67,0x6c,0x65, // google //여기 고치면 URL 바꾸기 가능. 각각의 알파벳마다 ASCII HEX값 지정만 알아내서 넣으면 가능.
    URL_EXPANSION_COM // .com
  };
  characteristic.writeValue( advdatacopy, sizeof(advdatacopy) );

  // start advertising
  BLE.advertise();  

  pinMode(PUSH, INPUT);
  pinMode(Rx, OUTPUT);
  
  //서보  
  servo.attach(SERVOPIN);
  servo.write(CLOSE);

  CurieIMU.begin();
  CurieIMU.setAccelerometerRange(2);  
  Serial.println("Run Success");
}
 
void loop() 
{  
  //푸시버튼, 문 열림상태 감지 함수.
  pushButton();
  if(accelFuncCnt == 5000){
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
      if(m == "26")
        openDoorByLoRa();
     }
   }
}


void pushButton()
{
  int i = digitalRead(PUSH);  // 12번 디지털 입력으로 전압을 읽어들임
  Serial.println(i);

  if(!i && !push_cnt)                // i 상태 == 1
  {     
    servo.write(OPEN);  //모터 90도 회전
    asButton = true;
    push_cnt = true;
  }
  else 
  {
    if(push_cnt)
    {     
      delay(100);
      servo.write(CLOSE);
      Serial.println("button pushed");
      push_cnt = false;
    }
  }
}

void openDoorByLoRa(void)
{
  servo.write(OPEN);
  delay(100);  
  servo.write(CLOSE);
  Serial.println("Open Success");
}

void doorCheckByAccel(void){
    // read accelerometer measurements from device, scaled to the configured range
  CurieIMU.readAccelerometerScaled(ax, ay, az);

  // display tab-separated accelerometer x/y/z values
  ax = ax / gravity_earth;
  ay = ay / gravity_earth;
  az = az / gravity_earth;
  gForce = ax*ax + ay*ay+ az*az;
  gForce = sqrt(gForce) * 10;
  
  if(String(gForce) != String(preVal)){
    diffCount++;
  }else{
    diffCount = 0;
  }
  preVal = gForce;
  
  if(diffCount > 10 && !isMoving){
    movingCount++;
    isMoving = true;
    if(asButton){
      Serial.println("Door is open inside");
      LoRa.SendMessage("Door is opened inside",HEX);
    }else{
      Serial.println("Door is opened outside");
      LoRa.SendMessage("Door is open outside",HEX);
    }
    asButton = false;
  }else if(isMoving){
    cnt_initial++;
  }

  if(cnt_initial == 100){
    cnt_initial = 0;
    isMoving = false;
    diffCount = 0;
  }
//  Serial.print("gForce: ");
//  Serial.print(gForce);
//  Serial.print("          movingCount: ");
//  Serial.print(movingCount);
//  Serial.print("          cnt_initial: ");
//  Serial.println(cnt_initial);
}
