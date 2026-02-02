# Reaction Timer - Bluetooth & Mobile Edition

A competitive, two-player version of the game that communicates wirelessly with a smartphone. It includes player turn tracking and average score calculations.

### 🔌 Hardware Setup
- **Bluetooth Module:** HC-05 or HC-06 connected to **PA9 (TX)** and **PA10 (RX)**.
- **Input:** Touch Sensor or Push Button on **PA6**.
- **Visuals:** 3 LEDs connected to **PB6, PB7, and PB8**.



### 📱 Mobile Setup
1. Download a "Bluetooth Terminal" app from the Play Store or App Store.
2. Pair your phone with the HC-05/06 module.
3. Set the baud rate to **115200**.

### 🎮 Game Rules
1. Open the terminal and type **'start'** to begin.
2. The game will prompt **Player 1** to get ready.
3. After the LED sequence, tap the sensor.
4. The game automatically switches to **Player 2**.
5. After all rounds are complete, the app calculates the average and **declares a winner**!

### 📡 Serial Commands
- `start` / `START`: Begins a new match or the next round.
