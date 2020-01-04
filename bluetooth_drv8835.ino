// drv8835   Arduino pin
const int light = 3;
const int IN_B = 4;
const int EN_B  = 5;
const int EN_A  = 6;
const int IN_A = 7;
const int batteryPin = A7; 

char cmd[100];
int cmdIndex;

int oldLeftV;
int oldRightV; 
int speedSlider = 0;
int accelerometer = 0; 
int leftV = 0;
int rightV = 0;

int bSpeed=0;
int aSpeed=0;
int bSpeedOld;
int aSpeedOld;

boolean fastTurnAccMode = 0;
boolean lights = 0;

unsigned long lastCmdTime = 0;
unsigned long lastBattTime = 0;


boolean cmdStartsWith(const char *st) {
  for(int i = 0; ; i++) {
    if(st[i] == 0) return true;
    if(cmd[i] == 0) return false;
    if(cmd[i] != st[i]) return false;;
  }
  return false;
}


void onConnectionLost() {
  // stop 
  analogWrite(EN_B, 0);
  digitalWrite(IN_B, LOW);
  analogWrite(EN_A, 0);
  digitalWrite(IN_A, LOW);
}


void exeCmd() {
lastCmdTime = millis();

if (cmdStartsWith("lights")) {
  lights =! lights;
  if (lights) digitalWrite(light, HIGH);
  else digitalWrite(light, LOW);
}

if (cmdStartsWith("fastTurnAccMode")) fastTurnAccMode =! fastTurnAccMode;
  
if(cmdStartsWith("x ") ) {
  speedSlider = atoi(cmd + 2);
  //prosta petla histerezy
  if ((speedSlider < 55) && (speedSlider > -55)) speedSlider = 0; 
} 
    
if(cmdStartsWith("y ") ) {
  accelerometer = atoi(cmd + 2);
  //prosta petla histerezy
  if ((accelerometer < 55)  && (accelerometer > -55)) accelerometer = 0;
} 

if (speedSlider > 0) {
  if (accelerometer > 0) {  
    if (fastTurnAccMode) leftV = - accelerometer; // prędkość silnika lewego
    else leftV = speedSlider - accelerometer;
    rightV = speedSlider; // prędkość silnika prawego
  }
  else if (accelerometer < 0) {
    leftV = speedSlider; // prędkość silnika lewego
    if (fastTurnAccMode) rightV = accelerometer; // prędkość silnika prawego
    else rightV = speedSlider + accelerometer;
  }
  else   {
    leftV = speedSlider;
    rightV = speedSlider; 
  }
}

else if (speedSlider < 0) {
  if (accelerometer > 0) {  
    if (fastTurnAccMode) leftV =  accelerometer; // prędkość silnika lewego
    else leftV =  speedSlider + accelerometer;
    rightV = speedSlider; // prędkość silnika prawego
  }
  else if (accelerometer < 0) {
    leftV = speedSlider; // prędkość silnika lewego
    if (fastTurnAccMode) rightV = - accelerometer; // prędkość silnika prawego
    else rightV = speedSlider - accelerometer;
  }
  else   {
    leftV = speedSlider;
    rightV = speedSlider; 
  }
}

else {
  if (accelerometer > 0) {  
    if (fastTurnAccMode) leftV =  - accelerometer; // prędkość silnika lewego
    else leftV = speedSlider;
    rightV =  accelerometer; // prędkość silnika prawego
  }
  else if (accelerometer < 0) {
    leftV = - accelerometer; // prędkość silnika lewego
    if (fastTurnAccMode) rightV =  accelerometer; // prędkość silnika prawego
    else rightV = speedSlider;
  }
  else   {
    leftV = 0;
    rightV = 0; 
  }
}
  

if(cmdStartsWith("b ") ) {
  bSpeedOld = bSpeed;
  bSpeed = atoi(cmd + 2);
  //prosta petla histerezy
  if ((bSpeed < 55) && (bSpeed > -55)) bSpeed = 0; 
    
  if(bSpeed > 0) {
    if(bSpeedOld < 0) analogWrite(EN_B, 0);
    // do przodu
    digitalWrite(IN_B, HIGH);
    analogWrite(EN_B, bSpeed);
  }

  if(bSpeed < 0) {
    if(bSpeedOld > 0) analogWrite(EN_B, 0);
    // do tylu
    digitalWrite(IN_B, LOW);
    analogWrite(EN_B, -bSpeed);
  }

  if(bSpeed == 0) analogWrite(EN_B, 0);

}

else if(cmdStartsWith("a ") ) {
  aSpeedOld = aSpeed;
  aSpeed = atoi(cmd + 2);
   //prosta petla histerezy
  if ((aSpeed < 55) && (aSpeed > -55)) aSpeed = 0;
    
  if(aSpeed > 0) {
    if(aSpeedOld < 0) analogWrite(EN_A, 0);
    // do przodu
    digitalWrite(IN_A, HIGH);
    analogWrite(EN_A, aSpeed);
  }
    
  if(aSpeed < 0) {
    if(aSpeedOld > 0) analogWrite(EN_A, 0);
    // do tylu
    digitalWrite(IN_A, LOW);
    analogWrite(EN_A, -aSpeed);
  }
    
  if(aSpeed == 0) analogWrite(EN_A, 0);

}

else {
  motorLeft(leftV);
  oldLeftV = leftV; 
  motorRight(rightV); 
  oldRightV = rightV;
  } 
  
}


