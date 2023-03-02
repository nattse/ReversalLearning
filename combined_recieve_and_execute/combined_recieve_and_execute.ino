/* 
Setup variables
*/
bool size_recieved = false;
bool array_made = false;
bool sides_planned = false;
const byte numChars = 64;
char size_temp[numChars] = {};
int prot_size = 0; //Number of steps in the recieved time_plan
int RightLeverOut[25] = {}; 
int RightLeverProb[25] = {};
int LeftLeverOut[25] = {}; 
int LeftLeverProb[25] = {};
//int PressToSwitch[25] = {};
//int TimeToSwitch[25] = {};
int PressToAdvance[25] = {};
int TimeToAdvance[25] = {};

int consec_left = 0;
int consec_right = 0;

int num_presses = 0;
int step = 0;
/*
Actual proccess variables
*/

bool start_round = false;

bool ir_broken = false;
int no_single_ir = 0; //Sometimes IR measurements fluctuate to where there will randomly be a >10 value surrounded by zeros, so we want to eliminate these
int no_single_ir_in = 0; //But if the IR sensor is good, i.e. doesn't throw random large values out, we don't need this scheme and can get better temporal resolution

bool lever_pressed = false;
bool lever_out = false;
int session_press = -1; //True when the lever was pressed at least once during a presentation period, signals food reward and then is reset to false for start of next presentation

bool food_delay = false;
unsigned long food_delay_timer;
unsigned long food_delay_thresh = 2000;
bool dispense_on = false;
unsigned long dispense_time;

unsigned long start_time;
unsigned long last_time;

//Lever and dispenser pins and stuff
int left_lever_control = 12;
int left_lever_report = A5;
int left_led = 2;
int right_lever_control = 13;
int right_lever_report = A4;
int right_led = 3;
int dispense_control = 11;

int control; 
int sensorValue;

//IR detector pins and stuff
int ir_source = 9;
int detect_power = 8;
int detect_signal = A3;

void setup() {
  Serial.begin(9600);

  //Lever and dispenser pins and stuff
  pinMode(left_lever_control, OUTPUT);
  pinMode(left_lever_report, INPUT);
  pinMode(left_led, OUTPUT);

  pinMode(right_lever_control, OUTPUT);
  pinMode(right_lever_report, INPUT);
  pinMode(right_led, OUTPUT);

  pinMode(dispense_control, OUTPUT);
  //IR detector pins and stuff
  pinMode(ir_source, OUTPUT);
  pinMode(detect_power, OUTPUT);
  pinMode(detect_signal, INPUT);
  digitalWrite(ir_source, HIGH);
  digitalWrite(detect_power, HIGH);
    
  Serial.println("ready");
  protocol_setup();
  
  start_time = millis();
  //Serial.println(start_time);
  last_time = start_time;  
  Serial.println("entering loop");
  Serial.println(RightLeverOut[0]);
  Serial.println(RightLeverProb[0]);
  Serial.println(LeftLeverOut[0]);
  Serial.println(LeftLeverProb[0]);
  //Serial.println(PressToSwitch[0]);
  //Serial.println(TimeToSwitch[0]);
  Serial.println(PressToAdvance[0]);
  Serial.println(TimeToAdvance[0]);
}

void loop() { 
  unsigned long time = millis() - last_time;
  check_switch();
  measure_ir();
  if (lever_out) {
    if (RightLeverOut[step] == 1) {
      read_lever('r');
    }
    if (LeftLeverOut[step] == 1) {
      read_lever('l');      
    }
  }
  check_food();
  if (time < 3000) { 
    start_round = false;
  }
  if (start_round == false) {
    return;
  }
  //if (step > (prot_size - 1)) {
  //  return;  
  //} 
  else {
    if (session_press != -1) {
      if (RightLeverOut[step] == 1){
        retract_lever('r');
      }
      if (LeftLeverOut[step] == 1) {
        retract_lever('l');
      }
      reward_calculation();
      session_press = -1;
      start_round = false;
      last_time = millis();
      num_presses += 1;
    }        
    else {
      if (RightLeverOut[step] == 1) {
        extend_lever('r');
      }
      if (LeftLeverOut[step] == 1) {
        extend_lever('l');
      }
    }
  }
}

