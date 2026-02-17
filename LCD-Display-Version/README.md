# STM32 Reaction Time Game

A fun reaction-time game built on an **STM32F401** microcontroller. LEDs count down like a traffic light, and the moment they all go dark — **you tap a touch sensor as fast as you can!** Your reaction time (in milliseconds) is displayed so you can see exactly how quick your reflexes are.

The game uses a **16×2 I2C LCD screen** to show messages and display your reaction time. Source code: `LCD-Display-Version.c`.

---

## Table of Contents

1. [What Does This Project Do?](#what-does-this-project-do)
2. [What You'll Need (Shopping List)](#what-youll-need-shopping-list)
3. [Wiring It Up](#wiring-it-up)
4. [Software Setup (Step-by-Step)](#software-setup-step-by-step)
5. [Creating Your Keil Project](#creating-your-keil-project)
6. [Adding the Code](#adding-the-code)
7. [Compiling (Building) the Code](#compiling-building-the-code)
8. [Uploading to the Board (Flashing)](#uploading-to-the-board-flashing)
9. [How to Play](#how-to-play)
10. [Pin Configuration Reference](#pin-configuration-reference)
11. [Troubleshooting](#troubleshooting)

---

## What Does This Project Do?

Think of it like the start of a race:

1. **Three LEDs light up one by one** — like a "Ready, Set..." countdown.
2. **All LEDs turn OFF at once** — that's your "GO!" signal.
3. **You touch the sensor** as fast as you can.
4. **The board measures your time** in milliseconds and displays it.

The faster you tap, the lower your score. Can you beat 200ms?

---

## What You'll Need (Shopping List)

Here's everything you need to buy/gather before starting. You can find all of these on Amazon, Robu.in, or any electronics store.

### Components

| # | Item | Why You Need It |
|---|------|-----------------|
| 1 | **STM32F401 Black Pill Board** (or STM32F407 Discovery) | This is the "brain" — the microcontroller that runs your code |
| 2 | **TTP223 Capacitive Touch Sensor Module** | The button you'll tap — it senses your finger without needing to press anything |
| 3 | **3 × LEDs** (any color — Red, Yellow, Green looks best!) | The countdown lights |
| 4 | **3 × 220Ω Resistors** (or 330Ω — both work) | These protect the LEDs from burning out |
| 5 | **Breadboard** | A board with holes to plug your wires into — no soldering needed! |
| 6 | **Jumper Wires** (Male-to-Male and Male-to-Female) | Wires to connect everything together |
| 7 | **Micro-USB Cable** | To connect the board to your computer for programming |
| 8 | **ST-Link V2 Debugger** (may be built into your board) | Used to upload code from your computer to the board |

| 9 | **16×2 LCD Display with I2C Backpack** (PCF8574 module) | The screen that shows messages like "GO!" and your reaction time |

> **Tip:** When buying the LCD, make sure it comes with the **I2C backpack already soldered on the back**. It's a small blue circuit board attached to the LCD. This saves you from needing 16 wires — you'll only need 4!

---

## Wiring It Up

### Understanding the Board's Pins

Your STM32 board has tiny labels printed next to each pin (like PA6, PB9, GND, 3.3V). These labels tell you which pin is which. Here's what the abbreviations mean:

- **PA6** = Port A, Pin 6
- **PB9** = Port B, Pin 9
- **GND** = Ground (the negative/return wire — like the minus side of a battery)
- **3.3V** = Power output (this powers your sensors and LEDs)

### LED Wiring

Each LED has two legs:
- **Long leg (+)** = Positive (called the "Anode")
- **Short leg (−)** = Negative (called the "Cathode")

Wire each LED like this:

```
STM32 Pin ──→ 220Ω Resistor ──→ LED (long leg) ──→ LED (short leg) ──→ GND
```

| LED | STM32 Pin | Color (suggested) | Role |
|-----|-----------|-------------------|------|
| LED 1 | **PC14** | Red | First countdown light |
| LED 2 | **PC15** | Yellow | Second countdown light |
| LED 3 | **PB9** | Green | Third countdown light |

### Touch Sensor Wiring

The TTP223 module has 3 pins:

| TTP223 Pin | Connect To | What It Does |
|------------|-----------|--------------|
| **VCC** | STM32 **3.3V** | Powers the sensor |
| **GND** | STM32 **GND** | Completes the circuit |
| **SIG** (or OUT) | STM32 **PA6** | Sends a signal when you touch it |

### LCD Wiring

The I2C LCD backpack has 4 pins:

| LCD Pin | Connect To | What It Does |
|---------|-----------|--------------|
| **VCC** | STM32 **5V** (or **3.3V** if 5V not available) | Powers the LCD |
| **GND** | STM32 **GND** | Ground connection |
| **SDA** | STM32 **PB7** | Data line (carries the text to display) |
| **SCL** | STM32 **PB6** | Clock line (keeps communication in sync) |

### Wiring Summary Diagram

```
                        STM32F401 Black Pill
                    ┌─────────────────────────┐
                    │                         │
      LED 1 (Red)◄──┤ PC14              PA6  ├──► Touch Sensor (SIG)
   LED 2 (Yellow)◄──┤ PC15              PB6  ├──► LCD SCL (clock)
    LED 3 (Green)◄──┤ PB9               PB7  ├──► LCD SDA (data)
                    │                         │
    All GNDs ──────►┤ GND               3.3V ├──► Touch VCC / LCD VCC
                    │                         │
                    └─────────────────────────┘

   Note: Each LED needs a 220Ω resistor between the STM32 pin and the LED.
```

---

## Software Setup (Step-by-Step)

You'll be using **Keil µVision5** — a free program that lets you write code for microcontrollers and upload it to the board.

### Step 1: Download Keil µVision5

1. Go to the [Keil Product Download Page](https://www.keil.com/download/product/).
2. Click on **MDK-ARM** (this is the version for ARM-based boards like STM32).
3. Fill in the short form (you can type "Student" or "Personal" for company name).
4. Check your email for the download link and click it.

### Step 2: Install the Software

1. **Right-click** the downloaded installer and select **"Run as administrator"**.
2. Click **Next** through the prompts. Use the default installation folder.
3. When it asks for company details, type **"Personal"** or **"Student"**.
4. Wait for it to finish — this may take 5–10 minutes.

> **Important:** Don't skip the "Run as administrator" step — the installer needs permission to set up drivers.

### Step 3: Install the STM32 Device Pack

After installation, the **Pack Installer** window will open automatically. If it doesn't, open Keil and go to **Pack → Pack Installer**.

1. Wait a moment for the device list to load (you'll see a loading bar).
2. In the **search box**, type **STM32F4**.
3. Find **Keil::STM32F4xx_DFP** in the list.
4. Click the **"Install"** button next to it.
5. Wait for it to download and install (this adds all the files Keil needs to understand your board).

> **What is a "Pack"?** Think of it as a plugin. Without it, Keil doesn't know what kind of board you have, so it can't help you write code for it.

For a complete visual walkthrough of these steps, refer to this video guide: [YouTube - Keil Setup Tutorial](https://youtu.be/TO85lArN1B8)

---

## Creating Your Keil Project

### Step 4: Create a New Project

1. Open Keil µVision5.
2. Go to **Project → New µVision Project...**
3. Choose a folder to save your project in (e.g., `Desktop/ReactionGame/`).
4. Name it something like `ReactionGame` and click **Save**.

### Step 5: Select Your Microcontroller

A window called **"Select Device for Target"** will pop up:

1. In the search box, type your exact board model:
   - For Black Pill: type **STM32F401CCU6**
   - For Discovery: type **STM32F407VG**
2. Click on your board in the list.
3. Click **OK**.

> **Double-check this!** If you pick the wrong chip, nothing will work. Check the text printed on the big black chip on your board — it will say something like `STM32F401CCU6`.

### Step 6: Configure the Runtime Environment

A window called **"Manage Run-Time Environment"** will appear. You need to check **two boxes**:

1. Expand **CMSIS** → check **CORE**
2. Expand **Device** → check **Startup**

Then click **OK**.

> **Why these two?** 
> - **CMSIS CORE** = gives you access to all the STM32's built-in features (timers, interrupts, GPIO).
> - **Startup** = tells the board how to start running your code when it powers on.

---

## Adding the Code

### Step 7: Add the Source File

1. In the **Project** panel on the left, you'll see **"Source Group 1"**.
2. **Right-click** on it → select **"Add Existing Files to Group 'Source Group 1'..."**
3. Navigate to where you downloaded/saved the `.c` file and select **`LCD-Display-Version.c`**.
4. Click **Add**, then **Close**.

You should now see the file appear under "Source Group 1" in the Project panel. Double-click it to open and view the code.

---

## Compiling (Building) the Code

"Compiling" means translating the C code you wrote into instructions the microcontroller can understand.

### Step 8: Build the Project

1. Go to **Project → Build Target** (or press **F7** on your keyboard).
2. Look at the **Build Output** panel at the bottom of the screen.
3. You want to see:

```
"ReactionGame.axf" - 0 Error(s), 0 Warning(s)
```

**If you see errors:**
- Make sure you selected the correct device (Step 5).
- Make sure you enabled CMSIS CORE and Startup (Step 6).
- Make sure you added the `.c` file to the project.

---

## Uploading to the Board (Flashing)

"Flashing" means sending the compiled code from your computer into the STM32 board's memory.

### Step 9: Configure the Debugger

1. Go to **Project → Options for Target** (or press **Alt + F7**).
2. Click the **Debug** tab at the top.
3. On the right side, select your debugger from the dropdown:
   - If using **ST-Link**: select **ST-Link Debugger**
   - If using **CMSIS-DAP**: select **CMSIS-DAP Debugger**
4. Click the **Settings** button next to the dropdown.
5. Under the **Flash Download** tab, make sure **"Reset and Run"** is checked. This makes the board automatically start running your code after uploading.
6. Click **OK** to close.

### Step 10: Upload (Flash) the Code

1. Connect your STM32 board to your computer via the **ST-Link or USB cable**.
2. Go to **Flash → Download** (or press **F8**).
3. Wait for the output panel to show:

```
"Erase Done."
"Programming Done."
"Verify OK."
```

**Your code is now on the board!**


---

## How to Play

1. **Power on** the board (plug in USB).
2. **Watch the LCD** — it will say `"Reaction Time"` → `"Game Ready!"`.
3. **Watch the LEDs** — they will light up one-by-one:
   - LED 1 (Red) turns ON
   - LED 2 (Yellow) turns ON
   - LED 3 (Green) turns ON
   - All three are ON together for a moment...
4. **All LEDs go OFF** and the screen says `"GO! Touch Now!"`.
5. **Touch the sensor on PA6 as fast as you can!**
6. **Your reaction time appears** on the LCD screen in milliseconds.

### Score Guide

| Reaction Time | Rating |
|---------------|--------|
| < 150 ms | Superhuman! |
| 150 – 250 ms | Excellent |
| 250 – 350 ms | Good |
| 350 – 500 ms | Average |
| > 500 ms | Keep practicing! |

> **To play again:** Press the **RESET button** on your STM32 board.

---

## Pin Configuration Reference

### Pin Map

| Pin | Port | Direction | Function | Details |
|-----|------|-----------|----------|---------|
| PA6 | GPIOA | Input | Touch Sensor (EXTI) | Rising-edge interrupt, no pull-up/pull-down |
| PB6 | GPIOB | Alt Function (AF4) | I2C1 SCL | Open-drain, internal pull-up, 100kHz clock to LCD |
| PB7 | GPIOB | Alt Function (AF4) | I2C1 SDA | Open-drain, internal pull-up, data line to LCD |
| PB9 | GPIOB | Output | LED 3 (Green) | Third countdown LED |
| PC14 | GPIOC | Output | LED 1 (Red) | First countdown LED |
| PC15 | GPIOC | Output | LED 2 (Yellow) | Second countdown LED; also turns ON when touch is detected |

**LCD Screen:**

| Parameter | Value |
|-----------|-------|
| Type | 16×2 Character LCD (HD44780) |
| Interface | I2C via PCF8574 backpack |
| I2C Address | `0x27` (try `0x3F` if `0x27` doesn't work) |
| Communication Speed | 100 kHz (Standard Mode) |
| Mode | 4-bit, 2 lines, 5×8 dot font |



### Clock & Peripheral Configuration

| Peripheral | Setting |
|------------|---------|
| System Clock | 16 MHz (HSI — internal oscillator) |
| SysTick Timer | 1 ms tick interval (used to measure reaction time) |
| EXTI Line 6 | Rising-edge trigger on PA6, through NVIC vector `EXTI9_5_IRQn` |
| SYSCFG Clock | Enabled for EXTI multiplexing |

---

## Troubleshooting

### "No target connected" when flashing
- Make sure the USB cable is plugged in properly.
- Try a different USB port.
- Check that you selected the right debugger in Step 9.

### LEDs don't light up
- Check your wiring — make sure the **long leg** of the LED goes towards the STM32 pin (through the resistor).
- Make sure GND is connected.
- Try a different LED to rule out a dead one.

### Touch sensor doesn't respond
- Check that VCC goes to 3.3V and GND goes to GND.
- Make sure SIG/OUT goes to **PA6**.
- Try touching the sensor — some need a firm touch, others are very sensitive.

### LCD screen is blank
- **Adjust the contrast:** On the back of the LCD's I2C backpack, there's a small blue knob (potentiometer). Turn it slowly with a small screwdriver until you see text.
- **Check the I2C address:** Most LCDs use address `0x27`, but some use `0x3F`. If your LCD doesn't respond, change `#define LCD_I2C_ADDR 0x27` to `0x3F` in the code and re-flash.
- Make sure SDA goes to **PB7** and SCL goes to **PB6**.

### Build errors in Keil
- Make sure you installed the **STM32F4 DFP Pack** (Step 3).
- Make sure you enabled **CMSIS → CORE** and **Device → Startup** (Step 6).
- Make sure the `.c` source file is added to your project.



---

## Project Files

| File | Description |
|------|-------------|
| `LCD-Display-Version.c` | Full game code with I2C LCD output — messages and reaction time shown on the 16×2 screen |

---



*Built using bare-metal C on STM32F401*
