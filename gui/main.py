import serial
import os
import sys
import time
import tkinter as tk
from tkinter import *
from tkinter.ttk import *
from tkinter.filedialog import * 
from time import sleep

from serial.tools.list_ports import comports

def serial_list_ports():
    ports = []
    for n, (port, desc, hwid) in enumerate(sorted(comports()), 1):
        ports.append(port)
    return ports

def serial_connect():
    global port_list
    global ser
    ser.port = port_list.get()
    try:
        ser.open()
    except serial.SerialException as e:
        print(e)
        return 0
    print("Connected to " + port_list.get())
    button_connect['text'] = "Disconnect"
    button_connect['command'] = serial_disconnect

    # Update GUI info
    time.sleep(0.25)
    serial_read()
    device_get_info()
    flash_get_info()
    file_get_list()
    return 1

def serial_disconnect():
    global ser
    global file_list
    try:
        ser.close()
    except serial.SerialException as e:
        print(e)
        return 0
    print("Disconnected from " + ser.portstr)
    button_connect['text'] = "Connect"
    button_connect['command'] = serial_connect
    file_list.delete(0, tk.END)
    flash_capacity_label['text'] = "Flash Capacity"
    flash_capacity['value'] = 0

    return 1

def serial_refresh():
    global port_list
    ports = serial_list_ports()
    port_list=ttk.Combobox(usb64_window, values = ports)
    port_list.grid(row=0, column=0, sticky=NSEW, columnspan = 2)
    port_list.SelectedItem = port_list.current(0)

def serial_read():
    global ser
    global file_list
    c = ""
    if (ser.isOpen() and ser.inWaiting() > 0):
        c = ser.read(ser.inWaiting())
        debug_text.insert("0.0", c) 

    usb64_window.after(10, serial_read) # check serial again soon

def device_get_info():
    if ser.isOpen():
        ser.reset_input_buffer()
        ser.reset_output_buffer()

        # 0xA9 is the command to ask for the device welcome message
        # Format is [0xA0], Readback: [string]
        ser.write(bytearray(b'\xA0'))
        line = ser.readline()
        debug_text.insert("0.0", line) 
        return 0

def flash_get_info():
    if ser.isOpen():
        ser.reset_input_buffer()
        ser.reset_output_buffer()

        # 0xA5 is the command to ask for flash info
        # Format is [0xA5], Readback (string): "A5, total size(bytes), free space(bytes)"
        ser.write(bytearray(b'\xA5'))

        # Read back info from device
        line = ser.readline()
        attempts = 3
        while line.decode('ascii')[:2] != "A5" and attempts > 0 :
            line = ser.readline()
            attempts = attempts - 1
        if attempts == 0:
            print("Error retrieving flash info")
            return 0

        # If successful parse the info and set the capacity bar
        flash_info = line.decode('ascii').split(",")
        flash_size = int(flash_info[1])
        flash_remaining = int(flash_info[2])
        global flash_capacity, flash_capacity_label 
        flash_capacity['value'] = (flash_size - flash_remaining) / flash_size * 100
        flash_capacity_label['text'] = "Capacity: " + \
                                        str(round((flash_size - flash_remaining)/1024/1024,2)) + "MB" + \
                                        " / " + \
                                        str(round(flash_size/1024/1024,2)) + "MB"
    return 0

def flash_format():
    if ser.isOpen():
        ser.reset_input_buffer()
        ser.reset_output_buffer()

        # 0xA6 is the command to format the flash chip
        # Format is [0xA6], Readback: [string]
        ser.write(bytearray(b'\xA6'))
        line = ser.readline()
        debug_text.insert("0.0", line) 
        return 0

def file_get_list():
    global file_list
    if ser.isOpen():
        ser.reset_input_buffer()
        ser.reset_output_buffer()

        # 0xA1 is the command to ask for the file list
        # Format is [0xA1], Readback: ["A1",num_files] [filename1,filename2,.. filenamen]
        ser.write(bytearray(b'\xA1'))

        # Read back info from device
        line = ser.readline()
        attempts = 3
        while line.decode('ascii')[:2] != "A1" and attempts > 0 :
            line = ser.readline()
            attempts = attempts - 1
        if attempts == 0:
            print("Error retrieving file list info")
            return 0

        # If successful parse the info and set the capacity bar
        returned_command = line.decode('ascii').split(",")
        num_files = int(returned_command[1])
        file_list.delete(0, tk.END)
        if num_files == 0:
            file_list.insert(1, "No files to show")
        while num_files > 0:
            line = ser.readline()
            file_list.insert(1, line)
            num_files = num_files - 1
        line = ser.readline()
        print("file_get_list returned: ", line)
    return 0

