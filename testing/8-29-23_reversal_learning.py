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
instructions = pd.read_excel('config.xlsx', index_col = 0)
utils.check_config_vars(instructions)

steps = instructions.shape[1]
right_out = instructions.loc['Right Lever Out'].values
right_out = [1 if i == 'yes' else 0 for i in right_out]
right_prob = instructions.loc['Right Lever Reward %'].values
right_prob = [int(i) for i in right_prob]
left_out = instructions.loc['Left Lever Out'].values
left_out = [1 if i == 'yes' else 0 for i in left_out]
left_prob = instructions.loc['Left Lever Reward %'].values
left_prob = [int(i) for i in left_prob]
advance_press = instructions.loc['Number of presses'].values
advance_press = [int(i) - 1 for i in advance_press]
advance_duration = instructions.loc['Duration'].values
advance_duration = [int(i) for i in advance_duration]
cycles_required_right = [int(i) - 1 for i in instructions.loc['Presses to trigger right lever'].values]
cycles_required_left = [int(i) - 1 for i in instructions.loc['Presses to trigger left lever'].values]
max_presses = [int(i) + 1 for i in instructions.loc['Max rewarded consecutive presses'].values]

lever_timeout = utils.check_config_advanced(instructions)['Lever Timeout']
ir_timeout = utils.check_config_advanced(instructions)['IR Timeout']

plan_details = [steps,
                right_out,
                right_prob,
                left_out,
                left_prob,
                advance_press,
                advance_duration,
                cycles_required_right,
                cycles_required_left,
                max_presses,
                lever_timeout,
                ir_timeout]
for a in plan_details:
    print(a)
ard_number = instructions.loc['Arduino ID Port'].iloc[0]
camera_choice = instructions.loc['Cam ID'].iloc[0]
filename = os.path.join(instructions.loc['Filepath'][0], sys.argv[1])
log = open(filename + '.txt', 'w')
ser = serial.Serial(f'/dev/ttyACM{ard_number}', 9600, timeout=0.5)
def arduino_setup():
    global r_t_s
    global program_done
    log.write(sys.argv[1])
    log.write(f'Experiment began: {datetime.datetime.now().strftime("%d/%m/%Y %H:%M:%S")}\n')
    #ser = serial.Serial(f'/dev/cu.usbmodem11201', 9600, timeout=0.5)
    utils.send_and_check(ser, plan_details)
    log.write(f'Experiment plan succesfully transfered to Arduino:\n{plan_details}\n')
    readouts = []
    while True:
        reports = ser.readline().decode().strip()
        if len(reports) > 1:
            if reports == 'entering loop':
                r_t_s = True
            print(reports)
            log.write(f'{reports}\n')
            if reports == "complete end":
                program_done = True
                break
 
    
try:
    x = threading.Thread(target=arduino_setup, args=()).start() #Runs the arduino protocol

    # We turn off auto-exposure and peg the exposure time to 300
    # If using IR camera, use exposure time = 100
    exp_settings = f'v4l2-ctl -d /dev/video{camera_choice} --set-ctrl=exposure_time_absolute=100 --set-ctrl=auto_exposure=1'
    camset = subprocess.Popen(exp_settings.split()) #This runs the exposure setting command in terminal
    r_t_s = False
    program_done = False
    #GStreamer code - waits for the Arduino and then immediately starts video capture outside of python (to cut down on the amount of processing we have to do on frames)
    while r_t_s == False:
        pass
    # Full color camera
    #gstr_arg = 'gst-launch-1.0 v4l2src device=/dev/video{} num-buffers=-1 do-timestamp=true ! image/jpeg,width=3840,height=2160,framerate=30/1 ! queue ! avimux ! filesink location={}.avi -e'.format(camera_choice,sys.argv[1])

    # IR camera
    gstr_arg = f'gst-launch-1.0 v4l2src device=/dev/video{camera_choice} num-buffers=-1 do-timestamp=true ! image/jpeg,width=1920,height=1080,framerate=30/1 ! queue ! avimux ! filesink location={filename + ".avi"} -e'
    print(gstr_arg)
    print('\n')
    gstr_cmd = gstr_arg.split()
    video_capture = subprocess.Popen(gstr_cmd)
    vid_id = video_capture.pid
    while program_done == False:
        pass
    os.kill(vid_id,signal.SIGINT)
    sys.exit('Ended via completion of Arduino protocol')
except KeyboardInterrupt: 
    log.close()
    ser.close()
    sys.exit('Ended via KeyboardInterrupt')
