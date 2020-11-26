import os, serial,time
import numpy as np

def str_process(str):
    str_data = str.split('\n')
    # print(str_data)
    for i in str_data:
        if 'state' in i:
            data_type = "state"
            data = i.split(' ')
            data.pop(0)
            # data.pop(-1)
            data = [float(i.strip(",")) for i in data]
            # print(data)
            return data,data_type

        elif 'input' in i:
            data_type = "input"
            data = i.split(' ')
            data.pop(0)
            # data.pop(-1)
            # print(data)
            data = [float(i) for i in data]
            return data,data_type
        else:
            print("wrong input\n")

if __name__ == "__main__":
    state_x0 = []
    state_x1 = []
    state_x2 = []
    input = []
    data_counter = 0
    # receive_data = "state: 12.34 45.67 15.6 \n\r"
    # receive_data,data_type = str_process(receive_data)
    # state_x0.append(receive_data[0])
    # state_x1.append(receive_data[1])
    # state_x2.append(receive_data[2])


    sys_name = os.name
    print("system name is:" + sys_name)
    if sys_name is 'nt':
        ser_port = 'com3'
    elif  sys_name is 'posix':
        ser_port = '/dev/ttyACM0'
    else:
        print("system name Error")
    ser = serial.Serial()
    ser.port = ser_port
    ser.baudrate = 115200
    ser.bytesize = serial.EIGHTBITS
    ser.stopbits = serial.STOPBITS_ONE
    
    try:
        ser.open()
    except serial.SerialException:
        print("serial exception")
    
    if ser.isOpen():
        print("seirial opened")
        while(1):
            # x = ser.readline()
            x = ser.read_until(b'\n',size=None)
            x_str = x.decode("utf-8")
            # print(x_str)
            if(("state" in x_str ) or ("input" in x_str )):
                data, data_type = str_process(x_str)
                if data_type is "state":
                    state_x0.append(data[0])
                    state_x1.append(data[1])
                    state_x2.append(data[2])
                elif data_type is "input":
                    input.append(data[0])
                    # print(data[0])
                    data_counter += 1
                    if data_counter == 100:
                        break
                else:
                    print("received data wrong")
        ser.close()
    else:
        print("cannnot open serial port")

    print(input,state_x0)
    np.save("../data/input.npy",input)
    np.save("../data/state0.npy",state_x0)
    np.save("../data/state1.npy",state_x1)
    np.save("../data/state2.npy",state_x2)