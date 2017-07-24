#include <LoRaShield.h>
#include <CurieBLE.h>
#include <Servo.h>

#define SERVOPIN 5          //servo Motor Pin
#define TriggerPin 8        //Trig Pin
#define EchoPin 9           //Echo Pin
#define Tx 10               //LoRa
#define Rx 11               //LoRa
#define PUSH 12             //pushButton Pin
#define OPEN 60          //angle of door open
#define CLOSE 0          //angle of door close

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
long duration_val = 0;
boolean push_cnt = false;  //서보모터가 돌아가 있는지 여부
int cnt = 0;               //초음파 센서 알고리즘
boolean door_val = false; //문의 상태 TRUE:OPEN FALSE:CLOSE
boolean asButton = false; //버튼을 통해 문이 열렸는지 확인하는 변수


void setup() 
{
  //로라 설정
  LoRa.begin(38400);
  Serial.begin(9600);
  // enable if you want to log values to Serial Monitor
  // Serial.begin(9600);



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

  //서보
  pinMode(PUSH, INPUT);
  servo.attach(SERVOPIN);
  servo.write(CLOSE);
}
 
void loop() 
{  
  //푸시버튼, 문 열림상태 감지 함수.
  pushButton();
  DoorOpenState();
  //로라 실행구간.
   while (LoRa.available())
   {
    s = LoRa.ReadLine();
    Serial.print("LoRa.ReadLine() = ");
    Serial.println(s);

    m = LoRa.GetMessage();
    if(m != ""){
      Serial.println("Recv from LoRa : " + m);
     }
   }
}


void pushButton()
{
  int i = digitalRead(PUSH);  // A5 아날로그 입력으로 전압을 읽어들임

  if(!i)// i 상태 == 1
  {     
    servo.write(OPEN);  //모터 60도 회전
    asButton = true;
    push_cnt = true;
  }
  else 
  {
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
 
  //진원이형이 해결한구간?
  //duration_val = pulseIn(EchoPin, HIGH); // Waits for the echo pin to get high
  //returns the Duration in microseconds

  long Distance_mm = Distance(duration_val); // Use function to calculate the distance
  
  /*if(Distance_mm > 500 && Distance_mm < 30000) 
  {
    cnt++;
    
  }
  else
  {
    cnt--;    
  }
  if(cnt > 10)
  {  
    door_val = true;
    if(asButton){
      LoRa.SendMessage("Door is open inside",HEX);
    }else{
      LoRa.SendMessage("Door is open outside",HEX);
    }
    
    cnt = 11;
  }
  else if (cnt == 0){
    door_val = false;
    asButton = false;
   }
  if (cnt < 0)
    cnt = 0; */
}

//초음파 센서 거리 계산 공식
long Distance(long _time)
{
  // Calculates the Distance in mm
  // ((time)*(Speed of sound))/ toward and backward of object) * 10
  long DistanceCalc; // Calculation variable
  DistanceCalc = ((_time / 2.9) / 2); // Actual calculation in mm
  //DistanceCalc = time / 74 / 2; // Actual calculation in inches
  return DistanceCalc; // return calculated value
}


boolean openDoorByLoRa(void)
{
  //if(Closed){
  servo.write(OPEN);
  
  servo.write(CLOSE);
  Serial.println("Open Success");
  LoRa.SendMessage("Open Success",HEX);
  return true;

  //}
  //if(Opened){
  //Serial.println("Open Fail");
  //LoRa.SendMessage("Open Fail",HEX);
  // return false;}
}
