#include <Uduino.h>
#define outputA 5
#define outputB 4
#include <Servo.h>
#define MAXSERVOS 8
int vib_pin=12;

Uduino uduino("Left_pedal");
int counter1=0;
int counter1_dir=0;
int lcounter1=0;
int lcounter1_dir=0;
int State1;
int LastState1;
int LastlastState1;
int i;
int li=0;
char *rotation1[]={"CW","CCW"};//String rotation 
char *lrotation1[]={"CW","CCW"};//String rotation 
char *Lastrotation1;
char *Lastlastrotation1;
char dataprint=false;

void setup() {
  pinMode(vib_pin,OUTPUT);
  pinMode(outputA,INPUT);
  pinMode(outputB,INPUT);

  Serial.begin(9600);

  LastState1=digitalRead(outputA);
  Lastrotation1="";
  Lastlastrotation1=" ";

  #if defined (__AVR_ATmega32U4__) // Leonardo
  while (!Serial) {}
#elif defined(__PIC32MX__)
  delay(1000);
#endif

  uduino.addCommand("s", SetMode);
  uduino.addCommand("d", WritePinDigital);
  uduino.addCommand("a", WritePinAnalog);
  uduino.addCommand("rd", ReadDigitalPin);
  uduino.addCommand("r", ReadAnalogPin);
  uduino.addCommand("br", BundleReadPin);
  uduino.addCommand("b", ReadBundle);
  uduino.addInitFunction(DisconnectAllServos);
  uduino.addDisconnectedFunction(DisconnectAllServos);
}

void ReadBundle() {
  char *arg = NULL;
  char *number = NULL;
  number = uduino.next();
  int len = 0;
  if (number != NULL)
    len = atoi(number);
  for (int i = 0; i < len; i++) {
    uduino.launchCommand(arg);
  }
}

void SetMode() {
  int pinToMap = 100; //100 is never reached
  char *arg = NULL;
  arg = uduino.next();
  if (arg != NULL)
  {
    pinToMap = atoi(arg);
  }
  int type;
  arg = uduino.next();
  if (arg != NULL)
  {
    type = atoi(arg);
    PinSetMode(pinToMap, type);
  }
}

void PinSetMode(int pin, int type) {
  //TODO : vérifier que ça, ça fonctionne
  if (type != 4)
    DisconnectServo(pin);

  switch (type) {
    case 0: // Output
      pinMode(pin, OUTPUT);
      break;
    case 1: // PWM
      pinMode(pin, OUTPUT);
      break;
    case 2: // Analog
      pinMode(pin, INPUT);
      break;
    case 3: // Input_Pullup
      pinMode(pin, INPUT_PULLUP);
      break;
    case 4: // Servo
      SetupServo(pin);
      break;
  }
}

void WritePinAnalog() {
  int pinToMap = 100;
  char *arg = NULL;
  arg = uduino.next();
  if (arg != NULL)
  {
    pinToMap = atoi(arg);
  }

  int valueToWrite;
  arg = uduino.next();
  if (arg != NULL)
  {
    valueToWrite = atoi(arg);

    if (ServoConnectedPin(pinToMap)) {
      UpdateServo(pinToMap, valueToWrite);
    } else {
      analogWrite(pinToMap, valueToWrite);
    }
  }
}

void WritePinDigital() {
  int pinToMap = -1;
  char *arg = NULL;
  arg = uduino.next();
  if (arg != NULL)
    pinToMap = atoi(arg);

  int writeValue;
  arg = uduino.next();
  if (arg != NULL && pinToMap != -1)
  {
    writeValue = atoi(arg);
    digitalWrite(pinToMap, writeValue);
  }
}

void ReadAnalogPin() {
  int pinToRead = -1;
  char *arg = NULL;
  arg = uduino.next();
  if (arg != NULL)
  {
    pinToRead = atoi(arg);
    if (pinToRead != -1)
      printValue(pinToRead, analogRead(pinToRead));
  }
}

