# Overview
Behavioral tasks such as sign tracking and reversal learning are complex both in the information they provide and in the methodology/protocols they can require. As such, we wanted to gain better control over data collection (e.g. sampling frequency, video quality and framerate) while at the same time making the application of protocols more human-readable. In order to accomplish these tasks, we have created our own operant conditioning chambers. Protocol instructions and instrument readings are routed through an Arduino to a single laptop that time-syncs the data with high-resolution video capture. 

This specific code repository is for a reversal learning task.

[Building the system](#building-the-system)

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

**Filepath** | any directory | Output data location

**Lever Timeout** | any integer; default = -1 | How many seconds the lever will stay out once presented. Once this time is up, levers will retract and no reward will be dispensed. Set to -1 to not use this feature

**IR Timeout** | any integer; default = 3 | After reward is calculated, breaking the IR beam in the food bin will not trigger the levers for this number of seconds
____________________________________________________________________________________________________________________________

## Output 

Read .txt file in python using `df = pd.read_csv(file, sep = '\t', skiprows=3, header = None)` where file is the filename.

**nose_in/nose_out 00:00:000** > nose entry/exit to food bin

**r_pr/l_pr 00:00:000** > right/left lever press that are sub-threshold (< **presses to trigger**) and didn't trigger lever retraction/reward calculation

**r_on/l_on 00:00:000** > right/left lever press that reached threshold (== **presses to trigger**) and triggered lever retraction and reward calculation

**rewarded** > denotes whether a reward was dispensed following an r_on/l_on event

**next step 00:00:000** > denotes the step has been incremented

**complete end** > end of protocol

# Building The System

These instructions assume you have downloaded this repository, the necessary packages, and the Arduino IDE. 

## Wiring

Any wiring system that matches the schematic below can be used, but we have provided instructions for replicating our setup below.

<p align="center">
 
![operant_chamber_wiring](https://user-images.githubusercontent.com/118491380/227418044-cb065a87-e8b8-4a8a-904e-f67036c5ebf5.png)
<p align="center">
Wiring schematic
</p>
 
We considered components inside our light-attenuating cabinets to be peripheral components, and those outside the cabinets to be central components (given their proximity to the laptop on which everything was run). 

Peripheral components: Left/right lever, food dispenser, IR emitter, IR receiver, right/left LED, camera

Central components: Arduino, relays, 28v power source

To save space, we wired as many central components as possible directly on an Arduino Proto Shield, grouped and identified by matching colors in the images below:

<p align="center">
<img src="https://github.com/nattse/ReversalLearning/blob/main/docs/operant_chamber_wiring_color_coded.png" height="400"> <img src="https://github.com/nattse/ReversalLearning/blob/main/docs/shield_fully_wired_color_coded.jpg" height="400">
<p align="center">
Lever reading (yellow), LED control (blue), IR nose detection (green)
</p>

Once the Proto Shield wiring was complete, the other central components were connected, illustrated in the image below. The 28v source was fed into a six-way split. Three pairs of channels were reserved to power each lever and food dispenser (1). The remaining three positive channels were fed through a Normally Open relay (2) and were used control each lever and food dispenser via their signal wires (3). Remaining connections to peripheral components were made through a terminal block (4).

<p align="center">
<img src="https://github.com/nattse/ReversalLearning/blob/main/docs/completed_setups.jpg" height="400"> 
</p>

## Hardware

Camera: **[ELP-USBFHD05MT-KL36IR](https://www.amazon.com/ELP-100fps-Infrared-Security-Housing/dp/B0BHW95L37)**

Levers: **Med Associates ENV-312-3 Retractable Mouse Lever**

Food Dispenser: **Med Associates ENV-203 Pellet Dispenser**

Relays: **[HiLetgo 5V One Channel Relay Module With optocoupler Support High or Low Level Trigger](https://www.amazon.com/HiLetgo-Channel-optocoupler-Support-Trigger/dp/B00LW15A4W)**

IR Emitter and Receiver: **[Chanzon 5mm IR Infrared LED Diode Emitter + Receiver](https://www.amazon.com/Emitter-Receiver-VS1838B-Infrared-Raspberry/dp/B07TLBJR5J?th=1)**

Food Receptacle: **[Med Associates ENV-303W Trough Pellet Receptacle](https://med-associates.com/product/trough-pellet-receptacle/)**

Food Receptacle Cover: Download and 3D print using feeder_cover.stl file. Designed by me

<p align="center">
<img src="https://github.com/nattse/ReversalLearning/blob/main/docs/alone.jpg" width="300"> <img src="https://github.com/nattse/ReversalLearning/blob/main/docs/sep.jpg" width="300"> <img src="https://github.com/nattse/ReversalLearning/blob/main/docs/combined.jpg" width="300">
</p>


1. Orient the food receptical cover so that the two circular holes are closest to the mouth of the food receptical, then align the four grooves on the inside of the food receptical cover with the four screws on the sides of the food receptical, then push down until fully seated. Insert IR emitter and receiver into circular holes (sides do not matter). Fix in place with small amount of hot glue or tape if needed. Then levers, food dispenser, and food receptacle should be set up according to your chamber's design. [Calibrate IR beam break system](#ir-nose-detection-calibration).
2. Fix left/right LEDs above the respective levers using hot glue or tape. Check that lever LEDs illuminate the correct lever by setting a config.xlsx to extend one lever at a time.

 


<p align="center">
<img src="https://github.com/nattse/ReversalLearning/blob/main/docs/chamber_image.jpeg" width="300">
 <p align="center">
  Camera placed above chamber
</p>

# IR nose detection calibration

The only thing in the code that needs to be changed is in `measure_ir()` in *combined_recieve_and_execute.ino*
The irValue is compared to two different numbers at these four lines: 

`if ((irValue < 300) and (ir_broken == false)) {`

`if (irValue < 300) {`

`if ((irValue > 700) and (ir_broken == true)) {`

`if (irValue > 700) {`

If irValue is below the low number, the beam is considered broken. If it higher than the large number, the beam is considered not broken.
The baseline irValue is different for every setup, and the appropriate threshold values vary as well. Use *send_recieve_dummy.ino* to get a readout of your setup's irValue. Stick your finger in the food bin and see what the values drop to, and set this as the lower threshold (replace `300` in `if ((irValue < 300) and (ir_broken == false)) {` and `if (irValue < 300) {` with your new number). While your finger is in there, notice how much the values can fluctuate. You want to set your higher threshold to be significantly greater than the largest fluctuation. So if baseline readings are at 100, then when you stick your finger in, the irValue bounces between 0 and 11, replace `700` in the above code with 30. 

# Example calibration

<p align="center">
<img src="https://github.com/nattse/ReversalLearning/blob/main/docs/baseline.jpg" width="700">
<p align="center">
Baseline ~994 
</p>

<p align="center">
<img src="https://github.com/nattse/ReversalLearning/blob/main/docs/nose_in.jpg" width="700">
<p align="center">
Upon nose poke, signal value drops to <200
</p>

<p align="center">
<img src="https://github.com/nattse/ReversalLearning/blob/main/docs/nose_out.jpg" width="700">
<p align="center">
Upon withdrawl, signal value returns to baseline
</p>

<p align="center">
<img src="https://github.com/nattse/ReversalLearning/blob/main/docs/thresholding.jpeg" width="700">
<p align="center">
Acceptable threshold values at 800 and 200
</p>

Result:

`if ((irValue < 800) and (ir_broken == false)) {`

`if (irValue < 800) {`

`if ((irValue > 200) and (ir_broken == true)) {`

`if (irValue > 200) {`
