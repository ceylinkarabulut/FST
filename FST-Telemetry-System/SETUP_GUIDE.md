# STM32 Nucleo Board Setup Guide
Please feel free to ask me anything if you need help!!! :3
This guide helps you configure the project for different Nucleo boards.

## Supported Boards

The code is compatible with most STM32 Nucleo boards. Common models:

- **NUCLEO-F103RB** (STM32F103RB - Cortex-M3)
- **NUCLEO-F401RE** (STM32F401RE - Cortex-M4) 
- **NUCLEO-F411RE** (STM32F411RE - Cortex-M4)
- **NUCLEO-L476RG** (STM32L476RG - Cortex-M4 Low-Power)
- **NUCLEO-G474RE** (STM32G474RE - Cortex-M4)

## Quick Reference Table

| Feature | Pin on Most Nucleos | Notes |
|---------|-------------------|-------|
| User LED | PA5 | Green LED, usually labeled LD2 |
| User Button | PC13 | Blue button labeled B1 |
| UART TX | PA2 (USART2) | Connected to ST-Link USB |
| UART RX | PA3 (USART2) | Connected to ST-Link USB |
| ADC Channel 0 | PA0 | For sensor input |
| ADC Channel 1 | PA1 | For sensor input |

## Step-by-Step Setup

### 1. Using STM32CubeMX (Recommended for Beginners)

1. **Create New Project**
   - Open STM32CubeMX
   - Click "New Project" or "Access to Board Selector"
   - Search for your Nucleo board (e.g., "NUCLEO-F401RE")
   - Click "Start Project"

2. **Configure Peripherals**

   **a) Enable ADC**
   - Find ADC1 in the left panel
   - Click to expand
   - Enable "IN0" through "IN7" (or as many as you need)
   - In Configuration tab → ADC1:
     - Resolution: 12 bits
     - Data Alignment: Right
     - Scan Conversion Mode: Disabled (we'll poll manually)

   **b) Enable UART**
   - Find USART2 in left panel
   - Mode: Asynchronous
   - Configuration → Parameter Settings:
     - Baud Rate: 115200
     - Word Length: 8 Bits
     - Stop Bits: 1
     - Parity: None

   **c) Enable Timer**
   - Find TIM2
   - Clock Source: Internal Clock
   - Configuration → Parameter Settings:
     - Prescaler: Calculate based on your clock (see below)
     - Counter Period: For 1 second interval

   **d) Configure GPIO**
   - LED is usually already configured
   - If not: PA5 → GPIO_Output

3. **Calculate Timer Values**

   Formula: `Update_Event = Clock / ((Prescaler + 1) * (Period + 1))`

   For 1Hz (1 second intervals):
   - If System Clock = 84 MHz (F401RE):
     - Prescaler = 8399 (8400 - 1)
     - Period = 9999 (10000 - 1)
     - Result: 84,000,000 / (8400 * 10000) = 1 Hz ✓

   - If System Clock = 72 MHz (F103RB):
     - Prescaler = 7199 (7200 - 1)
     - Period = 9999 (10000 - 1)

4. **Generate Code**
   - Click "Project Manager" tab
   - Project Name: "FST_Telemetry"
   - Toolchain: STM32CubeIDE
   - Click "GENERATE CODE"

5. **Add Your Code**
   - Open generated project in STM32CubeIDE
   - Replace `main.c` with my provided code


```ini
; For NUCLEO-F401RE
[env:nucleo_f401re]
platform = ststm32
board = nucleo_f401re
framework = stm32cube
upload_protocol = stlink
monitor_speed = 115200

; For NUCLEO-F103RB  
[env:nucleo_f103rb]
platform = ststm32
board = nucleo_f103rb
framework = stm32cube
upload_protocol = stlink
monitor_speed = 115200

; For NUCLEO-L476RG
[env:nucleo_l476rg]
platform = ststm32
board = nucleo_l476rg
framework = stm32cube
upload_protocol = stlink
monitor_speed = 115200
```

Commands:
```bash
# Build
pio run -e nucleo_f401re

# Upload to board
pio run -e nucleo_f401re --target upload

# Open serial monitor
pio device monitor
```

### 3. Board-Specific Adjustments

#### NUCLEO-F103RB (72 MHz)
```c
// In main.c, adjust TIM_Init():
htim2.Init.Prescaler = 7200 - 1;
htim2.Init.Period = 10000 - 1;
```

#### NUCLEO-F401RE (84 MHz) 
```c
// Default settings work!
htim2.Init.Prescaler = 8400 - 1;
htim2.Init.Period = 10000 - 1;
```

#### NUCLEO-L476RG (80 MHz, Low-Power)
```c
// In main.c, adjust TIM_Init():
htim2.Init.Prescaler = 8000 - 1;
htim2.Init.Period = 10000 - 1;

// Note: L4 series has different ADC configuration
// You may need to enable VREFINT in CubeMX
```

## Pin Mapping for Sensors

### Temperature Sensors (Task 1)
Connect up to 10 analog sensors to these pins:

| Sensor # | Nucleo Pin | ADC Channel |
|----------|-----------|-------------|
| 1 | PA0 | ADC_IN0 |
| 2 | PA1 | ADC_IN1 |
| 3 | PA4 | ADC_IN4 |
| 4 | PB0 | ADC_IN8 |
| 5 | PC1 | ADC_IN11 |
| 6 | PC0 | ADC_IN10 |
| 7 | PA6 | ADC_IN6 |
| 8 | PA7 | ADC_IN7 |
| 9 | PB1 | ADC_IN9 |
| 10 | PC2 | ADC_IN12 |

**Note:** Available channels vary by MCU. Check your datasheet pls :3.

## Testing Without Sensors

**Quick Test Setup:**
1. Flash the code
2. Leave ADC pins floating (no connections)
3. You'll read random noise voltages
4. This proves the code works!

**Better Test Setup:**
- Connect potentiometers to ADC pins
- Rotate to vary voltage 0-3.3V
- Watch values change in real-time

## Example Sensor Connections


## Getting Help

If you're stuck:

1. **Check your specific board's documentation:**
   - STM32 Nucleo User Manual (UM1724 for most boards)
   - MCU Datasheet (search "[your MCU] datasheet")

2. **Common Resources:**
   - [STM32 Community Forums](https://community.st.com/)
   - [STM32 Documentation](https://www.st.com/en/microcontrollers-microprocessors/stm32-32-bit-arm-cortex-mcus.html)

3. **Quick Checks:**
   - Does the LED blink? (Basic GPIO works)
   - Does UART echo back? (Communication works)
   - Do you see ANY ADC values? (ADC enabled)
   
