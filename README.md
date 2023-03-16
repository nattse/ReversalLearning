# Reversal Learning/Conditioning

The objective of this code repository is to give orders to and recieve information from multiple instruments in order to test rodent cue-reward associations in a dynamic manner.

## First time setup
These instructions assume you have downloaded this repository, the necessary packages, and the Arduino IDE. 
Connect the Arduino and use the IDE to upload *combined_recieve_and_execute.ino* to the Arduino. At this point also note the port the Arduino is on (e.g. COM**2**), as you will need to manually enter this into your config file. Plug in the USB camera next and use `v4l2-ctl --list-devices` to get the device number (e.g. /dev/video**2**).

## Before each run
All experiment conditions are set beforehand in the *config.csv* file. Each column represents the conditions for that stage of the experiment, and an experiment can contain as many stages as desired, each with their own unique conditions. The procedure in any stage is as follows:
- Video recording starts
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

**Duration** | any positive integer or -1 | Duration of time (ms) after which we advance to the next stage. If using Number of Presses, set to -1

**Arduino ID Port** | any integer | Also can be found using `sudo dmesg | tail`. Only need to enter this in the first column

**Cam ID** | any integer | `v4l2-ctl --list-devices`. Only need to enter this in the first column

____________________________________________________________________________________________________________________________

An example config.csv would be as follows:

|  | Stage 1 | Stage 2 |
| :---         |     :---:      |          ---: |
| Right Lever Out  |1|1|
| Right Lever Reward % |100|50|
| Left Lever Out |1|1|
| Left Lever Reward % |100|75|
| Switch stage at: |||
| Number of presses |-1|30|
| Duration |600000|-1|
| Arduino ID Port |0||
| Cam ID |2||

Using this configuration, we would start at Stage 1 with both levers being presented and rewarded 100% of the time. This would last for 600 seconds, after which we switch to Stage 2. In this stage both levers are still presented, but the reward probability of the right lever drops to 50%, and the left lever drops to 75%. After 30 lever presses, all processes are stopped and the video recording ends. 

## Running the experiment
Using Terminal, move to where these files are stored using `cd`. Run `python3 working_send_and_recieve.py filename` replacing filename with whatever you want the resulting .csv and video files to be titled. Output should give times when nose is in food magazine, when levers are pressed, and whenever the stage changes. 