def file_delete():
    global file_list
    if ser.isOpen():
        ser.reset_input_buffer()
        ser.reset_output_buffer()

        # 0xA4 is the command to delete a file
        # Format is [0xA4, "Filename to delete"]
        ser.write(bytearray(b'\xA4'))
        print(file_list.get(ACTIVE))
        ser.write(file_list.get(ACTIVE))
        line = ser.readline()
        print("file_delete returned: ", line)

    # Update GUI info
    flash_get_info()
    file_get_list()
    return 0

def file_download():
    if ser.isOpen():
        ser.reset_input_buffer()
        ser.reset_output_buffer()

        # 0xA2 is the command to download a file
        ser.write(bytearray(b'\xA2'))
        line = ser.readline()
        print("file_download returned: ", line)
    file = asksaveasfile()
    return 0

def file_upload():
    #Get the file to upload
    file = askopenfile(mode ="rb")
    if file is None:
        return 0
    print("Opening : ", file.name)
    filename = os.path.basename(file.name)
    content = file.read()

    if ser.isOpen():
        ser.reset_input_buffer()
        ser.reset_output_buffer()

        # 0xA3 is the command to upload a file
        # Format is [0xA3 "filename" data...]
        ser.write(bytearray(b'\xA3'))
        ser.write(filename.encode())
        ser.write(bytearray(b'\x00'))

        # Send the file
        print("Writing...")
        ser.reset_output_buffer()
        ser.write(content)
        time.sleep(0.5)
        response = ser.readline()
        print("file_upload returned: ", response)

        # Update GUI info
        flash_get_info()
        file_get_list()
    return 0

def terminal_clear():
    global debug_text
    debug_text.delete(1.0,END)
    return 0

# Craete Window
usb64_window = tk.Tk()
usb64_window.configure(bg='#2C3539')

# Setup serial interface
ser = serial.Serial(rtscts=1)
ser.baudrate = 500000
ser.timeout = 1
serial_refresh()

# Create buttons
button_connect=Button(usb64_window, text='Connect', command=serial_connect, width = 20)
button_refresh=Button(usb64_window, text='Refresh Ports', command=serial_refresh, width = 20)
button_download=Button(usb64_window, text='Download', command=file_download, bg='#AFF8D8', width = 20)
button_upload=Button(usb64_window, text='Upload', command=file_upload, bg='#FFF5BA', width = 20)
button_delete=Button(usb64_window, text='Delete', command=file_delete, bg='#FFABAB', width = 20)
button_clear=Button(usb64_window, text='Clear Log', command=terminal_clear, bg='#FFABAB', width = 20)
button_format=Button(usb64_window, text='Format', command=flash_format, bg='#FFABAB', width = 20)

# Create File list box
file_list=Listbox(usb64_window, font=('Calibri',12), height = 8)

# Create debug output
debug_text=Text(usb64_window, font=('Calibri',12), height = 8, bg="#111111", fg="#FFFFFF")

# Create Flash capacity progress bar
flash_capacity_label = Label(usb64_window, text = "Flash Capacity")
flash_capacity = Progressbar(usb64_window, orient = HORIZONTAL, value = 0)

# Create labels
file_list_label = Label(usb64_window, text = "File List")
debug_log_label = Label(usb64_window, text = "Device Log")

# Layout the GUI
button_connect.grid(row=0, column=2, sticky=NSEW, columnspan = 1)
button_refresh.grid(row=0, column=3, sticky=NSEW, columnspan = 1)
button_download.grid(row=1, column=0, sticky=NSEW, columnspan = 1, pady = 10)
button_upload.grid(row=1, column=1, sticky=NSEW, columnspan = 1, pady = 10)
button_delete.grid(row=1, column=2, sticky=NSEW, columnspan = 1, pady = 10)
button_clear.grid(row=1, column=3, sticky=NSEW, columnspan = 1, pady = 10)
file_list_label.grid(row=2, column=0, sticky=NSEW, columnspan = 4)
file_list.grid(row=3, column=0, sticky=NSEW, columnspan = 4)
debug_log_label.grid(row=4, column=0, sticky=NSEW, columnspan = 4)
debug_text.grid(row=5, column=0, sticky=NSEW, columnspan = 4)
flash_capacity_label.grid(row=6, column=0, sticky=NSEW, columnspan = 4)
flash_capacity.grid(row=7, column=0, sticky=NSEW, columnspan = 4)
button_format.grid(row=8, column=0, sticky=NSEW, columnspan = 4)

# Start the GUI and serial port
usb64_window.after(100, serial_read)
usb64_window.mainloop()
