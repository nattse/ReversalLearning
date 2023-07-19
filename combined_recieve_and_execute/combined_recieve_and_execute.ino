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
int PressToAdvance[25] = {};
int TimeToAdvance[25] = {};
int cycles_required_right[25] = {}; // number of discreet lever presses required; either -1 or some number
int cycles_required_left[25] = {};
//test variables
//int prot_size = 1;
//int RightLeverOut[] = {1};
//int RightLeverProb[] = {100};
//int LeftLeverOut[] = {1}; 
//int LeftLeverProb[] = {100};
//int PressToAdvance[] = {100};
//int TimeToAdvance[] = {3000};
//int cycles_required_right[] = {2}; // number of discreet lever presses required; either -1 or some number
//int cycles_required_left[] = {2};
//end test variables
unsigned long TimeSinceLastStep = 0; // When we change steps we reset the clock that we're comparing current time to
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
int session_press = -1;

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
int food_light = 4;
unsigned long food_light_time;
int control; 
int sensorValue;

//IR detector pins and stuff
int ir_source = 9;
int detect_power = 8;
int detect_signal = A3;

// Multi press scheme
int no_single_lever_right = 0;
int no_single_lever_left = 0;
int no_single_lever_off_right = 0;
int no_single_lever_off_left = 0;
int completed_cycles_right = 0;
int completed_cycles_left = 0;
bool lever_depressed_right = false;
bool lever_depressed_left = false;

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
  pinMode(food_light, OUTPUT);
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
  TimeSinceLastStep = start_time;
  Serial.println("entering loop");
  //Serial.println(RightLeverOut[0]);
  //Serial.println(RightLeverProb[0]);
  //Serial.println(LeftLeverOut[0]);
  //Serial.println(LeftLeverProb[0]);
  //Serial.println(PressToSwitch[0]);
  //Serial.println(TimeToSwitch[0]);
  //Serial.println(PressToAdvance[0]);
  //Serial.println(TimeToAdvance[0]);
}