void ReadDigitalPin() {
  int pinToRead = -1;
  char *arg = NULL;
  arg = uduino.next();
  if (arg != NULL)
  {
    pinToRead = atoi(arg);
  }
  
  if (pinToRead != -1)
    printValue(pinToRead, digitalRead(pinToRead));
}

void BundleReadPin() {
  int pinToRead = -1;
  char *arg = NULL;
  arg = uduino.next();
  if (arg != NULL)
  {
    pinToRead = atoi(arg);
    if (pinToRead != -1)
      printValue(pinToRead, analogRead(pinToRead));
  }
}

Servo myservo;

void loop() {
  uduino.update();
  if (uduino.isConnected()){
  int val;
  val=digitalRead(vib_pin);
  if(val==0)
  { 
    digitalWrite(vib_pin,LOW);
  }
  else{
    digitalWrite(vib_pin,HIGH);
  }
  State1=digitalRead(outputA);
  
  counter1_dir=0;
  if  (State1!=LastState1 && State1!=LastlastState1){
    if (digitalRead(outputB)!=State1){
    counter1 --;
    counter1_dir=-1;
    i=0;
    } else{
      counter1 ++;
      counter1_dir=1;
      i=1;
    }
    if (rotation1[i]==Lastrotation1 && rotation1[i]==Lastlastrotation1){  
      dataprint=true;
    }
    }
LastlastState1=LastState1;
LastState1=State1;
Lastlastrotation1=Lastrotation1;
Lastrotation1=rotation1[i];
  Serial.flush();
  if (dataprint==true){
    Serial.print("l");
    Serial.print("/");
    Serial.print(counter1_dir);
    Serial.print("/");
    Serial.print(counter1);
    Serial.println("/");
  }
  else{
    Serial.print("l");
    Serial.print("/");
    Serial.print(counter1_dir);
    Serial.print("/");
    Serial.print(counter1);
    Serial.println("/");
  }
 delay(1);
dataprint=false;
}
}

void printValue(int pin, int targetValue) {
  Serial.print(pin);
  Serial.print(" "); //<- Todo : Change that with Uduino delimiter
  Serial.println(targetValue);
  // TODO : Here we could put bundle read multiple pins if(Bundle) { ... add delimiter // } ...
}




/* SERVO CODE */
Servo servos[MAXSERVOS];
int servoPinMap[MAXSERVOS];
/*
void InitializeServos() {
  for (int i = 0; i < MAXSERVOS - 1; i++ ) {
    servoPinMap[i] = -1;
    servos[i].detach();
  }
}
*/
void SetupServo(int pin) {
  if (ServoConnectedPin(pin))
    return;

  int nextIndex = GetAvailableIndexByPin(-1);
  if (nextIndex == -1)
    nextIndex = 0;
  servos[nextIndex].attach(pin);
  servoPinMap[nextIndex] = pin;
}


void DisconnectServo(int pin) {
  servos[GetAvailableIndexByPin(pin)].detach();
  servoPinMap[GetAvailableIndexByPin(pin)] = 0;
}

bool ServoConnectedPin(int pin) {
  if (GetAvailableIndexByPin(pin) == -1) return false;
  else return true;
}

int GetAvailableIndexByPin(int pin) {
  for (int i = 0; i < MAXSERVOS - 1; i++ ) {
    if (servoPinMap[i] == pin) {
      return i;
    } else if(pin == -1 && servoPinMap[i] < 0) {
     return i; // return the first available index 
    }
  }
  return -1;
}

void UpdateServo(int pin, int targetValue) {
  int index = GetAvailableIndexByPin(pin);
  servos[index].write(targetValue);
  delay(10);
}

void DisconnectAllServos() {
  for (int i = 0; i < MAXSERVOS; i++) {
    servos[i].detach();
    digitalWrite(servoPinMap[i], LOW);
    servoPinMap[i] = -1;
  }
}
