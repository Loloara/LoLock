#include <SoftwareSerial.h>
//#include <LoRaShield.h>


//LoRaShield LoRa(10, 11);
SoftwareSerial BTSerial(0,1); // (RX, TX)

char recv_str[100];
byte data;

unsigned int loopCount = 0;
unsigned int count = 0;

const int TriggerPin = 8; //Trig pin
const int EchoPin = 9; //Echo pin
long Duration = 0;
int cnt = 0;
int ccnt = 0;
boolean doorVal = false;

int LED = 13;

void setup() {
//  pinMode(TriggerPin, OUTPUT); // Trigger is an output pin
//  pinMode(EchoPin, INPUT); // Echo is an input pin
  Serial.begin(115200); // Serial Output
  //LoRa.begin(38400);
//  pinMode(11, OUTPUT);
  BTSerial.begin(115200);
  
  pinMode(LED, OUTPUT);  
  digitalWrite(LED, LOW);
}

void loop() {

  if(Serial.available()){
    BTSerial.write(Serial.read());
  }
  
  if(recvMsg(1000)){
      Serial.print("recv: ");
      Serial.print((char*)recv_str);
      Serial.println("");
    }
  
/*
  while (LoRa.available())
  {
    String s = LoRa.ReadLine(); 
    Serial.print("LoRa.ReadLine() = ");
    Serial.println(s);
    
     String m = LoRa.GetMessage();    
    if(m != ""){
      Serial.print("LoRa.GetMessage() = ");
      Serial.println(m);
    }

    if(m == "280101")
      digitalWrite(led, HIGH);
    else if(m == "280100")
      digitalWrite(led, LOW);
  }
*/

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
    }
    time++;
    if(time > (timeout/50)) return false;
  }

  while(BTSerial.available() && (i < 1024)){
    recv_str[i++] = char(BTSerial.read());
  }
  recv_str[i] = '\0';
  BTSerial.print("Received Complete");
  return true;
}

