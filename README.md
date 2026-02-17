# STM32 Reflex Battle

A bare-metal STM32F4 implementation of a competitive reaction time game. This project measures human response speed with millisecond precision using hardware interrupts and high-speed timers.

##  Project Versions
This repository contains two distinct implementations based on your preferred output method:

1.  **[LCD-Display-Version](./LCD-Version):** A standalone version that outputs results to an LCD display (via Keil Semihosting/Debug).
2.  **[Bluetooth-Mobile-Version](./Bluetooth-Version):** A wireless version that connects to a smartphone via a HC-05/HC-06 Bluetooth module for a modern UI.

##  Key Technical Features
- **Zero-Latency Triggers:** Uses `EXTI` (External Interrupts) on Pin PA6 for instantaneous response capture.
- **Precision Timing:** Configures the `SysTick` timer at 1ms intervals.
- **Direct Register Access:** No HAL libraries used; written entirely in CMSIS/Bare-Metal C for performance and small binary size.

---
