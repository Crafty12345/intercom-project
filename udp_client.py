import pyaudio
import socket
from pynput.keyboard import Key, KeyCode, Listener
import json
from threading import Thread


import config
from helper import *

PORT = 5000

FORMAT = pyaudio.paInt16
CHUNK = config.CLIENT_CHUNK_SIZE

selectedIp: str | None = None

with open(config.CLIENTS_FILENAME, "r") as f:
    targetIps: dict[str, str] = json.load(f)

isKeyPressed = False

def onKeyPress(pKey: Key, pConn: socket.socket):
    global selectedIp
    # TODO: Refactor to do connection things in this function

    isKeyPressed = True
    keyStr = ""
    if (type(pKey) is Key):
        keyStr = pKey.name
    elif (type(pKey) is KeyCode):
        keyStr = pKey.char
    if (keyStr in targetIps.keys()):
        selectedIp = targetIps[keyStr]
        data = stream.read(CHUNK, False)
        conn.sendto(data, (selectedIp, PORT))

def onKeyReleased(pKey):
    isKeyPressed = False
    selectedIp = None

running = True

audio = pyaudio.PyAudio()

listDevices()

deviceIndex = selectDevice(config.INPUT_DEVICE)


stream = audio.open(format=FORMAT, 
                    channels=config.CHANNELS, 
                    rate=config.SAMPLE_RATE, 
                    input=True, 
                    frames_per_buffer=CHUNK,
                    input_device_index=deviceIndex)

with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as conn:
    with Listener(on_press=lambda key: onKeyPress(key, conn)) as listener:
        listener.join()
    ...



# while running:
#     try:
#         if ((isKeyPressed) and (selectedIp is not None)):
#             data = stream.read(CHUNK, False)

#             conn.sendto(data, (IP, PORT))
#     except (KeyboardInterrupt):
#         print("Exiting")
#         conn.close()
#         #stream.close()
#         running = False
