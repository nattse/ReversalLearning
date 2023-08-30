import re
import serial
import numpy as np
import pandas as pd

def packing(cargo):
    prepped = '<' + str(cargo) + '>'
    shipped = bytes(prepped, encoding='utf8')
    return(shipped)

def send_and_check(ser, plans):
    while True:
        back = ser.readline().decode().strip()
        if back == 'ready':
            print(back)
            break
    ser.write(packing(plans[0]))
    while True:
        back = ser.readline().decode().strip()
        if len(back) > 0:
            print(back)
        if 'recieved size' in back:
            size = re.search('size: (.+)', back)[1]
            if int(size) != plans[0]:
                raise Exception(f'Arduino got plan size {size} while we sent it size {len(plans[0])}')
            break
    for plan in plans[1:]:
        send_check_plan(ser, plan)

def send_check_plan(ser, plan_list):
    for num in plan_list:
        ser.write(packing(num))
    ser.write(packing('f'))
    feedback = []
    incoming = False
    while True:
        back = ser.readline().decode().strip()
        if incoming == False:
            if back != 'feedback':
                continue
            else:
                incoming = True
        else:
            if len(back) > 0:
                if 'done' in back:
                    if feedback != plan_list:
                        raise Exception('The time plan we sent was not the same as the one received back')
                    break
                else:
                    feedback.append(int(back))

def check_config_vars(config):
    rows, cols = config.shape
    static = np.where(config.index == 'System:')[0][0]
    var_names = [i for i in config.index[:static] if type(i) == str]
    var_names = [i for i in var_names if i not in ['Settings', 
                                                   'Switch stage at:']]        # If more intentionally blank lines are added, they must be included here
    for var in var_names:
        row_types = pd.isnull(config.loc[var])
        if any(row_types):
            raise Exception(f'Detected missing variable setting in config row: {var} \nNothing has been run yet, please fix and try again')

"""Some variables are not changed often, so to simplify the config file, we allow
these variables to be set once. However, we do still want the option to change these
variables. To accommodate both options, we set the Arduino code to expect a value for each 
step (identical to all of the other variables) while using this current function 
to handle cases where we only set the variable once (in which case we duplicate
that single value to match the number of steps) and cases where we have manually 
entered a value for each step (in which case we just pass that whole list along).
Either set once, or set for ALL steps.
"""
def check_config_advanced(config):
    return_dict = {}
    rows, cols = config.shape
    static = np.where(config.index == 'Advanced:')[0][0]
    var_names = [i for i in config.index[static+1:] if 'ID' not in i]
    for var in var_names:
        row_types = pd.isnull(config.loc[var])
        if not row_types[0] and all(row_types[1:]):                            # First column is a valid number, all others are blank                   
            return_dict[var] = cols * [int(config.loc[var].values[0])]
        elif all(row_types):
            raise Exception(f'All values missing in config row: {var}\nNothing has been run yet, please fix and try again')
        elif any(row_types):
            raise Exception(f'Error in config row: {var}. Either set once, or set for EVERY step. \nNothing has been run yet, please fix and try again')
        else:
            return_dict[var] = [int(i) for i in config.loc[var].values]
    return return_dict
        