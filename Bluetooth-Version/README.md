# Reaction Time Game -- STM32F401 + HC-05 Bluetooth

A two-player reaction time game built on the STM32F401 microcontroller. Three LEDs count down, then all go dark. The moment they go off, you slap a touch sensor as fast as you can. Your reaction time (in milliseconds) is sent over Bluetooth to a serial terminal app on your phone. After both players finish their rounds, the board declares a winner.

No screen. No keyboard. Just LEDs, a touch button, and your phone.

---

## Table of Contents

1. [What You Need](#what-you-need)
2. [Understanding the Pins](#understanding-the-pins)
3. [Wiring Everything Up](#wiring-everything-up)
4. [Setting Up the HC-05 Bluetooth Module](#setting-up-the-hc-05-bluetooth-module)
5. [Installing Keil uVision and Creating a Project](#installing-keil-uvision-and-creating-a-project)
6. [Loading the Code onto the Board](#loading-the-code-onto-the-board)
7. [Installing the Bluetooth Serial App](#installing-the-bluetooth-serial-app)
8. [Playing the Game](#playing-the-game)
9. [How the Code Works (Plain English)](#how-the-code-works-plain-english)
10. [Changing the Number of Rounds](#changing-the-number-of-rounds)
11. [Troubleshooting](#troubleshooting)

---

## What You Need

### Hardware

| Item | Why You Need It |
|------|-----------------|
| STM32F401 board (Black Pill or Nucleo-F401RE) | The brain of the project -- runs your code |
| HC-05 Bluetooth module | Sends game data wirelessly to your phone |
| 3 LEDs (any color) | The countdown lights |
| 3 resistors (220 ohm to 330 ohm) | Protect the LEDs from burning out |
| 1 TTP223 capacitive touch sensor module | The "button" players press |
| Breadboard | Where you plug everything in without soldering |
| Jumper wires (male-to-male and male-to-female) | Connect everything together |
| Micro-USB cable | Connects the board to your computer for programming |
| A smartphone (Android or iPhone) | Displays game results via Bluetooth |
| A computer (Windows) | For writing and uploading the code |

### Software

| Software | Purpose |
|----------|---------|
| Keil uVision 5 (MDK-ARM) | Write, compile, and flash the code to the board |
| STM32F4 Device Pack for Keil | Lets Keil understand your specific chip |
| A Bluetooth Serial app on your phone | Receives and displays game messages |

---

## Understanding the Pins

Before you wire anything, here is exactly which pins on the STM32F401 board do what in this project.

### LED Pins (Output -- on Port B)

| Pin | Role |
|-----|------|
| PB6 | LED 1 -- lights up first in the countdown |
| PB7 | LED 2 -- lights up second |
| PB8 | LED 3 -- lights up third, then all go dark |

### Touch Sensor Pin (Input -- on Port A)

| Pin | Role |
|-----|------|
| PA6 | Reads the touch sensor. When you touch it, this pin goes HIGH |

### Bluetooth (USART1) Pins (on Port A)

| Pin | Role |
|-----|------|
| PA9 | TX (Transmit) -- sends data FROM the board TO the HC-05 |
| PA10 | RX (Receive) -- receives data FROM the HC-05 TO the board |

### Power Pins

| Connection | Details |
|------------|---------|
| 3.3V | Powers the touch sensor and HC-05 (the HC-05 usually has an onboard regulator so 5V also works -- check your module) |
| GND | Common ground. Everything must share the same ground |

---

## Wiring Everything Up

Grab your breadboard and jumper wires. Follow these connections carefully. Double-check every wire before powering on.

### LEDs

Each LED has two legs: the longer leg is positive (anode), the shorter leg is negative (cathode).

For each LED, the circuit goes:

```
STM32 Pin --> Resistor (220-330 ohm) --> LED long leg (anode) --> LED short leg (cathode) --> GND
```

| STM32 Pin | Component |
|-----------|-----------|
| PB6 | Resistor then LED 1 then GND |
| PB7 | Resistor then LED 2 then GND |
| PB8 | Resistor then LED 3 then GND |

### TTP223 Touch Sensor

The TTP223 module typically has three pins labelled on its board:

| TTP223 Pin | Connect To |
|------------|------------|
| VCC | 3.3V on the STM32 |
| GND | GND on the STM32 |
| SIG (or OUT or I/O) | PA6 on the STM32 |

When you touch the sensor pad, the SIG pin goes HIGH. That is what triggers the interrupt in the code.

### HC-05 Bluetooth Module

The HC-05 has several pins. You only need four of them:

| HC-05 Pin | Connect To | Notes |
|-----------|------------|-------|
| VCC | 5V on the STM32 | Most HC-05 modules have an onboard 3.3V regulator, so they take 5V input. Check your module's datasheet. If your board only has 3.3V, that may work too on some modules. |
| GND | GND on the STM32 | Shared ground is critical |
| TXD | PA10 (RX) on the STM32 | The HC-05 sends data out through its TXD, so it connects to the board's receive pin |
| RXD | PA9 (TX) on the STM32 | The board sends data out through PA9, so it connects to the HC-05's receive pin |

**Important**: Notice the crossover. TX on one side connects to RX on the other side. This is how serial communication works -- one device's "mouth" talks into the other device's "ear."

**Voltage Warning**: The HC-05 RXD pin expects 3.3V logic. If your STM32 board outputs 3.3V on its GPIO pins (the F401 does), you are fine. If you were using a 5V board, you would need a voltage divider on the RXD line.

### Complete Wiring Summary

```
STM32F401              Component
----------             ---------
PB6  ---- [resistor] ---- LED 1 ---- GND
PB7  ---- [resistor] ---- LED 2 ---- GND
PB8  ---- [resistor] ---- LED 3 ---- GND

PA6  -------------------- TTP223 SIG
3.3V -------------------- TTP223 VCC
GND  -------------------- TTP223 GND

PA9  (TX) --------------- HC-05 RXD
PA10 (RX) --------------- HC-05 TXD
5V   -------------------- HC-05 VCC
GND  -------------------- HC-05 GND
```

---

## Setting Up the HC-05 Bluetooth Module

The HC-05 comes pre-configured with a default baud rate of 9600 (which is exactly what this code uses), so you generally do not need to change any settings. Just power it up and it will start blinking its onboard LED, waiting for a phone to connect.

If you have never paired an HC-05 before:

1. Power on your circuit (plug the STM32 into USB or a power source).
2. The HC-05's red LED should blink rapidly. That means it is in pairing mode.
3. On your phone, go to Settings, then Bluetooth, and scan for devices.
4. You should see a device called "HC-05" (or sometimes "HC-06" or a similar name).
5. Tap it to pair. It will ask for a PIN. The default PIN is usually **1234** or **0000**.
6. Once paired, the HC-05's LED will either slow its blink or go solid, depending on the module version.

You are now paired. But pairing alone does not mean data is flowing -- you need a serial terminal app (covered below) to actually open a connection and see the messages.

---

## Installing Keil uVision and Creating a Project

### Step 1: Download and Install Keil

1. Go to the Keil website: https://www.keil.com/download/product/
2. Download **MDK-ARM** (the free community edition works for projects under 32KB, which this project is).
3. Run the installer and follow the on-screen steps. Install to the default location.

### Step 2: Install the STM32F4 Device Pack

1. Open Keil uVision.
2. Go to the menu: **Project** then **Manage** then **Pack Installer**.
3. In the left panel, find **STMicroelectronics** then **STM32F4 Series**.
4. On the right panel, look for **Keil::STM32F4xx_DFP** and click **Install**.
5. Wait for the download to finish, then close the Pack Installer.

### Step 3: Create a New Project

1. In Keil, go to **Project** then **New uVision Project**.
2. Pick a folder to save your project in. Name it something like `ReactionGame`.
3. A window pops up asking you to select your device. Search for **STM32F401CCUx** (for the Black Pill) or **STM32F401RETx** (for the Nucleo). Select the correct one and click **OK**.
4. A "Manage Run-Time Environment" window pops up. Check these boxes:
   - **CMSIS** then **CORE**
   - **Device** then **Startup**
5. Click **OK**.

### Step 4: Add the Code File

1. In the Project panel on the left, right-click on **Source Group 1** and select **Add Existing Files to Group**.
2. Navigate to where you saved `show_case.c`, select it, and click **Add**.
3. Close the dialog.

### Step 5: Build the Project

1. Press **F7** or go to **Project** then **Build Target**.
2. The Build Output window at the bottom should show **0 Error(s)** if everything is set up correctly.
3. If you see errors about missing header files, make sure the STM32F4 Device Pack is installed and configured (Step 2 above).

---

## Loading the Code onto the Board

### For Black Pill (using ST-Link V2 programmer)

1. Connect your ST-Link V2 to the board:
   - SWDIO to SWDIO
   - SWCLK to SWCLK
   - GND to GND
   - 3.3V to 3.3V (only if you are powering the board from the ST-Link)
2. Plug the ST-Link into a USB port on your computer.
3. In Keil, go to **Flash** then **Configure Flash Tools**.
4. Click the **Debug** tab.
5. In the dropdown on the right, select **ST-Link Debugger**.
6. Click **Settings** and make sure the device is detected (you should see the chip ID).
7. Click **OK** twice to close the settings.
8. Go to **Flash** then **Download** (or press **F8**).
9. The bottom panel should say something like "Flash Download Successful." Your code is now on the board.

### For Nucleo Board (built-in ST-Link)

1. Just plug the Nucleo into your computer using the micro-USB cable. The Nucleo has a built-in ST-Link programmer.
2. Follow steps 3 through 9 above. The built-in ST-Link should be automatically detected.

### After Flashing

Press the reset button on your board (or unplug and replug USB). The board is now running your code.

---

## Installing the Bluetooth Serial App

You need a serial terminal app on your phone that can talk to the HC-05 over Bluetooth.

### Android

Download one of these free apps from the Google Play Store:

- **Serial Bluetooth Terminal** by Kai Morich (highly recommended -- clean and simple)
- **Bluetooth Terminal** by QWERTY

**How to set it up (using Serial Bluetooth Terminal):**

1. Open the app.
2. Tap the menu icon (three horizontal lines or three dots) and go to **Devices**.
3. You should see **HC-05** listed under Paired Devices (you paired it earlier, remember?).
4. Tap on **HC-05** to select it.
5. Go back to the terminal screen and tap the **Connect** icon (it looks like two plugs connecting, usually at the top).
6. The status bar should change to "Connected."
7. You are now ready. Any text the STM32 sends over Bluetooth will appear on your screen.

### iPhone

Bluetooth Serial apps on iPhone are trickier because iOS does not support the classic Bluetooth SPP protocol that the HC-05 uses. You have two options:

- **Option A**: Use an **HM-10** module instead of the HC-05. The HM-10 uses Bluetooth Low Energy (BLE), which iOS supports. The wiring is the same, but you may need to adjust the baud rate in the code.
- **Option B**: Use an Android phone or tablet for this project.

If you do use an HM-10 with an iPhone, apps like **LightBlue** or **nRF Connect** can communicate with it.

---

## Playing the Game

Now that everything is wired up, the code is on the board, and your phone is connected via the Bluetooth serial app, here is how the game works:

### Starting a Game

1. Open the Bluetooth serial app on your phone and connect to HC-05.
2. You should see a welcome message:
   ```
   === Reaction Time Game ===
   Each player plays 1 round(s)
   Winner determined by average time
   Send 'start' or 'START' to begin
   ```
3. Type **start** in the app's text input field and hit Send.

### Playing a Round

4. The app will show:
   ```
   *** NEW GAME STARTED ***
   Playing 1 round(s) per player

   Player 1 - Round 1/1
   Get ready...
   ```
5. Watch the LEDs on the breadboard. They will light up one by one (LED 1, then LED 2, then LED 3).
6. After a short pause, all three LEDs go dark at the same time.
7. The app shows: **GO! Touch now!**
8. Touch the sensor as fast as you can.
9. Your reaction time appears:
   ```
   Player 1 Time: 287 ms
   ```
10. The app then says:
    ```
    Send 'start' for next turn
    ```

### Player 2's Turn

11. Hand the board to Player 2 (or have them stand by the touch sensor).
12. Type **start** again in the app.
13. Player 2 goes through the same countdown and touch sequence.

### The Winner

14. After both players finish all their rounds, the app displays a summary:
    ```
    --------- GAME OVER ----------
    Player 1 Average: 287 ms
      Times: 287 ms

    Player 2 Average: 342 ms
      Times: 342 ms

    *** WINNER: Player 1 *** (287 ms)
    -----------------------------

    Send 'start' to play again
    ```
15. The player with the lower average reaction time wins. If both averages are equal, it is a tie.
16. Type **start** again to play another game.

---

## How the Code Works (Plain English)

If you are curious about what is happening under the hood, here is a simplified walkthrough of the code. You do not need to understand all of this to play the game, but it helps if you want to modify it.

### Initialization

When the board powers on, the `main()` function calls four setup functions:

- **GPIO_Init()** -- Tells the chip which pins are outputs (LEDs) and which are inputs (touch sensor).
- **SysTick_Init()** -- Sets up a millisecond timer. Think of it as a stopwatch that ticks every 1 millisecond.
- **EXTI_Init()** -- Configures an interrupt on pin PA6. An interrupt means: "Stop whatever you are doing and run this special code the instant the touch sensor is pressed."
- **USART1_Init()** -- Sets up serial communication at 9600 baud on pins PA9 and PA10. This is how the board talks to the HC-05.

### The Game Loop

The main `while(1)` loop runs forever and does the following:

1. **Waits for a command.** The HC-05 receives the text you type on your phone and forwards it to the board byte by byte. The `USART1_IRQHandler` interrupt collects these bytes into a buffer. When it sees a newline character (you pressed Send), it sets a `Command_Ready` flag.

2. **Checks the command.** If the command is "start" or "START", it proceeds. Otherwise, it sends back "Invalid command."

3. **Runs the LED sequence.** `LED_Sequence()` turns on LED 1, waits one second, turns on LED 2, waits one second, turns on LED 3, waits 1.5 seconds, then turns all of them off at once.

4. **Starts the stopwatch.** The SysTick timer starts counting milliseconds from zero.

5. **Waits for the touch.** The code enters a tight `while (!Reaction_Measured)` loop and literally does nothing until the touch sensor triggers the `EXTI9_5_IRQHandler` interrupt.

6. **Records the time.** When the interrupt fires, it stops the SysTick timer and saves the count (in milliseconds) as the reaction time.

7. **Switches players.** If Player 1 just went, it is now Player 2's turn (and vice versa).

8. **Checks if the game is over.** If both players have played all their rounds, it calls `Declare_Winner()`, which calculates averages, prints all the results, and announces who won.

---

## Changing the Number of Rounds

Open `show_case.c` and look at line 16:

```c
#define ROUNDS_PER_GAME 1
```

Change the `1` to however many rounds you want each player to play. For example, for a best-of-5:

```c
#define ROUNDS_PER_GAME 5
```

Rebuild and re-flash the board. That is it. The game supports up to 10 rounds per player (because the time arrays are sized to hold 10 values).

Note: The game is hardcoded for exactly 2 players. You cannot change the number of players without rewriting parts of the code.

---

## Troubleshooting

### "I see nothing on the Bluetooth app"

- Make sure the HC-05 is **connected**, not just paired. Open the Bluetooth serial app and tap Connect.
- Double-check your TX/RX wiring. Remember: PA9 (TX) goes to HC-05 RXD, and PA10 (RX) goes to HC-05 TXD. A crossover.
- Press the reset button on your STM32 board. The welcome message is only sent once at startup, so if the phone was not connected at that point, you missed it. After resetting, connect quickly.

### "The LEDs do not light up"

- Check that the LED long leg (anode) faces the resistor/pin side and the short leg (cathode) faces GND.
- Make sure the resistors are actually connected (push them firmly into the breadboard).
- Confirm you are using PB6, PB7, and PB8 -- not some other pins.

### "I touch the sensor but nothing happens"

- Make sure the TTP223 SIG pin is connected to PA6 (not some other pin).
- Check that the TTP223 is powered (VCC to 3.3V, GND to GND).
- Try touching the sensor pad directly, not the edge of the module.
- Some TTP223 modules have a solder jumper to toggle between momentary and toggle mode. Make sure it is in momentary mode (usually the default).

### "Build errors in Keil"

- Make sure the STM32F4 Device Pack is installed.
- Make sure you selected the correct device when creating the project (STM32F401CCUx or STM32F401RETx).
- Make sure you checked CMSIS CORE and Device Startup in the Run-Time Environment.

### "Flash download failed"

- Check your ST-Link connections (SWDIO, SWCLK, GND).
- Make sure the ST-Link driver is installed. Keil usually installs it, but you can also download it from the ST website.
- Try holding the reset button on the board, clicking Download in Keil, and then releasing the reset button.

### "The reaction time seems way too fast or way too slow"

- If the time is always 0 ms, the touch sensor might be triggering before the game says "GO." Make sure nothing is touching the sensor during the LED countdown.
- If the time seems oddly large, the software delay loop calibration might differ on your board. The core logic (SysTick timer) should be accurate since it is based on the hardware clock, but the LED delays are approximate.