/*
Measurements (IR beam, levers) and controls of devices are done via functions that have global on/off switches and counters,
allowing us to loop through them while maintaining their states. In the main body, all timing is relative to last_time, which resets
every full [nose in/lever press/reward calculation] loop. We loop through the top, not allowing start_round to be true until the cooldown period
is over (representing time after food reward is given where mouse cannot trigger new round of lever presentation). All the while, measure_ir is trying
to start the round. Once cooldown ends, an IR beam break results in entry to the bottom portion of the loop. Levers are extended and we wait for read_lever
to change the session_press variable. Once session_press is changed, we retract all levers and calculate reward based on which lever was pressed (which is 
represented by the session_press variable). last_time is then reset, session_press and some other lever-related variables wiped, and we start over again until
conditions are met to proceed to the next step.
*/
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
  else {
    if (session_press != -1) {
      if (RightLeverOut[step] == 1){
        retract_lever('r');
      }
      if (LeftLeverOut[step] == 1) {
        retract_lever('l');
      }
      //Serial.println(num_presses); 
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

/*
Check two conditions (number of presses since start of step, time since step started) 
to determine whether to move on to the next step
*/
void check_switch() {
  if (step == prot_size) {
    Serial.println("complete end"); 
    retract_lever('r');
    retract_lever('l');
    return;
  }
  if (PressToAdvance[step] != -1) {
    if (num_presses > PressToAdvance[step]) {
      step += 1;
      num_presses = 0;
      TimeSinceLastStep = millis();
      Serial.print("next step ");
      send_report();
    }
  }
  if (TimeToAdvance[step] != -1) {
    if (((millis() - TimeSinceLastStep) / (1000)) > TimeToAdvance[step]) { // Measure in seconds
      step += 1;
      num_presses = 0;
      retract_lever('r');
      retract_lever('l');
      start_round = false;
      TimeSinceLastStep = millis();
      Serial.print("next step ");
      send_report();
    }
  }
}

/*
These irValue comparisons (<10 being beam broken, >15 being beam unbroken) are
correct for new IR emitter/reciever pairs. Signals may begin to fluctuate around these 
values if the emitter/reciever pairs get degraded (e.g. chewed). This can appear as erratic 
nose in/out signals or as lagging in python recordings
*/
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

/*
Dispenser requires a rising/falling edge so we time a quick HIGH/LOW switch outside the main loop
An LED illuminatng the food bin upon reward also gets timed outside the loop
*/
void check_food() {
  //Serial.println("chec food");
  if (food_delay) {
    if ((millis() - food_delay_timer) < food_delay_thresh) {
      return;
    }
    else if (dispense_on == false) {
      digitalWrite(dispense_control, HIGH);
      digitalWrite(food_light, HIGH);
      dispense_on = true;
      dispense_time = millis();
      food_light_time = millis();
      //Serial.println("dispensing...");     
    }    
    else if ((millis() - dispense_time) > 5){
      digitalWrite(dispense_control, LOW);
      dispense_on = false;    
      food_delay = false;  
      Serial.println("rewarded");
    }
  }
  if ((millis() - food_light_time) > 3000) {
  digitalWrite(food_light, LOW);
  food_light_time = 0;
  }
}

/*
Will not dispense if one lever is used 5 times or more, consecutively, useful during training when both levers are 100% rewarded
If this behavior is not desired, remove "&& consec_right < 5" from below, or change to a different number of consecutive presses
*/
void reward_calculation(){
  long r = random(100);
  if (session_press == 1) {
    if (r < RightLeverProb[step] && consec_right < 5) {
      food_delay = true; // handle in func
      food_delay_timer = millis(); // handle in func
    }  
    else {
      return;
    }
  }
  else if (session_press == 2) {
    if (r < LeftLeverProb[step] && consec_left < 5) {
      food_delay = true;
      food_delay_timer = millis();
    }
    else {
      return;
    }
  }
}

/*
Extend levers
*/
void extend_lever(char side){
  if (side == 'r') {
    digitalWrite(right_lever_control, HIGH);
    digitalWrite(right_led, HIGH);
    //Serial.print("extending right");
  }
  if (side == 'l') {
    digitalWrite(left_lever_control, HIGH);
    digitalWrite(left_led, HIGH);
    //Serial.print("extending left");
  }
  //send_report();
  lever_out = true;
  //last_time = millis();
}

/*
Pull levers in, reset lever-related variables
*/
void retract_lever(char side){
  //Serial.println("retractintime");
  if (side == 'r') {
    digitalWrite(right_lever_control, LOW);
    digitalWrite(right_led, LOW);
  }
  if (side == 'l') {
    digitalWrite(left_lever_control, LOW);
    digitalWrite(left_led, LOW);
  }
  //Serial.print("retracting");
  //send_report();
  lever_out = false;
  no_single_lever_right = 0;
  no_single_lever_off_right = 0;
  no_single_lever_left = 0;
  no_single_lever_off_left = 0;
  completed_cycles_right = 0;
  completed_cycles_left = 0;
  lever_depressed_right = false;
  lever_depressed_left = false;
  //last_time = millis();
}

/*
The variable session_press is changed to signal to main loop that a lever has been pressed. It is then used in reward
calculation to specify which lever was pressed and so what percentage chance of reward should be used.
After completion, resetting of lever-related variables is done through the retract_lever function as it is cleaner to code it in there.
I noticed that certain levers will occasionally come out and immediately provide two or three low voltage signals,
which causes the lever presentation to finish prematurely. Not sure why this happens but some levers do it more 
than others. To remedy this, we use the same no_single_(signal) scheme used in ir_control, where a signal ON
(or additionally in this case, OFF) requires five repeated presentations before it is considered a true signal. 
*/
void read_lever(char side){
  //Serial.println("readlever");
  if (side == 'r') {
    sensorValue = analogRead(right_lever_report);
    float voltage = sensorValue * (5.0 / 1023.0);
    if ((voltage < 0.5) && (not lever_depressed_right)) {
      if (no_single_lever_right > 5) {
        lever_depressed_right = true;
        Serial.print("r_pr ");
        send_report();
        if (cycles_required_right[step] == -1) {
          Serial.print("r_on ");
          send_report();
          session_press = 1;
          consec_right += 1;
          consec_left = 0;
        }
        else if (completed_cycles_right == cycles_required_right[step]) {
          Serial.print("r_on ");
          send_report();
          session_press = 1;
          consec_right += 1;
          consec_left = 0;
        }
      }
      else {
        no_single_lever_right += 1;
        no_single_lever_off_right = 0;
      }
    }
    else if ((voltage > 0.5) && (lever_depressed_right)) {
      if (no_single_lever_off_right > 5) {
        completed_cycles_right += 1;
        //Serial.print("r_off");
        send_report();
        lever_depressed_right = false; 
      }
      else if (voltage > 0.5) {
      no_single_lever_right = 0;
      no_single_lever_off_right += 1;
      }
    }
  }
  else {
    sensorValue = analogRead(left_lever_report);
    float voltage = sensorValue * (5.0 / 1023.0);
    if ((voltage < 0.5) && (not lever_depressed_left)) {
      if (no_single_lever_left > 5) {
        lever_depressed_left = true;
        Serial.print("l_pr ");
        send_report();
        if (cycles_required_left[step] == -1) {
          Serial.print("l_on ");
          send_report();
          session_press = 1;
          consec_left += 1;
          consec_right = 0;
        }
        else if (completed_cycles_left == cycles_required_left[step]) {
          Serial.print("l_on ");
          send_report();
          session_press = 1;
          consec_left += 1;
          consec_right = 0;
        }
      }
      else {
        no_single_lever_left += 1;
        no_single_lever_off_left = 0;
      }
    }
    else if ((voltage > 0.5) && (lever_depressed_left)) {
      if (no_single_lever_off_left > 5) {
        completed_cycles_left += 1;
        //Serial.print("l_off");
        send_report();
        lever_depressed_left = false; 
      }
      else if (voltage > 0.5) {
      no_single_lever_left = 0;
      no_single_lever_off_left += 1;
      }
    }
  }
}

/*
Gives time since start of experiment in Min:Sec:Millis
*/
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
Vomits up as many variables as possible to assist in debugging
*/
void status_check() {
  Serial.print("consec_left/right = ");
  Serial.print(consec_left);
  Serial.println(consec_right);

  Serial.print("num_presses = ");
  Serial.println(num_presses);

  Serial.print("step = ");
  Serial.println(step);

  Serial.print("TimeSinceLastStep = ");
  Serial.println(TimeSinceLastStep);

  Serial.print("no_single_ir/no_single_ir_in = ");
  Serial.print(no_single_ir);
  Serial.println(no_single_ir_in);
  
  Serial.print("session_press = ");
  Serial.println(session_press);
  
  if (food_delay) {
    Serial.println("food_delay!");
  }
  else {
    Serial.println("no food delay");
  }

  Serial.print("food_delay_timer = ");
  Serial.println(food_delay_timer);

  Serial.print("dispense_time = ");
  Serial.println(dispense_time);

  Serial.print("start_time/last_time = ");
  Serial.print(start_time);
  Serial.println(last_time);

  Serial.print("left/right_lever_report = ");
  Serial.print(left_lever_report);
  Serial.println(right_lever_report);

  if (start_round) {
    Serial.println("start_round True!");    
  }
  else {
    Serial.println("start_round False!");
  }
  if (lever_out) {
    Serial.println("lever_out True");
  }
  else {
    Serial.println("lever_out False");
  }
}

/*
The code below allows us to upload an n-step protocol to the Arduino by filling a number of arrays representing different
variables (like which levers are presented, what each lever's reward probability is) with n elements. We first use get_size
to determine how many steps/elements are going into each array, and then fill arrays. Serial communication between the computer
and the Arduino can be tricky, so we continuously read back what we've recieved so that our main program can check it. 
Getting the protocol size from get_size is essential since we can't create arrays of a specific size on the fly; if we want
to read back the data in the arrays or check to see if we've gone through all the steps, we can't just iterate through all the
elements in the arrays. So we need to know exactly how many relevent elements there are.

Essentially don't change anything in get_size or fill_array because sending/recieving information with confidence is hard.
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
  fill_array(PressToAdvance);
  fill_array(TimeToAdvance);
  fill_array(cycles_required_right);
  fill_array(cycles_required_left);
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
