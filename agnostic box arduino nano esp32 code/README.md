# Agnostic Box Arduino Nano ESP32 Code

This folder contains Arduino sketches for the Agnostic Box.

The code is made for the Arduino Nano ESP32 and is intended to be opened and uploaded with the [Arduino IDE](https://www.arduino.cc/en/software). Some sketches use the [Mozzi](https://github.com/sensorium/Mozzi) audio synthesis library.

## Arduino IDE Setup

1. Install the Arduino IDE from <https://www.arduino.cc/en/software>.
2. Open the Arduino IDE.
3. Open `Tools > Board > Boards Manager...`.
4. Search for `Nano ESP32`.
5. Install the Arduino Nano ESP32 board package.
6. Select the board from `Tools > Board`.
7. Select the correct USB port from `Tools > Port`.

For generic ESP32 board support, add this URL in `File > Preferences > Additional boards manager URLs`, then install the `esp32` platform from Boards Manager:

```text
https://espressif.github.io/arduino-esp32/package_esp32_index.json
```

## Mozzi Library Setup

1. Open `Sketch > Include Library > Manage Libraries...`.
2. Search for `Mozzi`.
3. Install the Mozzi library.
4. Open one of the sketches in this folder.
5. Compile and upload it to the Arduino Nano ESP32.

## License

MIT License

Copyright (c) 2026 chihauccisoilconte

Permission is hereby granted, free of charge, to any person obtaining a copy
of this repository and associated documentation files (the "Work"), to deal
in the Work without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Work, and to permit persons to whom the Work is furnished to do
so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Work.

THE WORK IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE WORK OR THE USE OR OTHER DEALINGS IN THE
WORK.
