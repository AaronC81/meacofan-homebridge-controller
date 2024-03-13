# Meacofan HomeBridge Controller

This project connects a [MeacoFan 1056](https://www.meaco.com/products/meacofan-1056-air-circulator)
desk fan to HomeKit using [Homebridge](https://homebridge.io/), enabling it to be controlled using
your iOS device or Siri.

The PCB blasts the same IR commands as the fan's real remote control, accepting commands over I2C so
that it can be controlled from a Raspberry Pi.

> [!WARNING]
> This is quite rough around the edges. I wouldn't recommend building one of these yourself!

## PCB

The PCB is a KiCad project in `hardware/pcb`. (The BOM is hopefully self-explanatory from the
schematic!)

Once assembled, the on-board ISP port can be used to flash the ATtiny85. You'll also need to set
appropriate fuses to use the internal 8 MHz clock, which the software relies on. The AVRDUDE flags
for this are:

```
-U lfuse:w:0xe2:m -U hfuse:w:0xdf:m -U efuse:w:0xff:m
```

(Note that, until the clock is increased with these fuses, you may need to slow down ISP
communication speed: `-B 125kHz`)

## Homebridge Plugin

`homebridge-plugin` contains a Homebridge accessory plugin which implements the
[`Fan`](https://developers.homebridge.io/#/service/Fan) service. This sends I2C commands to the PCB
to match the desired speed set through HomeKit. (HomeKit only supports 0%-100% speeds, so these
are mapped onto the fan's 1-12 speed settings.)

You will need the `i2c-tools` package installed to provide the `i2ctransfer` command-line tool.
Make sure that I2C is enabled in your `raspi-config` too. If everything is working OK, the device
should appear at address 0x40 in `i2cdetect`.

## Software

The software is written as a PlatformIO project targeting the ATtiny85, using the Arduino framework.

### Communication

The ATtiny85 acts as a bit-banged I2C peripheral on address 0x40. Only one operation is supported,
a single-byte write where the byte is in the following format:

```
  .--------------- checksum - number of 1s in rest of packet
.-'-.
c c c 1 s s s s
        '--.--'
           '------ new desired speed (0-12), 0 means off
```

These packets are designed to be idempotent, so your host should probably send these multiple times
to account for the sloppy I2C bit-banging.

### IR Commands

The fan operates with NEC-format IR codes at address 0x80. The individual commands are documented
below: 

| Purpose                | Command | Raw        |
| ---------------------- | ------- | ---------- |
| Power                  | 0x92    | 0x6D927F80 |
| Vertical oscillation   | 0x8A    | 0x758A7F80 |
| Horizontal oscillation | 0x87    | 0x78877F80 |
| Eco                    | 0x81    | 0x7E817F80 |
| Pre-on                 | 0x82    | 0x7D827F80 |
| Pre-off                | 0x83    | 0x7C837F80 |
| Light                  | 0x9E    | 0x619E7F80 |
| Speed up               | 0x89    | 0x76897F80 |
| Speed down             | 0x9F    | 0x609F7F80 |

## Enclosure

There's a very rough 3D printed enclosure which keeps the IR LED aimed in `hardware/case`. 
