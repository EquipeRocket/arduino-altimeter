import serial
import time
import csv
from termcolor import colored 
import matplotlib.pyplot as plt

def printPreliminary(decimalData):
    print colored("Preliminary Data", 'yellow')
    print colored(" Highest Altitude: %s" % max(decimalData), 'yellow')
    print colored(" Number of Entries: %s" % len(decimalData), 'yellow')

def createTableFile(decimalData):
    with open("data/data.csv", "wb") as file:
        writer = csv.writer(file)
        for data in decimalData:
            writer.writerow([data])

def convertToDecimal(data, size):
    buffer = []
    for i in xrange(int(size)/2):
        buffer.append(int(data[i*16:16*(i+1)], 2))
    return buffer

def askToSerial(ser, question, delay):
    ser.write(question)
    ser.flush()
    time.sleep(delay)
    return ser.read(ser.inWaiting())

def createGraph(data):
    plt.figure()
    plt.gca().set_position((.1, .3, .8, .6))
    plt.title("Launch Profile - On-board Altimeter")
    plt.plot(data)
    plt.xlabel("Time (Seconds)")
    plt.ylabel("Altitude (Meters)")
    plt.figtext(.1, .07, "Rocket: Rock One, Lift-off Weight: 200g, Engine: v1.0\nLocation: Cornelio Procopio-PR, Date: March 5, 2017\nApogee: %d, Sample Rate: 10Sps" % (max(data)))
    plt.savefig('./data/profile.png')

# Initiate
print colored("Altimeter Data Logger - Equipe Rocket 2017", 'blue')

# Connection
ser = serial.Serial('/dev/cu.usbmodem1411', 115200)
print colored("Connected to %s" % ser.name, 'green')
time.sleep(5)

# Sanity Check
print "Performing sanity check..."
ser.write('2')
if ser.read(7) == "CONN_OK":
    print colored(" CONN_OK", 'green')
else:
    print colored(" CONN_NOT_OK", 'red')

# Get Data Size
print "Getting log size..."
size = askToSerial(ser, '3', 2)
print colored(" Log size is %s bytes." % size, 'green')

# Data Dumping
print "Dumping the data..."
ser.write('1')
data = ser.read(int(size)*8)

# Data Validation
if len(data) == (int(size)*8):
    print colored(" Data received with the correct size.", 'green')
else:
    print colored(" Data is corrupt.", 'red')

# Save RAW Data
with open("data/data.s", "wb") as file:
    file.write(data)

decimalData = convertToDecimal(data, size)  # Convert from binary to decimal.
createTableFile(decimalData)                # Creates CSV.
createGraph(decimalData)                    # Creates Altitude Graph.
printPreliminary(decimalData)               # Print preliminary data.
