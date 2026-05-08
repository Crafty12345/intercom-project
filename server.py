import queue
import signal
import socket
import sys
import threading

import pyaudio

HOST = "localhost" if len(sys.argv) <= 1 else sys.argv[1]
PORT = 1234 if len(sys.argv) <= 2 else int(sys.argv[2])
HOST = "0.0.0.0"
PORT = 9999

def acceptData(s, buff):
    while 1:
        data = s.recv(512)
        if not data:
            break
        buff.put(data)
    s.close()

def runSocketConnectiion(pPort, pAudioStream):
    global s
    buff = queue.Queue()
    while True:
        try:
            s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            s.bind(("0.0.0.0", int(PORT)))
            print(f"Binded to port {PORT}")
            s.listen(5)
            print("listening")
            conn, addr = s.accept()
            print("accepted")
            print(f"Received connection from {addr}")
            t = threading.Thread(target=acceptData, args = (conn, buff))
            t.daemon = True
            t.start() # begin receiving data
            #acceptData(conn, buff)
            while 1:
                try:
                    frame = buff.get(True, 5)  # blocking get with 5 second timeout
                except queue.Empty:
                    #print("End of stream")
                    break

                pAudioStream.write(frame)
        except (ConnectionRefusedError, ConnectionAbortedError) as e:
            print(str(e))
        finally:
            s.close()


s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.bind(("0.0.0.0", int(PORT)))
print(f"Binded to port {PORT}")
s.listen(5)

#t = threading.Thread(target=acceptData, args = (s, buff))
#t.daemon = True
#t.start() # begin receiving data

p = pyaudio.PyAudio()
DEVICE_INDEX=16
NUM_CHANNELS = 1
SAMPLE_RATE = 44100

stream = p.open(format=pyaudio.paInt16,
                channels=NUM_CHANNELS,
                rate=SAMPLE_RATE,
                output=True,
                output_device_index=DEVICE_INDEX)

def interrupt_handler(signal, frame):
    print("Quitting...")

    stream.stop_stream()
    stream.close()
    p.terminate()
    s.close()

    sys.exit(0)

signal.signal(signal.SIGINT, interrupt_handler)
signal.signal(signal.SIGTERM, interrupt_handler)

runSocketConnectiion(PORT, stream)