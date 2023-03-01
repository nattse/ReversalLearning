
bool ir_broken = false;
int no_single_ir = 0; //Sometimes IR measurements fluctuate to where there will randomly be a >10 value surrounded by zeros, so we want to eliminate these
int no_single_ir_in = 0; //But if the IR sensor is good, i.e. doesn't throw random large values out, we don't need this scheme and can get better temporal resolution

bool lever_pressed = false;
bool lever_out = false;
bool session_press = false; //True when the lever was pressed at least once during a presentation period, signals food reward and then is reset to false for start of next presentation

bool food_delay = false;
unsigned long food_delay_timer;
unsigned long food_delay_thresh = 2000;
bool dispense_on = false;
unsigned long dispense_time;

//int timing_plan[some_length];
int plan_length = 7;
int timing_plan[7] = {3000,10000,3000,10000,3000,10000,2000}; //Use some odd number (with the last one being potentially very large) so that you collect data after the final lever retraction
int step = 0; //timing_plan[step]
//int lever_plan[50];
int lever_plan[3] = {'l','r','b'};
int lp_step = 0; //lever_plan[lp_step]

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
  Serial.println("ready");

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
  //get_imported_arguments();
  //create_timing_plan(); //timing_plan: an array in the form [first wait duration, first lever out duration, second wait duration, second lever out duration...]
  
  //create_lever_plan(); //lever_plan: an array in form [which lever, which lever, which lever, etc] half the length of timing_plan. (1 == 'r', 0 == 'l')
  
  //create_food_delay(); //food_delay_thresh = the amount of time between when the lever retracts and when the food is dispensed
  //send_start_signal();
  start_time = millis();
  Serial.println(start_time);
  last_time = start_time;  
}

void loop() { 
  unsigned long time = millis() - last_time;
  measure_ir();
  //delay(50); //testing
  //return; //testing
  if (lever_out) {
    if (lever_plan[step] == "b") {
      read_lever("r");
      read_lever("l");      
    }
    else {
      read_lever(lever_plan[step]);
    }
  }
  check_food();
  if (step > (plan_length - 1)) {
    return;  
  } 
  if (timing_plan[step] > time) {
    return;
  }
  else {
    if (lever_out) {
      retract_lever();
      if (session_press == true) {
        food_delay = true;
        food_delay_timer = millis();
        session_press = false;
      }        
    }
    else {
      extend_lever();
    }
    
  }
}

void measure_ir() {
  int irValue = analogRead(detect_signal);
  //Serial.println(irValue); //testing
  //return; //testing
  if ((irValue < 10) and (ir_broken == false)) {
    if (no_single_ir_in > 5){
      Serial.print("nose_in ");
      send_report();
      ir_broken = true;
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
  if (irValue > 15) {
    no_single_ir_in = 0;
  }
}

void check_food() {
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

void extend_lever(){
  if (lever_plan[lp_step] == 'r' || lever_plan[lp_step] == 'b') {
    digitalWrite(right_lever_control, HIGH);
    Serial.print("extending right");
  }
  if (lever_plan[lp_step] == 'l' || lever_plan[lp_step] == 'b') {
    digitalWrite(left_lever_control, HIGH);
    Serial.print("extending left");
  }
  send_report();
  lever_out = true;
  step += 1; 
  last_time = millis();
}

void retract_lever(){
  if (lever_plan[lp_step] == 'r' || lever_plan[lp_step] == 'b') {
    digitalWrite(right_lever_control, LOW);
  }
  if (lever_plan[lp_step] == 'l' || lever_plan[lp_step] == 'b') {
    digitalWrite(left_lever_control, LOW);
  }
  Serial.print("retracting");
  send_report();
  lever_out = false;
  step += 1;
  lp_step += 1;
  last_time = millis();
}

void read_lever(char side){
  static bool lever_right_on = false;
  static bool lever_left_on = false;
  if (side == "r") {
    sensorValue = analogRead(right_lever_report);
    float voltage = sensorValue * (5.0 / 1023.0);
    if ((voltage < 0.5) and (lever_right_on == false)) {
      Serial.print("r_on");
      send_report();
      lever_right_on = true;
      session_press = true;
    }
    else if ((voltage > 0.5) and (lever_right_on == true)) {
      Serial.print("r_off");
      send_report();
      lever_right_on = false;    
    }
  }
  else {
    sensorValue = analogRead(left_lever_report);
    float voltage = sensorValue * (5.0 / 1023.0);
    if ((voltage < 0.5) and (lever_left_on == false)) {
      Serial.print("l_on");
      send_report();
      lever_left_on = true;
      session_press = true;
    }
    else if ((voltage > 0.5) and (lever_left_on == true)) {
      Serial.print("l_off");
      send_report();
      lever_left_on = false;    
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