void check_switch() {
  if (step == prot_size) {
    Serial.println("complete end");    
    return;
  }
  if (TimeToAdvance[step] == -1) {
    if (num_presses > PressToAdvance[step]) {
      step += 1;
      num_presses = 0;
      Serial.println("next step");
    }
  }
  else {
    if ((millis() - start_time) > TimeToAdvance[step]) {
      step += 1;
      num_presses = 0;
      Serial.println("next step");
    }
  }
}

void measure_ir() {
  int irValue = analogRead(detect_signal);
  //Serial.println(irValue);
  if ((irValue < 10) and (ir_broken == false)) {
    if (no_single_ir_in > 5){
      Serial.print("nose_in ");
      send_report();
      ir_broken = true;
      if (lever_out == false) {
        start_round = true;
      }
    }
    else {
      no_single_ir_in += 1;
    }
  }
  if (irValue < 10) {
    no_single_ir = 0;
  }
  if ((irValue > 15) and (ir_broken == true)) {
    if (no_single_ir > 5) {
      Serial.print("nose_out ");
      send_report();
      no_single_ir = 0;
      ir_broken = false;  
    }
    else {
      no_single_ir += 1;
    }
  }
  if (irValue > 10) {
    no_single_ir_in = 0;
  }
}

void check_food() {
  //Serial.println("chec food");
  if (food_delay) {
    if ((millis() - food_delay_timer) < food_delay_thresh) {
      return;
    }
    else if (dispense_on == false) {
      digitalWrite(dispense_control, HIGH);
      dispense_on = true;
      dispense_time = millis();
      Serial.println("dispensing...");     
    }    
    else if ((millis() - dispense_time) > 5){
      digitalWrite(dispense_control, LOW);
      dispense_on = false;    
      food_delay = false;  
      Serial.println("finished dispensing");
    }
  }
}

//Will not dispense if one lever is used three times or more, consecutively
void reward_calculation(){
  //Serial.println("rewardtime");
  long r = random(100);
  if (session_press == 1) {
    if (r < RightLeverProb[step] && consec_right < 3) {
      food_delay = true; // handle in func
      food_delay_timer = millis(); // handle in func
    }  
    else {
      return;
    }
  }
  else if (session_press == 2) {
    if (r < LeftLeverProb[step] && consec_left < 3) {
      food_delay = true;
      food_delay_timer = millis();
    }
    else {
      return;
    }
  }
}

void extend_lever(char side){
  if (side == 'r') {
    digitalWrite(right_lever_control, HIGH);
    //Serial.print("extending right");
  }
  if (side == 'l') {
    digitalWrite(left_lever_control, HIGH);
    //Serial.print("extending left");
  }
  //send_report();
  lever_out = true;
  //last_time = millis();
}

void retract_lever(char side){
  //Serial.println("retractintime");
  if (side == 'r') {
    digitalWrite(right_lever_control, LOW);
  }
  if (side == 'l') {
    digitalWrite(left_lever_control, LOW);
  }
  //Serial.print("retracting");
  //send_report();
  lever_out = false;
  //last_time = millis();
}

void read_lever(char side){
  //Serial.println("readlever");
  if (side == 'r') {
    sensorValue = analogRead(right_lever_report);
    float voltage = sensorValue * (5.0 / 1023.0);
    if ((voltage < 0.5) and (session_press != 1)) {
      Serial.print("r_on");
      send_report();
      session_press = 1;
      consec_right += 1;
      consec_left = 0;
    }
    else if ((voltage > 0.5) and (session_press == 1)) {
      Serial.print("r_off");
      send_report();
    }
  }
  else {
    sensorValue = analogRead(left_lever_report);
    float voltage = sensorValue * (5.0 / 1023.0);
    if ((voltage < 0.5) and (session_press != 2)) {
      Serial.print("l_on");
      send_report();
      session_press = 2;
      consec_left += 1;
      consec_right = 0;
    }
    else if ((voltage > 0.5) and (session_press == 2)) {
      Serial.print("l_off");
      send_report();    
    }
  }
}

