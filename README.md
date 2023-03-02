# Reversal Learning/Conditioning

The objective of this code repository is to give orders to and recieve information from multiple instruments in order to test rodent cue-reward associations in a dynamic manner.

## First time setup
These instructions assume you have downloaded this repository, the necessary packages, and the Arduino IDE. 
Connect the Arduino and use the IDE to upload *combined_recieve_and_execute.ino* to the Arduino. At this point also note the port the Arduino is on (e.g. COM**2**), as you will need to manually enter this into your config file. Plug in the USB camera next and use `v4l2-ctl --list-devices` to get the device number (e.g. /dev/video**2**).

## Before each run
All experiment conditions are set beforehand in the *config.csv* file. Each column represents the conditions for that stage of the experiment, and an experiment can contain as many stages as desired, each with their own unique conditions.
