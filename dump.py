import serial
import time
import csv
from termcolor import colored 
import matplotlib.pyplot as plt
from struct import unpack
import numpy as np
from binascii import unhexlify

def decode_float(s):
  return unpack('<f', unhexlify(s))[0]

def printPreliminary(decimalData):
    print colored("Preliminary Data", 'yellow')
    print colored(" Highest Altitude: %.2f" % max(decimalData), 'yellow')
    print colored(" Number of Entries: %s" % len(decimalData), 'yellow')

def createTableFile(decimalData):
    with open("data/data.csv", "wb") as file:
        writer = csv.writer(file)
        for data in decimalData:
            writer.writerow([data])

def convertToDecimal(data, size):
    buffer = []
    for i in xrange(size/8):
        buffer.append(float(decode_float(data[i*8:8*(i+1)])))
    return buffer

def askToSerial(ser, question, delay):
    ser.write(question)
    ser.flush()
    time.sleep(delay)
    return ser.read(ser.inWaiting())

def createGraph(data, sampling, size):
    plt.figure()
    plt.gca().set_position((.12, .3, .8, .6))
    plt.title("Launch Profile - On-board Altimeter")
    plt.plot(np.linspace(start=0, stop=size/8/sampling, num=size/8), data)
    plt.xlabel("Time (Seconds)")
    plt.ylabel("Altitude (Meters)")
    plt.figtext(.12, .07, "Rocket: Rock One\nLocation: Cornelio Procopio-PR, Date: March 5, 2017\nApogee: %.2fm, Sample Rate: %dSps" % (max(data), sampling))
    plt.savefig('./data/profile.png')

# Initiate
print colored("Altimeter Data Logger - Equipe Rocket 2017", 'blue')

# Connection
ser = serial.Serial('/dev/cu.usbserial-A700eSpX', 115200)
print colored("Connected to %s" % ser.name, 'green')
ser.read(233)

# Sanity Check
print "Performing sanity check..."
ser.write('2')
if ser.read(7) == "CONN_OK":
    print colored(" CONN_OK", 'green')
else:
    print colored(" CONN_NOT_OK", 'red')

# Get Data Size
print "Getting sample size..."
ser.write('3')
size = int(ser.read(16), 2)*8
print colored(" Sample size is %s bytes." % (size*4/8), 'green')

# Get Sample Rate
print "Getting sample rate..."
ser.write('4')
sampling = int(ser.read(16), 2)
print colored(" Sample rate is %ssps." % sampling, 'green')

# Data Dumping
print "Dumping the data..."
ser.write('1')
data = ser.read(size)

# Data Validation
if len(data) == (size):
    print colored(" Data received with the correct size.", 'green')
else:
    print colored(" Data is corrupt.", 'red')

# Save RAW Data
with open("data/data.s", "wb") as file:
    file.write(data)

decimalData = convertToDecimal(data, size)  # Convert from hexadecimal to decimal.
createTableFile(decimalData)                # Creates CSV.
createGraph(decimalData, sampling, size)    # Creates Altitude Graph.
printPreliminary(decimalData)               # Print preliminary data.