# Reaction Timer - LCD/Debug Edition

This version is designed for users who want a direct hardware-to-display experience. It uses the STM32's internal debug interface to output data.

### 🔌 Hardware Setup
- **Microcontroller:** STM32F401/F411 Discovery or Nucleo.
- **Input:** Touch Sensor or Push Button connected to **PA6**.
- **Visuals:** 3 LEDs connected to **PA9, PA10, and PA11**.
- **Display:** Results are output via `printf` to the IDE Serial/LCD console (Semihosting).

### 🎮 How to Play
1. Watch the LED sequence (PA9 -> PA10 -> PA11).
2. When all LEDs turn **OFF**, touch the sensor on **PA6**.
3. Your reaction time will appear on the debug console in milliseconds.
