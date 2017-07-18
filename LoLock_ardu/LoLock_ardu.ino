#include <SoftwareSerial.h>
#include <LoRaShield.h>

#define RxD 0        //BLE
#define TxD 1        //BLE
#define Tx 10        //LoRa
#define Rx 11        //LoRa
#define LED 13       //inner LED
#define TriggerPin 8 //Ultrasonnic
#define EchoPin 9    //Ultrasonnic

SoftwareSerial BTSerial(RxD,TxD); // (Rx, Tx)
LoRaShield LoRa(Tx, Rx);  // (Tx, Rx)

char recv_str[100];
byte data;

String s,m;                    //LoRa ReadLine & GetMessage
String member[6];              //member
bool member_out[6];    //going out state
int member_num=0;               //member number

unsigned int loopCount = 0;
unsigned int count = 0;

long Duration = 0;
int cnt = 0;
int ccnt = 0;
boolean doorVal = false;

void setup() {
  Serial.begin(115200); // Serial Output
  BTSerial.begin(115200);
  LoRa.begin(38400);  
  pinMode(Rx, OUTPUT);  //LoRa output pin
//pinMode(TriggerPin, OUTPUT); // Trigger is an output pin
//pinMode(EchoPin, INPUT); // Echo is an input pin
  
  pinMode(LED, OUTPUT);  
  digitalWrite(LED, LOW);
  
  LoRa.SendMessage("Initializing from server",HEX); //first send message to server after booting
}

void loop() {
  while(Serial.available()){
    data = Serial.read();
    BTSerial.write(data);
    if(data == 'm')
      printMember();
  }
  
  if(recvMsg(1000)){
      Serial.write("recv: ");
      Serial.write((char*)recv_str);
      Serial.write('\n');
      switch(recv_str[0]){
        case '0': //first connected message
          BTSerial.write("LoLock Connected\n");
          Serial.write("LoLock Connected\n");
        break;
        case '1': //door open
          BTSerial.write("Open the door\n");
          Serial.write("Open the door\n");
        break;
      }
    }
  

  while (LoRa.available())
  {
    s = LoRa.ReadLine(); 
    //Serial.print("LoRa.ReadLine() = ");
    //Serial.println(s);
    
    m = LoRa.GetMessage();
    if(m != ""){
      Serial.print("Recv from LoRa : ");
      Serial.println(m);
      
      switch(m.charAt(0)){
        case '0':     //add member
          if(member_num < 6)
            addMember(m.substring(1));
          else{
            Serial.println("Member is full");
            LoRa.SendMessage("MEMBER_FULL",HEX);
          }
        break;
        case '1':   //open the door by LoRa
          if(openDoorByLoRa()){
            Serial.println("Open Success");
            LoRa.SendMessage("Open Success",HEX);
          }
          else{
            Serial.println("Open Fail");
            LoRa.SendMessage("Open Fail",HEX);
          }
        break;
        default:
          Serial.println("Not Define Command");
      }
    }
  }


  /*
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
  */
  
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


    if(loopCount == 0){
      LoRa.SendMessage("Hello LoRa",HEX);
      LoRa.PrintTTV("12", count);
      LoRa.SendTTV();
    }
  
  delay(100); // Wait to do next measurement
  loopCount++;
  count++;
  if(loopCount == 100)
    loopCount=0;
  if(count == 999999)
    count=0;
    */
}
/*
long Distance(long time)
{
  // Calculates the Distance in mm
  // ((time)*(Speed of sound))/ toward and backward of object) * 10

  long DistanceCalc; // Calculation variable
  DistanceCalc = ((time / 2.9) / 2); // Actual calculation in mm
  //DistanceCalc = time / 74 / 2; // Actual calculation in inches
  return DistanceCalc; // return calculated value
}
*/

void setupBlueToothConnection();

boolean recvMsg(unsigned int timeout){
  //wait for feedback
  unsigned int time = 0;
  unsigned int i = 0;
  
  //waiting for the first character with time out
  while(true){
    delay(50);
    if(BTSerial.available()){
      recv_str[i++] = char(BTSerial.read());
      break;
    }else
      return false;
    time++;
    if(time > (timeout/50)) return false;
  }

  while(BTSerial.available() && (i < 1024)){
    recv_str[i++] = char(BTSerial.read());
  }
  recv_str[i] = '\0';
  BTSerial.write("Received Complete");
  return true;
}

void addMember(String recv_str){                          //멤버 추가
  member[recv_str.charAt(0)-(int)('0')] = recv_str.substring(2);
  if(recv_str.charAt(1) != '0')
    member_out[recv_str.charAt(0)-(int)('0')] = true;
  member_num++;
  
  Serial.print("Member[");
  Serial.print(recv_str.charAt(0));
  Serial.print("] : ");
  Serial.println(member[recv_str.charAt(0)-(int)('0')] + " : REGISTERED");
}
boolean comparePreviousMemberForRegister(char* recv_str);   //멤보 등록

boolean goOut(void);                                        //나갈 때
boolean saveCurrentMember(void);                            //현재 연결된 멤버 저장
boolean comparePreviousMemberForLog(void);                  //저장된 멤버에서 현재 연결된 멤버 비교

boolean comeIn(void);                                       //들어올 때
boolean comparePreviousMemberInOutList(char* recv_str);     //연결된 멤버 나간 리스트에서 찾기

boolean openDoorByBLE(void);                                 //자동문
boolean openDoorByLoRa(void){                                //원격 문 제어
  return false;
}

void printMember(void){   //print member with state
  for(int i=0;i<member_num;i++){
    Serial.print("Member[");
    Serial.print(i);
    Serial.print("] : ");
    Serial.print(member[i]);
    if(member_out[i])
      Serial.println(" : OUT");
    else
      Serial.println(" : IN");
  }
}

