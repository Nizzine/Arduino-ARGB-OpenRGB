# Arduino ↔ OpenRGB ARGB Controller

Control ARGB LED strips via Arduino (Nano, RP2040, etc.) over USB, using OpenRGB. Great if your motherboard lacks ARGB headers or you want custom zones.

I'll explain below how I achieved a working solution how to control ARGB LEDS with Arduino Nano (clone) via OpenRGB.

I'll try to make this as beginner friendly as possible, but Quick Start for non-beginners also exists.

More of the technical details are towards the end.

Important notice: I only learned this stuff by trial and error. I do not know how to fix probably any of the issues you might have, so please, find your way to Google if you encounter problems not addressed here.

### We're using (tested, working solution):
- Windows environment.
- Arduino Nano (clone) or any other microcontroller (MCU) such as Raspberry Pi RP2040 (Pico).
- Arduino IDE and OpenRGB.
- ARGB LED strips or computer fans/lights with ARGB connectors. 5 volts.
- External 5 volt power supply.

## Quick Start for non-beginners

1. Flash the `NanoARGB.ino` sketch in this repo to your microcontroller using **Arduino IDE**.
2. Wire your LED strip(s):
   - Data pin → (Nano D2 or RP2040 pin)
   - Connect **common ground** between PSU, microcontroller, and LED strip(s)
   - Use a dedicated 5 V PSU rated 2× the max current (e.g., 5 V/6 A)
3. In **OpenRGB**, go to **Serial Devices** and add:
   - Port: e.g., `COM3`
   - Baud: `115200`
   - LED count: total number of LEDs in your script (e.g., `30`)
   - Protocol: **KeyboardVisualizer**
4. Press “OK” and apply an effect. Your strip should light up!

---
**Protocol options**:
  - `KeyboardVisualizer`: expects packets starting with `0xAA`, followed by RGB buffer and checksum.
  - `Adalight`: starts with `"Ada"` header, simplified framing.
  - You can switch by changing the protocol in the OpenRGB UI—but make sure your sketch matches.


