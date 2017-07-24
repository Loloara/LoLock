#include <LoRaShield.h>
#include <Servo.h>

LoRaShield LoRa1(10, 11);
Servo servo;

const int TriggerPin = 5; //Trig pin
const int EchoPin = 6; //Echo pin
#define servoPin 4 // servo Motor Pin

long Duration = 0;
int cnt = 0;
int ccnt = 0;
boolean doorVal = false;
boolean stateFlag = true;
    unsigned long t_1, t_2, t_3;
void setup() {
  pinMode(TriggerPin, OUTPUT); // Trigger is an output pin
  pinMode(EchoPin, INPUT); // Echo is an input pin
  Serial.begin(115200); // Serial Output
  LoRa1.begin(38400);
  servo.attach(servoPin);
  servo.write(0);
}

void loop() {
  float distance;
  unsigned long p_t, c_t;
  /*digitalWrite(TriggerPin, LOW);
  delayMicroseconds(2);*/
  digitalWrite(EchoPin, LOW);
  digitalWrite(TriggerPin, HIGH); // Trigger pin to HIGH
  delayMicroseconds(10); // 10us high
  digitalWrite(TriggerPin, LOW); // Trigger pin to HIGH
  
  Duration = NizPulseIn(EchoPin, HIGH, 37)/5.82;
  
  distance = ((float)(340*Duration)/10000)/2; // Use function to calculate the distance
  //Serial.println(stateFlag);
 
  Serial.print("Duration : ");
  Serial.println(Duration);
  //Serial.print("t_1 : ");
  //Serial.println(t_1);
  //Serial.print("t_2 : ");
  //Serial.println(t_2);
  //Serial.print("t_3 : ");
  //Serial.println(t_3);
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
    }
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
  {
    doorVal = false;
  }
  if (cnt < 0)
  {
    cnt = 0;
  }
  */
  
  /*Serial.print("Duration : ");
  Serial.println(Duration);
  Serial.print("Distance : ");
  Serial.print(distance);
  Serial.println("cm");
  delay(50);*/
  /*if(doorVal)
  Serial.println("Door Open!");
  else
  Serial.println("Door Closed");
  
  delay(100); // Wait to do next measurement*/
}

unsigned long NizPulseIn(int pin, int value, int timeout) { // the following comments assume that we're passing HIGH as value. timeout is in milliseconds
    unsigned long now = micros();
    while(digitalRead(pin) == value) { // wait if pin is already HIGH when the function is called, but timeout if it never goes LOW
        if (micros() - now > (timeout*500)) {
            return 0;
        }
    }
    //t_1 = micros()/100;
    now = micros(); // could delete this line if you want only one timeout period from the start until the actual pulse width timing starts
    while (digitalRead(pin) != value) { // pin is LOW, wait for it to go HIGH befor we start timing, but timeout if it never goes HIGH within the timeout period
        if (micros() - now > (timeout*500)) { 
            return 0;
        }
    }
    //t_2 = micros()/100;
    now = micros();
    while (digitalRead(pin) == value) { // start timing the HIGH pulse width, but time out if over timeout milliseconds
        if (micros() - now > (timeout*500)) {
            return 0;
        }
    }
    //t_3 = micros()/100;
    return micros() - now;
}
