# Agnostic Box Pure Data Patch

This folder contains a Pure Data patch for using the Agnostic Box as a serial controller for FM synthesis.

The patch `arduino_controlling_FM_synth.pd` receives the serial array sent by the Arduino Nano ESP32, separates the values in Pure Data, and uses them to control an FM sine synthesizer.

## Install Pure Data

Download and install Pure Data Vanilla:

- Pure Data project site: <https://puredata.info/>
- Pure Data source and install notes: <https://github.com/pure-data/pure-data>
- Miller Puckette's Pd download page: <http://msp.ucsd.edu/software.html>

After installing Pd, open it once and check that audio works:

1. Open `Media > Audio Settings...`.
2. Select your audio input/output device.
3. Turn on `DSP` in the main Pd window.
4. Open `Media > Test Audio and MIDI...` if you want to confirm sound output.

## Install the `comport` External

The patch uses the `[comport]` object to read serial data from the Agnostic Box. `[comport]` is not part of Pd Vanilla, so install it with Deken, Pd's external/package manager.

Useful links:

- Deken documentation: <https://deken.puredata.info/docs>
- Deken external search: <https://deken.puredata.info/search.html>
- Pd external installation is also described in the Pure Data README: <https://github.com/pure-data/pure-data>

Install from inside Pure Data:

1. Open Pd.
2. Go to `Help > Find Externals...`.
3. Search for `comport`.
4. Install the package that matches your operating system.
5. Restart Pure Data.
6. Create a new object called `[comport]`. If it creates without an error, the install worked.

## Upload the Arduino Serial Sketch

Upload the mapped serial sketch to the Arduino Nano ESP32:

```text
../agnostic box arduino nano esp32 code/sensor_mapped_serial_test/sensor_mapped_serial_test.ino
```

That sketch sends seven values over serial:

```text
pot0 pot1 pot2 button0 button1 button2 gyro
```

The mapped values are:

- `pot0`, `pot1`, `pot2`: `1-127`
- `button0`, `button1`, `button2`: `0` or `1`
- `gyro`: `0-127`, based on `0-90` degrees, clamped for out-of-range motion

The Arduino sketch uses:

```cpp
Serial.begin(115200);
```

So the Pd patch should use the same baud rate:

```text
[comport 0 115200]
```

If the existing patch has another baud rate, edit the `[comport]` object so it matches the Arduino sketch.

## Connect the Agnostic Box to Pure Data

1. Connect the Agnostic Box by USB.
2. Close the Arduino Serial Monitor. Only one program can usually use the serial port at a time.
3. Open `arduino_controlling_FM_synth.pd` in Pure Data.
4. Turn on `DSP`.
5. Click the `devices` message in the patch.
6. Check the Pd console for the serial device list.
7. Change the `open 0` message if needed. For example, use `open 1`, `open 2`, etc. depending on the listed device number.
8. Click the correct `open` message.

If serial data is arriving, the number boxes after `[unpack f f f f f f f]` should update when you turn knobs, press buttons, or tilt the box.

## How the Serial Parsing Works

The Arduino sends a line of space-separated numbers, ending with a newline:

```text
64 23 127 0 1 0 45
```

In the Pd patch:

- `[comport]` receives raw serial bytes.
- `[sel 13 10]` detects carriage return and newline characters.
- The list-building objects collect each line.
- `[fudiparse]` converts the incoming text into Pd numbers.
- `[unpack f f f f f f f]` separates the seven values.

The value order is:

```text
1. pot0
2. pot1
3. pot2
4. button0
5. button1
6. button2
7. gyro
```

## Using the Values for FM Synthesis

FM synthesis uses one oscillator to modulate the frequency of another oscillator.

In this patch:

- `[osc~]` objects generate sine waves.
- `[mtof]` converts controller values into musical frequencies.
- `[*~]` scales modulation depth.
- `[+~]` adds modulation to the carrier frequency.
- `[dac~]` sends the final audio to the speakers.

A useful control layout is:

- `pot0`: carrier pitch
- `pot1`: modulator pitch
- `pot2`: modulation depth or output level
- `gyro`: extra modulation depth, feedback, or vibrato-like movement
- `button0`, `button1`, `button2`: enable/disable modulation paths, hold notes, or switch presets

Start with low volume before turning on DSP. FM patches can get loud quickly when modulation depth or feedback is high.

## Troubleshooting

- If `[comport]` says it cannot create, install the `comport` external with `Help > Find Externals...` and restart Pd.
- If the device will not open, close Arduino IDE Serial Monitor and any other serial terminal.
- If values do not move, click `devices`, confirm the device number, then send the matching `open` message.
- If the values look wrong, make sure the Pd baud rate matches `Serial.begin(115200)` in the Arduino sketch.
- On Linux, your user may need permission to access serial devices such as `/dev/ttyACM0` or `/dev/ttyUSB0`.

## Credits

Created for the Agnostic Box project by chihauccisoilconte.

This Pure Data patch uses Pure Data Vanilla and the `comport` external for serial communication.

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
