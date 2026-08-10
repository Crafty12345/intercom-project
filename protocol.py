from enum import Enum

HEADER_SIZE = 4

class Signal(Enum):
    AUDIO = b"\x00\x00\x00\x01"
    STOP = b"\x00\x00\x00\x02"