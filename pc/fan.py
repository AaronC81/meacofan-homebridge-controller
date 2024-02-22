from ft260 import FT260Device

class Fan:
    ft260: FT260Device

    def __init__(self):
        self.ft260 = FT260Device()

    def connect(self):
        self.ft260.connect()
        self.ft260.set_i2c_speed(60)

    def disconnect(self):
        self.ft260.disconnect()

    def set_speed(self, speed: int):
        self.ft260.i2c_write(0x40, FT260Device.I2CCondition.START_AND_STOP, [0b10000000 | speed])

    def turn_off(self):
        self.set_speed(0)
