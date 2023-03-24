# Overview
Behavioral tasks such as sign tracking and reversal learning are complex both in the information they provide and in the methodology/protocols they can require. As such, we wanted to gain better control over data collection (e.g. sampling frequency, video quality and framerate) while at the same time making the application of protocols more human-readable. In order to accomplish these tasks, we have created our own operant conditioning chambers. Protocol instructions and instrument readings are routed through an Arduino to a single laptop that time-syncs the data with high-resolution video capture. 

This specific code repository is for a reversal learning task.

## First time setup
These instructions assume you have downloaded this repository, the necessary packages, and the Arduino IDE. 
Connect the Arduino and use the IDE to upload *combined_recieve_and_execute.ino* to the Arduino. At this point also note the port the Arduino is on (e.g. COM**2**), as you will need to manually enter this into your config file. Plug in the USB camera next and use `v4l2-ctl --list-devices` to get the device number (e.g. /dev/video**2**).

## Before each run
All experiment conditions are set beforehand in the *config.csv* file. Each column represents the conditions for that stage of the experiment, and an experiment can contain as many stages as desired, each with their own unique conditions. Once video recording has begun, the procedure in any stage is as follows:
- Nose poke into the food receptacle begins the trial
- Levers are presented
- Upon lever press, all levers are retracted
- Food pellet is dispensed depending on the pressed lever's probability of reward (or if that same lever has been pressed 5 or more times in a row; see function reward_calculation() in combined_recieve_and_execute.ino to turn off)
- New trial initiation is blocked for three seconds after reward is dispensed (*need to consider that a mouse may be slow to retrieve the food at first and so may trigger a new trial upon retrieval - something we'll need to work out through trial and error*)
- Check to see if ready to move to next stage, and if so, change conditions (e.g. levers presented/reward probability) according to the next stage. Otherwise, wait for nose poke to begin another trial

### Using config.csv

**Right Lever Out** | yes/no | Determines whether the right lever is presented 

**Right Lever Reward %** | 0-100 | Determines the probability of a food reward being dispensed

**Left Lever Out** | yes/no | Same as Right Lever Out

**Left Lever Reward %** | 0-100 | Same as Right Lever Reward

**Switch stage at** | Unused row 

**Number of presses** | any positive integer or -1 | Total number of lever presses needed to advance to the next stage. If using Duration instead, set to -1

**Duration** | any positive integer or -1 | Duration of time (seconds) after which we advance to the next stage. If using Number of Presses, set to -1

**Arduino ID Port** | any integer | Also can be found using `sudo dmesg | tail`. Only need to enter this in the first column

**Cam ID** | any integer | `v4l2-ctl --list-devices`. Only need to enter this in the first column

____________________________________________________________________________________________________________________________

An example config.csv would be as follows:

|  | Stage 1 | Stage 2 |
| :---         |     :---:      |          ---: |
| Right Lever Out  |yes|yes|
| Right Lever Reward % |100|50|
| Left Lever Out |yes|yes|
| Left Lever Reward % |100|75|
| Switch stage at: |||
| Number of presses |-1|30|
| Duration |600|-1|
| Arduino ID Port |0||
| Cam ID |2||

Using this configuration, we would start at Stage 1 with both levers being presented and rewarded 100% of the time. This would last for 600 seconds, after which we switch to Stage 2. In this stage both levers are still presented, but the reward probability of the right lever drops to 50%, and the left lever drops to 75%. After 30 lever presses, all processes are stopped and the video recording ends. 

## Running the experiment
Using Terminal, move to where these files are stored using `cd`. Run `python3 working_send_and_recieve.py filename` replacing filename with whatever you want the resulting .csv and video files to be titled. Output should give times when nose is in food magazine, when levers are pressed, and whenever the stage changes. 

# Operant Conditioning Chamber Wiring Diagram

Levers: **Med Associates ENV-312-3 Retractable Mouse Lever**

Food Dispenser: **Med Associates ENV-203 Pellet Dispenser**

Relays: **[HiLetgo 5V One Channel Relay Module With optocoupler Support High or Low Level Trigger](https://www.amazon.com/HiLetgo-Channel-optocoupler-Support-Trigger/dp/B00LW15A4W)**

IR Emitter and Receiver: **[Chanzon 5mm IR Infrared LED Diode Emitter + Receiver](https://www.amazon.com/Emitter-Receiver-VS1838B-Infrared-Raspberry/dp/B07TLBJR5J?th=1)**

![operant_chamber_wiring](https://user-images.githubusercontent.com/118491380/227418044-cb065a87-e8b8-4a8a-904e-f67036c5ebf5.png)

![chamber_image](https://user-images.githubusercontent.com/118491380/227365957-fa8b2439-1884-4f26-b954-e5c664bc3012.jpg)

