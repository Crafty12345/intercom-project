import pyaudio
import socket
from pynput.keyboard import Key, KeyCode, Listener
import json
from threading import Thread, Event
import secrets

import config
from protocol import Signal, HEADER_SIZE
from helper import *

FORMAT = pyaudio.paInt16
CLIENT_CHUNK = config.CLIENT_CHUNK_SIZE
# Make sure SERVER_CHUNK is much greater than CLIENT_CHUNK
SERVER_CHUNK = CLIENT_CHUNK * 2

# TODO: Replace this with a cryptographically secure sequence of bytes
stopToken = secrets.token_bytes(CLIENT_CHUNK - HEADER_SIZE)

selectedIp: str | None = None

with open(config.CLIENTS_FILENAME, "r") as f:
    targetIps: dict[str, str] = json.load(f)

isKeyPressed = False

audio = pyaudio.PyAudio()

running = True

hostConn = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

def onKeyPress(pKey: Key, pConn: socket.socket):
    global selectedIp

    isKeyPressed = True
    keyStr = ""
    if (type(pKey) is Key):
        keyStr = pKey.name
    elif (type(pKey) is KeyCode):
        keyStr = pKey.char
    if (keyStr in targetIps.keys()):
        selectedIp = targetIps[keyStr]
        header = Signal.AUDIO.value
        data = inputStream.read(CLIENT_CHUNK-HEADER_SIZE, False)
        actualData = header + data
        pConn.sendto(actualData, (selectedIp, config.PORT))

def runServer(pStopEvent: Event):
    global running
    hostConn.bind((config.BIND_IP,  config.PORT))
    stream = audio.open(format=pyaudio.paInt16, 
        channels=config.CHANNELS, 
        rate=config.SAMPLE_RATE,
        output=True,
        output_device_index=outputDeviceIndex)
    while ((running) and (not pStopEvent.is_set())):
        if running:
            data, addr = hostConn.recvfrom(SERVER_CHUNK, socket.MSG_WAITALL)
            header = data[:HEADER_SIZE]
            actualData = data[HEADER_SIZE:]
            if (header == Signal.AUDIO.value):
                stream.write(actualData)
            elif (header == Signal.STOP.value):
                if ((actualData == stopToken) and (addr[0] == config.BIND_IP)):
                    print("Stopping server")
                    running = False
            else:
                print("Invalid header: " + repr(header))

            if (pStopEvent.is_set()):
                print("Exiting")
            #print("Received message: %s" % data)
    print("Stopping server")
    stream.stop_stream()
    stream.close()
    hostConn.close()
    return False

listDevices()

inputDeviceIndex = selectDevice(config.INPUT_DEVICE)
outputDeviceIndex = selectDevice(config.OUTPUT_DEVICE)

inputStream = audio.open(format=FORMAT, 
                    channels=config.CHANNELS, 
                    rate=config.SAMPLE_RATE,
                    input=True, 
                    frames_per_buffer=CLIENT_CHUNK,
                    input_device_index=inputDeviceIndex)


stopEvent = Event()
serverThread = Thread(target=runServer, args=(stopEvent,))
serverThread.start()


with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as conn:
    with Listener(on_press=lambda key: onKeyPress(key, conn)) as listener:
        try:
            listener.join()
        except (KeyboardInterrupt):
            print("Stopping keyboard listener")
            listener.stop()

    conn.sendto(Signal.STOP.value + stopToken, (config.BIND_IP, config.PORT))

stopEvent.set()
serverThread.join()