## Preparation (Beginners):
1. Get Arduino IDE and OpenRGB.
2. Make sure your board is detected in Arduino IDE. If you have ESP8622 or just some random board, that should work, check out this article how to add other board managers to Arduino IDE:
[[https://support.arduino.cc/hc/en-us/articles/360016466340-Add-third-party-platforms-to-the-Boards-Manager-in-Arduino-IDE]]
3. In Arduino IDE, go to Library Manager and download FastLED library.
4. Test that your microcontroller is working correctly, Arduino, Raspberry, what have you. Verify it by uploading blink or other example sketch to your board.
If problems arise at this stage, Google is your friend.

## Setup
### Setting up OpenRGB
1. Open OpenRGB and go to settings tab.
2. Scroll down to Serial Devices and add a new device:
Add the port of your microcontroller seen in Arduino IDE. (COM5 or something).
Check the Baud (baudrate) of your microcontroller in Arduino IDE. 115200 is a good number.
TOTAL number of leds your system has. Put 10. If you need more, then check Wiring / Hardware below.
Important notice! Arduino cannot handle more than few leds, so either you need to make sure that the script you are running, reduces the brightness of the leds at least to half or you need a separate power supply for the LEDS. More about the wiring these later.
Protocol: Keyboard visualizer.
You can use Adalight if you have written the main script using it instead of FastLED and it should work. I haven't tested.
3. Save and exit OpenRGB software.

funfact: OpenRGB does not actually know if there is a microcontroller or anything on the other side of the USB cable, this just sets up a serial port to send it to.
There won't be errors or crashing of OpenRGB if you decide to pull your microcontoller out randomly.

Important notice episode 2 - The return of exitcode 1:
You can't upload scripts to your board from Arduino IDE if OpenRGB is open.

### Setting up the Script

Simply download or copy the NanoARGB.ino script and change the variables to your needs.


## Technical Babble About The Script

The script works by taking what ever raw serial data OpenRGB sends to Arduino, Pico, or whatever you are using for it to be decoded by the microcontoller.

### Detailed version:
OpenRGB generates full LED frame data (RGB values for every LED).

It packs those frames into a serial protocol:
KeyboardVisualizer style (0xAA … checksum)
Or Adalight ("Ada" … data).
It then blasts that over the COM port at the baud rate you set.

Your microcontoller is just a translator:
Listens to serial, parses the incoming frame, writes it out with FastLED as strict WS2812B timing pulses.

Why this is necessary:
WS2812/ARGB strips cannot be driven directly by UART — they need microsecond-precise timing.
So OpenRGB doesn't do that but instead sends the full raw data to your microcontroller to figure it out.


### Now, few things that I found:
1. Allow FastLED interruptions:
What this does, is allows FastLED to control the resetting of the LED status. Without it, your microcontroller might freeze randomly. Does not apply to all microcontrollers. Experiment.
hint: if your microcontroller freezes during playback of LED animation, try pressing reset on the microcontroller and if that returns the functionality of your microcontroller, then interrupts should be allowed.
```c
#define FASTLED_ALLOW_INTERRUPTS 1
```

2. Define baudrate:
Makes sure the serial communication at microcontroller's end is happening the same rate as OpenRGB expects it to.
Use this or what ever your microcontoller supports. You might be able to check it in your Arduino IDE.
```c
#define BAUDRATE 115200
```

3. Number of LEDs in your script:
Always make sure you update the OpenRGB side of the LED amount matches what is in your script.

## Wiring / Hardware

### Power math (so you size things right)
WS2812/“5 V ARGB” worst case ≈ 60 mA/LED @ full-white.
75 LEDs -> 4.5 A worst-case. Real use is usually lower, but size the PSU ≥ 2× headroom if you can (e.g., 5 V/10 A).
If you cap brightness to ~30–40%, typical draw is ~1.3–1.8 A.

### Wiring that actually works (order matters)
Use a proper 5 V DC supply, not a phone charger. (Look for regulated bricks / Mean Well style.)
Common ground is mandatory: connect PSU GND ↔ MCU GND ↔ LED GND (star or short ground bus). No common ground = unstable data.
Power LEDs directly from the PSU, not from the Nano/Pico 5 V pin.
Cut USB 5 V back-feed if you also power the MCU from the LED PSU.
Easiest: leave MCU on USB only, LEDs on PSU, share GND only.
If you must feed MCU from the PSU 5 V pin, do not also have USB 5 V connected (use a data-only cable or a Schottky diode on 5 V).
Thick, short power leads to the strip(s): 18–20 AWG. Avoid skinny JST pigtails for the main feed.
Power injection: with 15–20 LEDs per segment you’ll still benefit from:
Inject 5 V/GND at the start of each strip, and optionally at the far end if whites look pink/yellow.
Bulk caps: put a 1000 µF (or bigger) 6.3 V+ electrolytic across 5 V<->GND at the start of each strip.
Data line resistor: 330–470 Ω in series at the strip DIN to tame ringing.
Level shifting (RP2040 only): RP2040 data is 3.3 V, strips want ~5 V logic. Many “work”, but it’s marginal, especially when V drops. Use a 74AHCT125/74HCT14 (AHCT preferred). Nano is 5 V, so this isn’t needed there.
Ground reference for data: keep data and its ground bundled; don’t run data long by itself.


## Troubleshooting

Always check wiring! Bad cables are bad mmmkay!

### Microcontroller freezes randomly:
if your microcontroller freezes during playback of LED animation: 
- Try pressing reset on the microcontroller and if that returns the functionality of your microcontroller, then interrupts should be allowed in script.
- Try lowering the brightness of your LEDS in OpenRGB.
- Try limiting the max power usage in script by adding these in void setup():
```c
FastLED.setBrightness(96);  // 0..255
FastLED.setMaxPowerInVoltsAndMilliamps(5, 1500); // e.g., 1.5A budget
```
- Make sure your microcontroller gets enough power.

### Low brightness causes flickering
- Try adding to void setup():
```c
FastLED.setDither(0);
```
### Arduino IDE gives exitcode 1 when trying to upload script
- Exit OpenRGB. From the hidden icons too.

### Animations are delayed
You're most likely asking too much from your microcontroller.
- Get a more powerful microcontroller such as RP2040 (Raspberry Pi Pico)
- Try a higher clockspeed for your board if possible.
