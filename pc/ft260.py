from __future__ import annotations
import hid
from typing import Tuple, List
from dataclasses import dataclass
from enum import Enum

class FT260Device:
    _hid: hid.device

    def connect(self):
        self._hid = hid.device()
        self._hid.open(0x0403, 0x6030)
        # self._hid.set_nonblocking(1)

    def disconnect(self):
        if self._hid is not None:
            self._hid.close()
        self._hid = None

    # Commands are documented here: 
    #    https://ftdichip.com/wp-content/uploads/2021/02/AN_394_User_Guide_for_FT260.pdf
    
    def get_chip_version(self) -> Tuple[int, int, int, int]:
        return tuple(self._hid.get_feature_report(0xA0, 64)[1:5])
    
    def get_system_status(self) -> FT260Device.SystemStatus:
        return FT260Device.SystemStatus.decode(self._hid.get_feature_report(0xA1, 64))
    
    def get_i2c_status(self) -> FT260Device.I2CStatus:
        return FT260Device.I2CStatus.decode(self._hid.get_feature_report(0xC0, 64))
    
    def set_i2c_speed(self, speed_khz: int):
        self._hid.send_feature_report([0xA1, 0x22, speed_khz & 0xFF, speed_khz >> 8])

    def i2c_write(self, device_addr: int, condition: I2CCondition, data: List[int]):
        # The report ID encodes an approximate length, which is supposed to be the smallest multiple
        # of 4 which fits the data being sent.
        # However, you don't _need_ to use this. It's perfectly fine to use the largest packet size,
        # and then specify a much smaller length within the packet.
        # To make our life easier, just do that!
        if len(data) > 60:
            raise ValueError("`data` is too large - up to 60 bytes at a time can be sent")
        if (0b10000000 & device_addr) > 0:
            raise ValueError("`device_addr` must be only 7 bits")

        payload = [
            0xDE, # Up to 60 bytes
            device_addr,
            condition.value,
            len(data),
            *data,
        ]
        self._hid.write(payload)
    
    @dataclass
    class SystemStatus:
        clock: int
        suspended: bool
        ready: bool
        i2c_enabled: bool

        def decode(data: List[int]) -> FT260Device.SystemStatus:
            return FT260Device.SystemStatus(
                clock={ 0: 12, 1: 24, 2: 48 }[data[2]],
                suspended=bool(data[3]),
                ready=bool(data[4]),
                i2c_enabled=bool(data[4]),
            )
        
    @dataclass
    class I2CStatus:
        # Status
        controller_busy: bool
        error: bool
        address_nack: bool
        data_nack: bool
        arbitration_lost: bool
        idle: bool
        bus_busy: bool

        speed_khz: int

        def decode(data: List[int]) -> FT260Device.I2CStatus:
            return FT260Device.I2CStatus(
                controller_busy=bool(data[1] & (1 << 0)),
                error=bool(data[1] & (1 << 1)),
                address_nack=bool(data[1] & (1 << 2)),
                data_nack=bool(data[1] & (1 << 3)),
                arbitration_lost=bool(data[1] & (1 << 4)),
                idle=bool(data[1] & (1 << 5)),
                bus_busy=bool(data[1] & (1 << 6)),

                speed_khz=(data[3] << 8) | data[2],
            )
        
    class I2CCondition(Enum):
        NONE = 0
        START = 2
        REPEATED_START = 3
        STOP = 4
        START_AND_STOP = 6
