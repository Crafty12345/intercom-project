import pyaudio
import socket

import config
from helper import *

IP = "192.168.4.98"
PORT = 5000
CHUNK = config.CLIENT_CHUNK_SIZE * 4
SAMPLE_RATE = config.SAMPLE_RATE

OUTPUT_DEVICE = config.OUTPUT_DEVICE

running = True

#print("Hostname=%s" % socket.gethostname())
conn = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)


conn.bind((IP,  PORT))

allData = ""

listDevices()
deviceIndex = selectDevice(OUTPUT_DEVICE)

if (deviceIndex == -1):
    raise LookupError("Device with name '%s' not found" % OUTPUT_DEVICE)
else:
    print("Located output device '%s' with index %d" % (OUTPUT_DEVICE, deviceIndex))

audio = pyaudio.PyAudio()
stream = audio.open(format=pyaudio.paInt16, 
                    channels=config.CHANNELS, 
                    rate=SAMPLE_RATE, 
                    output=True, 
                    output_device_index=deviceIndex)


while running:
    frames = []
    try:
        data, addr = conn.recvfrom(CHUNK, socket.MSG_WAITALL)
        stream.write(data)
        frames = []
        #print("Received message: %s" % data)
    except (KeyboardInterrupt):
        print("Closing server")
        conn.close()
        stream.close()
        running = False