//Gives time since start of experiment in Min:Sec:Millis
void send_report() {
  unsigned long initial_mils = millis() - start_time;
  int report_mils = initial_mils % 1000; //What remains after converting to seconds
  int initial_sec = initial_mils / 1000; //Seconds without minutes removed
  int report_min = initial_sec / 60; //Removing minutes
  int report_sec = initial_sec % 60; //What remains after converting to minutes
  char buf[11];
  sprintf(buf,"%02d:%02d:%03d",report_min,report_sec,report_mils);
  Serial.println(buf);
}

/*
The code below allows us to upload a protocol to the Arduino which will tell it
how long to wait before presenting the lever and how long to present the lever for,
for every single lever presentation. This protocol (time_plan in the .py file) is stored
here in finalArray in the form 
[wait, lever, wait, lever...ending_duration]

We also upload another plan which tells the Arduino which lever to use, represented by binary
entries in sideArray.

Once finalArray is filled, the code sends the stored data back to the .py file to ensure 
that no data was lost on the way over. 

As to how finalArray works together with prot_size:
finalArrary needs to be a gloabl variable, but we can't easily create an array on the fly
due to how the Arduino code works. As a result, we have to create finalArray at the start
and give it a ton of spaces that we will fill in, hence finalArray being of size 100 here.
However, we'll very likely never fill finalArray perfectly, so there will be extra zeros
trailing our last entry in finalArray. In order to step through finalArray without going past
our last entry, we also upload to the Arduino the number of entries there are in total, which
is stored in prot_size. 
*/



void protocol_setup() {
  static bool done = false;

  while (prot_size == 0) {
    get_size();
  }
  Serial.println(prot_size);    
  fill_array(RightLeverOut);
  fill_array(RightLeverProb);
  fill_array(LeftLeverOut);
  fill_array(LeftLeverProb);
  //fill_array(PressToSwitch);
  //fill_array(TimeToSwitch);
  fill_array(PressToAdvance);
  fill_array(TimeToAdvance);
}

void get_size() {
  char startMarker = '<';
  char endMarker = '>';
  static byte count = 0;
  static bool incoming = false;
  while (Serial.available() > 0 && size_recieved == false) {
    char rc = Serial.read();
    //Serial.println(rc);
    if (incoming == true) {
      if (rc != endMarker) {
        size_temp[count] = rc;
        count += 1;
      }
      else {
        size_temp[count] = '\0';
        size_recieved = true;
        incoming = false;
        count = 0;
        prot_size = atoi(size_temp);
        Serial.print("recieved size: ");
      }
    }
    else if (rc == startMarker) {
      incoming = true;
    }
  }
}

void fill_array(int someArray[]) {
  int big_count = 0;
  static char temp_temp[numChars] = {};
  char startMarker = '<';
  char endMarker = '>';
  static byte count = 0;
  static bool incoming = false;
  static bool arrayFlag = false;  
  while (arrayFlag == false) {
    if (Serial.available() > 0) {   
      char rc = Serial.read();
      if (incoming == true) {
        if (rc == 'f') {
          rc = 'm';
          big_count = 0;   
          temp_temp[numChars] = {};
          count = 0;
          incoming = false;
          Serial.println("feedback");
          for (int i=0; i<prot_size; i++) {
            Serial.println(someArray[i]);
          }
          Serial.println("done");
          return;
        }
        else if (rc != endMarker) {
          temp_temp[count] = rc;
          count += 1;
        }
        else {
          temp_temp[count] = '\0';
          incoming = false;
          count = 0;
          int tt = atoi(temp_temp);
          someArray[big_count] = tt;
          big_count += 1;
          temp_temp[numChars] = {};
          }
        }
      else if (rc == startMarker) {
        incoming = true;
      }
    }
  }
}  