void motorLeft(int V) {
  if (V > 0) {
    if (oldLeftV < 0) analogWrite(EN_B, 0); 
    digitalWrite(IN_B, HIGH); //Kierunek do przodu  
  } 

  else digitalWrite(IN_B, LOW); //Kierunek do tyłu   
  
  V=abs(V);
  analogWrite(EN_B, V);
}


void motorRight(int V) {
  if (V > 0) {  
    if (oldRightV < 0) analogWrite(EN_A, 0);
    digitalWrite(IN_A, HIGH); //Kierunek do przodu    
  } 

  else digitalWrite(IN_A, LOW); //Kierunek do tyłu    

  V=abs(V);
  analogWrite(EN_A, V);
}


void batteryRead() {
  double batteryV = 0;
  lastBattTime = millis();

  for (int i = 1; i < 100; i++)
  batteryV = (batteryV * (i - 1) + analogRead(batteryPin)) / i;

  int battery= (int) ((batteryV - 459) * 1.1654676258992805755395683453237 + 3555);
    // dla 3555mV ADC jest 459, 1.65... to faktor = (WartoscNapiecia.w.mV - 3555)/(WartoscADC - 459) 
  Serial.print("batteryV ");
  Serial.println(battery);
  Serial.print("batteryVadc ");
  Serial.println(batteryV);
}


void setup() {
  delay(500); // czekaj na uruchomienie bluetooth
  Serial.begin(9600);
  pinMode(light, OUTPUT);
  digitalWrite(light, LOW);
  pinMode(EN_B, OUTPUT);
  analogWrite(EN_B, 0);
  pinMode(EN_A, OUTPUT);
  analogWrite(EN_B, 0);
  pinMode(IN_B, OUTPUT);
  pinMode(IN_A, OUTPUT);
  pinMode(batteryPin, INPUT);
  cmdIndex = 0;
}


void loop() {
  // sprawdzanie napięcia baterii co 10s
  if( millis() - lastBattTime > 10000) batteryRead();
  // jeśli nic nie odebrane przez 500ms
  if( millis() - lastCmdTime > 500) onConnectionLost();
 
  if(Serial.available()) {
    
    char c = (char)Serial.read();
    
    if(c == '\n') {
      cmd[cmdIndex] = 0;
      exeCmd();  
      cmdIndex = 0;
    } 
    else {      
      cmd[cmdIndex] = c;
      if(cmdIndex<99) cmdIndex++;
    }
  }
}