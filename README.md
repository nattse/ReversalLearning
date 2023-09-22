# Overview
Behavioral tasks such as sign tracking and reversal learning are complex both in the information they provide and in the methodology/protocols they can require. As such, we wanted to gain better control over data collection (e.g. sampling frequency, video quality and framerate) while at the same time making the application of protocols more human-readable. In order to accomplish these tasks, we have created our own operant conditioning chambers. Protocol instructions and instrument readings are routed through an Arduino to a single laptop that time-syncs the data with high-resolution video capture. 

This specific code repository is for a reversal learning task.

## First time setup
These instructions assume you have downloaded this repository, the necessary packages, and the Arduino IDE. 

The only thing in the code that needs to be changed is in `measure_ir()` in combined_recieve_and_execute.ino
The irValue is compared to two different numbers at these four lines: 

`if ((irValue < 300) and (ir_broken == false)) {`

`if (irValue < 300) {`

`if ((irValue > 700) and (ir_broken == true)) {`

`if (irValue > 700) {`

If irValue is below the low number, the beam is considered broken. If it higher than the large number, the beam is considered not broken.
The baseline irValue is different for every setup, and the appropriate threshold values vary as well. Use send_recieve_dummy.ino to get a readout of your setup's irValue. Stick your finger in the food bin and see what the values drop to, and set this as the lower threshold (replace `300` in `if ((irValue < 300) and (ir_broken == false)) {` and `if (irValue < 300) {` with your new number). While your finger is in there, notice how much the values can fluctuate. You want to set your higher threshold to be significantly greater than the largest fluctuation. So if baseline readings are at 100, then when you stick your finger in, the irValue bounces between 0 and 11, replace `700` in the above code with 30. 

## Basic procedure employed

Once video recording has begun, the procedure is as follows:
- Nose poke into the food receptacle begins the trial
- Levers are presented
- Upon lever press, all levers are retracted
- Food pellet is dispensed depending on the pressed lever's probability of reward and whether the limit on consecutive lever presses has been hit
- New trial initiation is blocked for three seconds after reward is dispensed
- Check to see if ending conditions have been met; if not, wait for nose poke to begin another trial

## Before each run

### Checking Camera ID and Arduino ID
Connect the Arduino and use the IDE to upload *combined_recieve_and_execute.ino* to the Arduino. At this point also note the port the Arduino is on (e.g. COM**2**), and check that this matches the Arduino designated in the config file. Plug in the USB camera next and use `v4l2-ctl --list-devices` to get the device number (e.g. /dev/video**2**).

## Running the experiment
Using Terminal, move to where *working_send_and_recieve.py* is located stored using `cd`. Run `python3 working_send_and_recieve.py config_file filename` replacing config_file with either *training_1, training_2, reversal, random*, depending on the protocol you want to use. Replace filename with whatever you want the resulting .txt and video files to be titled. 

Config files that correspond to the different protocols are stored in the ./configs directory. By default, the *training_1, training_2, reversal, random* configs should not be altered except to enter a new Arduino ID or Camera ID if they change.

Custom config files can be created to alter specific settings, and settings can be made to automatically change throughout a single experiment by adding on additional stages. These additional stages can be entered into the custom config file by adding new columns of settings to the right of the original settings.

### Config settings 

**Right/Left Lever Out** | yes/no/random | Determines whether the right or left lever is presented. See random for random

**Right/Left Lever Reward %** | 0-100 | Determines the probability of a food reward being dispensed for that lever. See random for how reward payouts are handled during random rounds

**Presses to trigger right/left lever** | any positive integer | How many times a lever needs to be pressed in order to "count"; levers will not retract and reward will not be calculated until this threshold number of presses is reached

**Max rewarded consecutive presses** | any positive integer | Number of consecutive presses after which reward for the over-pressed lever will be ceased until other lever is pressed

**Number of presses** | any positive integer or -1 | Total number of lever presses needed to advance to the next stage. (Set to -1 if you only want to consider **Duration**)

**Duration** | any positive integer or -1 | Duration of time (seconds) needed to advance to the next stage. (Set to -1 if you only want to consider **Number of presses**)

**Arduino ID Port** | any integer | Also can be found using `sudo dmesg | tail`. Only need to enter this in the first column

**Cam ID** | any integer | `v4l2-ctl --list-devices`. Only need to enter this in the first column

**Lever Timeout** | any integer; default = -1 | How many seconds the lever will stay out once presented. Once this time is up, levers will retract and no reward will be dispensed. Set to -1 to not use this feature

**IR Timeout** | any integer; default = 3 | After reward is calculated, breaking the IR beam in the food bin will not trigger the levers for this number of seconds
____________________________________________________________________________________________________________________________

## Output 

Read .txt file using `df = pd.read_csv(file, sep = '\t', skiprows=3, header = None)` where file is the filename.

**nose_in/nose_out 00:00:000** > nose entry/exit to food bin

**r_pr/l_pr 00:00:000** > right/left lever press that are sub-threshold and didn't trigger lever retraction/reward calculation

**r_on/l_on 00:00:000** > right/left lever press that reached threshold and triggered lever retraction and reward calculation

**rewarded** > denotes whether a reward was dispensed followng an r_on/l_on event

**next step 00:00:000** > denotes the step has been incremented

**complete end** > end of protocol

# Operant Conditioning Chamber Wiring Diagram

Levers: **Med Associates ENV-312-3 Retractable Mouse Lever**

Food Dispenser: **Med Associates ENV-203 Pellet Dispenser**

Relays: **[HiLetgo 5V One Channel Relay Module With optocoupler Support High or Low Level Trigger](https://www.amazon.com/HiLetgo-Channel-optocoupler-Support-Trigger/dp/B00LW15A4W)**

IR Emitter and Receiver: **[Chanzon 5mm IR Infrared LED Diode Emitter + Receiver](https://www.amazon.com/Emitter-Receiver-VS1838B-Infrared-Raspberry/dp/B07TLBJR5J?th=1)**

![operant_chamber_wiring](https://user-images.githubusercontent.com/118491380/227418044-cb065a87-e8b8-4a8a-904e-f67036c5ebf5.png)

![chamber_image](https://user-images.githubusercontent.com/118491380/227365957-fa8b2439-1884-4f26-b954-e5c664bc3012.jpg)

