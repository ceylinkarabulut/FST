# 🏎️ FST Telemetry System Demo

**A professional embedded systems implementation for Formula Student Team sensor monitoring**

[![STM32](https://img.shields.io/badge/STM32-Nucleo-blue.svg)](https://www.st.com/en/evaluation-tools/stm32-nucleo-boards.html)
[![License](https://img.shields.io/badge/license-MIT-green.svg)](LICENSE)
---

## Overview

This project demonstrates **real-world embedded systems engineering** by implementing course assignments (sensor monitoring tasks) with actual STM32 hardware integration. Instead of just solving the programming exercises, this project shows how these concepts work in a real Formula Student telemetry system.

### What Makes This Different?

| Basic Assignment | This Project |
|-----------------|--------------|
| `scanf()` for input | **Real ADC readings** from hardware |
| `printf()` for output | **UART telemetry** in CSV format |
| Static calculations | **Real-time monitoring** at 1Hz |
| Console application | **Hardware alerts** with LED indicators |
| No visualization | **Python live plotting** and data logging |

---

## Features

### Task 1: Temperature Sensor Array Monitoring
- Reads 10 temperature sensors via ADC channels
- Converts raw ADC values to actual temperatures (LM35 compatible)
- Calculates average, maximum, and minimum in real-time
- Sends data over UART for logging and visualization

### Task 2: Digital Number Analyzer
- Utility function for digit manipulation
- Calculates sum, count, and reversal of numbers
- Useful for CAN ID parsing or telemetry data processing

### Task 3: Battery Cell Voltage Monitor
- Monitors 8 battery cells via ADC
- Detects cells below 3.3V threshold
- **Visual LED alerts** when critical voltage detected
- Reports min/max voltages and cell balance

### Bonus Features
-  **Real-time Python visualization** with matplotlib
-  **Automatic data logging** to CSV files
- **Hardware alerts** using onboard LED
- **UART telemetry** ready for CAN bus integration
- **Timer-based sampling** at configurable intervals

---

## Hardware Requirements

### Minimum Setup (What You Need)
- **STM32 Nucleo board** (any model: F103, F401, L476, etc.)
- **USB cable** (for programming and UART communication)
- **PC** with serial terminal or Python

### Optional Enhancements
- **LM35 temperature sensors** (or any analog sensor)
- **Potentiometers** (for simulating varying sensor inputs)
- **Voltage dividers** (for battery cell simulation)
- **Jumper wires** and breadboard

---

## Wiring Diagram (the best I can provide right noe :(( )

### Basic Setup (Using Nucleo's Internal ADC)

```
STM32 Nucleo Board
┌─────────────────────────────────────┐
│                                     │
│  PA0 (ADC_IN0) ──┐                  │
│  PA1 (ADC_IN1) ──┤                  │
│  PA4 (ADC_IN4) ──┼── Connect to     │
│  ...            ─┘   sensor or      │
│                      potentiometer  │
│                                     │
│  PA5 ────────────── LED (onboard)   │
│                                     │
│  PA2/PA3 (USART2) ── USB (ST-Link)  │
│                      for telemetry  │
└─────────────────────────────────────┘
```

**Notes:**
- Most Nucleo boards have onboard LED on PA5
- USART2 (PA2/PA3) is connected to ST-Link USB for easy PC communication
- Adjust ADC channel pins based on your specific Nucleo model
- Use voltage dividers if monitoring >3.3V (like real battery cells)

---

## Software Setup (yay!)

### 1. STM32 Development Environment

**Option A: Using STM32CubeIDE (Recommended)**
```bash
# Download from: https://www.st.com/en/development-tools/stm32cubeide.html

# Create new project for your Nucleo board
# Copy main.c into Core/Src/main.c
# Build and flash to board
```

**Option B: Using PlatformIO**
```bash
# Install PlatformIO
pip install platformio

# Create project
pio init --board nucleo_f401re  # Replace with your board

# Copy main.c to src/
# Build and upload
pio run --target upload
```

### 2. Python Visualizer Setup

```bash
# Install dependencies
pip install pyserial matplotlib numpy

# Run visualizer (replace COM3 with your port)
python telemetry_visualizer.py COM3        # Windows
python telemetry_visualizer.py /dev/ttyACM0  # Linux
```

**Finding Your Serial Port:**
- **Windows:** Check Device Manager → Ports (COM & LPT)
- **Linux:** Run `ls /dev/ttyACM*` or `ls /dev/ttyUSB*`
- **Mac:** Run `ls /dev/tty.usb*`

---

## Quick Start Guide

### Step 1: Flash the STM32
1. Open `main.c` in STM32CubeIDE
2. Select your Nucleo board model
3. Build the project
4. Connect Nucleo via USB
5. Flash to board

### Step 2: Connect Sensors (Optional)
- Connect temperature sensors or potentiometers to ADC pins
- For testing, you can run without sensors (will read floating voltages)

### Step 3: View Telemetry
**Option A: Serial Terminal**
```bash
# Use any serial terminal at 115200 baud
# You'll see CSV data like:
Time(ms),AvgTemp(C),MaxTemp(C),MinTemp(C),BattMin(V),BattMax(V),LowCells,Alert
1000,25.34,28.12,22.45,3.45,3.67,0,0
2000,25.89,28.34,23.01,3.42,3.65,0,0
```

**Option B: Python Visualizer (Recommended)**
```bash
python telemetry_visualizer.py COM3
```
You'll see real-time graphs updating!

---

## Example Output

### Serial Terminal Output
```
=== FST Telemetry System v1.0 ===
Initializing sensors...

Time(ms),AvgTemp(C),MaxTemp(C),MinTemp(C),BattMin(V),BattMax(V),LowCells,Alert
1000,24.56,27.34,22.11,3.45,3.67,0,0
CELLS: 3.45V 3.52V 3.48V 3.61V 3.55V 3.49V 3.58V 3.67V

2000,24.78,27.45,22.34,3.44,3.66,0,0
CELLS: 3.44V 3.51V 3.47V 3.60V 3.54V 3.48V 3.57V 3.66V

⚠️  LOW BATTERY ALERT! Cell voltage below threshold!
3000,25.01,27.56,22.56,3.21,3.65,1,1
CELLS: 3.21V 3.50V 3.46V 3.59V 3.53V 3.47V 3.56V 3.65V
```

### Visual Output (Python)
The visualizer shows three graphs:
1. **Temperature Trends** - Average, min, max over time with shaded range
2. **Battery Voltage** - Min/max cell voltages with alert threshold
3. **Cell Status Bar Chart** - Individual cell voltages (green = healthy, red = low)

---

## 🔧 Customization Guide

### Adjusting Sampling Rate
```c
// In main.c, line 21
#define SAMPLING_RATE_MS 1000  // Change to 500 for 2Hz, 2000 for 0.5Hz
```

### Changing Battery Threshold
```c
// In main.c, line 20
#define BATTERY_LOW_THRESHOLD 3.3f  // Change to your desired voltage
```

### Adding More Sensors
```c
// In main.c, line 18
#define NUM_TEMP_SENSORS 10  // Increase for more sensors
#define NUM_BATTERY_CELLS 8  // Increase for more cells
```

### Adapting for Your Nucleo Board
- Check your board's pinout datasheet
- Adjust ADC channels in `Task1_ReadTemperatures()` and `Task3_ReadBatteryVoltages()`
- Update LED pin in `GPIO_Init()` if not on PA5
- Modify clock configuration in `SystemClock_Config()`

---

##  Real-World FST Application

### How This Relates to Formula Student

In a real Formula Student car, this type of system would:

1. **Temperature Monitoring**
   - Motor temperature
   - Inverter temperature  
   - Brake disc temperature
   - Accumulator temperature

2. **Battery Management**
   - Individual cell voltage monitoring
   - Cell balancing detection
   - Safety threshold alerts
   - State of charge estimation

3. **Data Communication**
   - CAN bus integration (replace UART)
   - Real-time dashboard display
   - Pit crew telemetry
   - Data logging for post-race analysis


## What I Learned

This project demonstrates understanding of:

-  **Real-time embedded systems** - Timer interrupts, periodic sampling
-  **Hardware interfacing** - ADC, UART, GPIO peripherals  
-  **Data acquisition** - Sensor reading, signal conditioning
-  **System architecture** - Modular code, separation of concerns
-  **Telemetry systems** - Data formatting, transmission protocols
-  **Visualization** - Python integration, real-time plotting
-  **Safety-critical design** - Threshold monitoring, visual alerts
-  **Professional practices** - Documentation, version control, testing

---

## 📂 Project Structure

```
FST-Telemetry-Demo/
├── README.md                 # This file
├── main.c                    # STM32 application code
├── telemetry_visualizer.py   # Python real-time plotter
├── requirements.txt          # Python dependencies
└── LICENSE                   # MIT License
```

---

## 🤝 Contributing

Found a bug or have an improvement? Feel free to:
1. Fork this repository
2. Create a feature branch
3. Submit a pull request

Or open an issue for discussion!

---

## License

This project is open source under the MIT License. See LICENSE file for details.

---

## Acknowledgments

- Formula Student Team for the inspiration
- STMicroelectronics for excellent development tools
- Course assignments that formed the foundation
- Open source community for Python visualization libraries

---

## 📧 Contact

Questions? Improvements? Let's discuss!

**Your Name** - Formula Student Software Team
- GitHub: [@ceylinkarabulut](https://github.com/ceylinkarabulut)
- Email: ceylinkarabulut@std.ieu.edu.tr

---

## Future Enhancements ( I mean I hope so :( )

- [ ] CAN bus integration for multi-ECU communication
- [ ] SD card logging for offline data analysis
- [ ] Web dashboard using ESP32 WiFi bridge
- [ ] Integration with racing simulation software
- [ ] Kalman filtering for noise reduction
- [ ] Predictive battery life algorithm
- [ ] Multi-language support for international teams

---

**For Formula Student IEU**
