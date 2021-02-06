# tetris-table

This project aims to have a playable tetris game on a led table.

The project is constitued by:
- part-arduino: the arduino schema that drives the leds
- part-common: shared definitions / config / code.
- part-windows: windows executable that drives the arduino.

High level description:
- Parts communicate through a Serial interface.
- A command layer is built on top the serial link.
- The windows part generate commands to fill a color buffer
- the arduino part interpret commands and drives the leds
