import pyaudio
import socket
from helper import *


IP = "127.0.0.1"
PORT = 5000

FORMAT = pyaudio.paInt16
CHANNELS = 1
RATE = 44100
CHUNK = 1024*4
INPUT_DEVICE = "pipewire"

running = True

audio = pyaudio.PyAudio()



listDevices()
deviceIndex = selectDevice(INPUT_DEVICE)


stream = audio.open(format=FORMAT, 
                    channels=CHANNELS, 
                    rate=RATE, 
                    input=True, 
                    frames_per_buffer=CHUNK, 
                    input_device_index=deviceIndex)

conn = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
if (conn):
    print("Successfully connected")

while running:
    try:
        data = stream.read(CHUNK, False)

        conn.sendto(data, (IP, PORT))
    except (KeyboardInterrupt):
        print("Exiting")
        conn.close()
        #stream.close()
        running = False