import re
import serial

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



""" 
    ser.write(packing(len(time_plan)))
    while True:
        back = ser.readline().decode().strip()
        if len(back) > 0:
            print(back)
        if 'recieved size' in back:
            size = re.search('size: (.+)', back)[1]
            if int(size) != len(time_plan):
                raise Exception(f'Arduino got plan size {size} while we sent it size {len(time_plan)}')
            break
    for num in time_plan:
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
                    if feedback != time_plan:
                        raise Exception('The time plan we sent was not the same as the one received back')
                    break
                else:
                    feedback.append(int(back))
    for num in side_plan:
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
                    if feedback[0:len(side_plan)] != side_plan:
                        raise Exception('The time plan we sent was not the same as the one received back')
                    break
                else:
                    feedback.append(int(back))
    ser.close()
"""

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