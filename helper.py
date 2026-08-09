import pyaudio

temp = pyaudio.PyAudio()

def listDevices():
    temp = pyaudio.PyAudio()

    print("Available devices:")
    for i in range(temp.get_device_count()):
        currentDevice = temp.get_device_info_by_index(i)
        print(f"{i}: " + currentDevice["name"])

def selectDevice(pName: str) -> int:
    deviceIndex = -1
    for i in range(temp.get_device_count()):
        currentDevice = temp.get_device_info_by_index(i)
        if (currentDevice["name"] == pName):
                deviceIndex = i
    if (deviceIndex == -1):
        raise LookupError("Device with name '%s' not found" % pName)
    else:
        print("Located output device '%s' with index %d" % (pName, deviceIndex))

    return deviceIndex