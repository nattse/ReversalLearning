# Reversal Learning/Conditioning

The objective of this code repository is to give orders to and recieve information from multiple instruments in order to test rodent cue-reward associations in a dynamic manner.

## First time setup
These instructions assume you have downloaded this repository, the necessary packages, and the Arduino IDE. 
Connect the Arduino and use the IDE to upload *combined_recieve_and_execute.ino* to the Arduino. At this point also note the port the Arduino is on (e.g. COM**2**), as you will need to manually enter this into your config file. Plug in the USB camera next and use `v4l2-ctl --list-devices` to get the device number (e.g. /dev/video**2**).

## Before each run
All experiment conditions are set beforehand in the *config.csv* file. Each column represents the conditions for that stage of the experiment, and an experiment can contain as many stages as desired, each with their own unique conditions. The backbone of the experiment does not change, however. The flow is as follows:
- Video and timer start
- Nose poke into the food recepticle begins the trial
- Levers are presented
- Upon lever press, all levers are retracted
- Food pellet is dispensed depending on the pressed lever's probability of reward
- Check to see if conditions to proceed to next stage are satisfied, and if so, change conditions according to the next stage
- New trial initiation is blocked for three seconds after reward is dispensed (*need to consider that a mouse may be slow to retrieve the food at first and so may trigger a new trial upon retrieval*)

### Using config.csv

**Right Lever Out** | yes/no | Determines whether the right lever is presented 

**Right Lever Reward %** | 0-100 | Determines the probability of a food reward being dispensed

**Left Lever Out** | yes/no | Same as Right Lever Out

**Left Lever Reward %** | 0-100 | Same as Right Lever Reward

**Switch stage at** | Unused row 

**Number of presses** | any positive integer or -1 | Total number of lever presses needed to advance to the next stage. If using Duration instead, set to -1

**Duration** | any positive integer or -1 | Duration of time (ms) after which we advance to the next stage. If using Number of Presses, set to -1


