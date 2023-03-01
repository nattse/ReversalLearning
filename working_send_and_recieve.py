import datetime
import serial
import cv2
import time
import pandas as pd
import numpy as np
import threading
import sys
import os
import subprocess
import signal
import atexit
import random
import configparser
import warnings
import utils
import re
import matplotlib.pyplot as plt
instructions = pd.read_csv('config.csv', index_col = 0)
right_out = instructions.loc['Right Lever Out'].values
right_out = [1 if i == 'yes' else 0 for i in right_out]
right_prob = instructions.loc['Right Lever Reward %'].values
right_prob = [int(i) for i in right_prob]
left_out = instructions.loc['Left Lever Out'].values
left_out = [1 if i == 'yes' else 0 for i in left_out]
left_prob = instructions.loc['Left Lever Reward %'].values
left_prob = [int(i) for i in left_prob]
advance_press = instructions.loc['Number of presses'].values
advance_press = [int(i) for i in advance_press]
advance_duration = instructions.loc['Duration'].values
advance_duration = [int(i) for i in advance_duration]
sections = instructions.shape[1]
plan_details = [sections,
                right_out,
                right_prob,
                left_out,
                left_prob,
                advance_press,
                advance_duration]
for a in plan_details:
    print(a)
ard_number = instructions.loc['Arduino ID Port'].iloc[0]
camera_choice = instructions.loc['Cam ID'].iloc[0]
def arduino_setup():
    global r_t_s
    global program_done
    ser = serial.Serial(f'/dev/ttyACM{ard_number}', 9600, timeout=0.5)
    utils.send_and_check(ser, plan_details)
    readouts = []
    while True:
        reports = ser.readline().decode().strip()
        if len(reports) > 1:
            if reports == 'entering loop':
                r_t_s = True
            print(reports)
            readouts.append(reports)
            if reports == "complete end":
                program_done = True
                break
    ser.close()
    readouts = pd.DataFrame(readouts)
    readouts.to_csv(sys.argv[1] + '.csv')

x = threading.Thread(target=arduino_setup, args=()).start() #Runs the arduino protocol
exp_settings = 'v4l2-ctl -d /dev/video{} --set-ctrl=exposure_absolute=300 --set-ctrl=exposure_auto=1'.format(camera_choice) #We turn off auto-exposure and peg the exposure time to 300
camset = subprocess.Popen(exp_settings.split()) #This runs the exposure setting command in terminal
r_t_s = False
program_done = False
#GStreamer code - waits for the Arduino and then immediately starts video capture outside of python (to cut down on the amount of processing we have to do on frames)
while r_t_s == False:
    pass
gstr_arg = 'gst-launch-1.0 v4l2src device=/dev/video{} num-buffers=-1 do-timestamp=true ! image/jpeg,width=3840,height=2160,framerate=30/1 ! queue ! avimux ! filesink location={}.avi -e'.format(camera_choice,sys.argv[1])
print(gstr_arg)
print('\n')
gstr_cmd = gstr_arg.split()
video_capture = subprocess.Popen(gstr_cmd)
vid_id = video_capture.pid
while program_done == False:
    pass
os.kill(vid_id,signal.SIGINT)