from pynput.keyboard import Key, KeyCode, Listener
from typing import cast

def onPress(pKey: Key | KeyCode):
    keyStr = ""
    if (type(pKey) is Key):
        keyStr = pKey.name
    elif (type(pKey) is KeyCode):
        keyStr = pKey.char
    
    print("%s pressed" % keyStr)
    print("type="  + repr(type(keyStr)))

with Listener(on_press=onPress) as listener:
    listener.join()


print("test")