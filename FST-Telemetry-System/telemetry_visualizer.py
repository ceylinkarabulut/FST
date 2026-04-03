#!/usr/bin/env python3
"""
FST Telemetry Visualizer
========================
Real-time data visualization for STM32 telemetry system
Displays temperature and battery data with alerts

Requirements:
    pip install pyserial matplotlib numpy

Usage:
    python telemetry_visualizer.py [COM_PORT]
    Example: python telemetry_visualizer.py COM3
    Linux: python telemetry_visualizer.py /dev/ttyACM0
"""

import serial
import matplotlib.pyplot as plt
import matplotlib.animation as animation
from matplotlib.patches import Rectangle
from collections import deque
import numpy as np
import sys
import time
from datetime import datetime

# Configuration
SERIAL_PORT = sys.argv[1] if len(sys.argv) > 1 else 'COM3'  # Change for your system
BAUD_RATE = 115200
MAX_POINTS = 100  # Number of data points to display
UPDATE_INTERVAL = 100  # ms

# Data storage
timestamps = deque(maxlen=MAX_POINTS)
avg_temps = deque(maxlen=MAX_POINTS)
max_temps = deque(maxlen=MAX_POINTS)
min_temps = deque(maxlen=MAX_POINTS)
batt_mins = deque(maxlen=MAX_POINTS)
batt_maxs = deque(maxlen=MAX_POINTS)
low_cell_counts = deque(maxlen=MAX_POINTS)
alerts = deque(maxlen=MAX_POINTS)

# Battery cell voltages (last reading)
cell_voltages = [0.0] * 8

class TelemetryVisualizer:
    def __init__(self):
        self.serial_port = None
        self.fig = None
        self.axes = None
        self.start_time = None
        self.data_log = []
        
    def connect_serial(self):
        """Connect to the STM32 board"""
        try:
            print(f"Connecting to {SERIAL_PORT} at {BAUD_RATE} baud...")
            self.serial_port = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)
            time.sleep(2)  # Wait for connection to establish
            print("Connected successfully :3")
            
            # Flush any existing data
            self.serial_port.flushInput()
            return True
        except serial.SerialException as e:
            print(f"Error connecting to serial port: {e}")
            print("\nAvailable ports:")
            import serial.tools.list_ports
            ports = serial.tools.list_ports.comports()
            for port in ports:
                print(f"  - {port.device}: {port.description}")
            return False
    
    def parse_csv_line(self, line):
        """Parse CSV telemetry data"""
        try:
            parts = line.strip().split(',')
            if len(parts) == 8 and parts[0].isdigit():
                data = {
                    'timestamp': float(parts[0]),
                    'avg_temp': float(parts[1]),
                    'max_temp': float(parts[2]),
                    'min_temp': float(parts[3]),
                    'batt_min': float(parts[4]),
                    'batt_max': float(parts[5]),
                    'low_cells': int(parts[6]),
                    'alert': int(parts[7])
                }
                return data
        except (ValueError, IndexError):
            pass
        return None
    
    def parse_cell_data(self, line):
        """Parse individual cell voltage data"""
        global cell_voltages
        try:
            if line.startswith("CELLS:"):
                voltages_str = line.replace("CELLS:", "").strip().split()
                voltages = [float(v.replace('V', '')) for v in voltages_str]
                if len(voltages) == 8:
                    cell_voltages = voltages
        except (ValueError, IndexError):
            pass
    
    def update_plot(self, frame):
        """Animation update function"""
        if self.serial_port is None or not self.serial_port.is_open:
            return
        
        # Read available data
        while self.serial_port.in_waiting:
            try:
                line = self.serial_port.readline().decode('utf-8', errors='ignore')
                
                # Parse CSV data
                data = self.parse_csv_line(line)
                if data:
                    # Store data
                    if self.start_time is None:
                        self.start_time = data['timestamp']
                    
                    rel_time = (data['timestamp'] - self.start_time) / 1000.0  # Convert to seconds
                    
                    timestamps.append(rel_time)
                    avg_temps.append(data['avg_temp'])
                    max_temps.append(data['max_temp'])
                    min_temps.append(data['min_temp'])
                    batt_mins.append(data['batt_min'])
                    batt_maxs.append(data['batt_max'])
                    low_cell_counts.append(data['low_cells'])
                    alerts.append(data['alert'])
                    
                    # Log data
                    self.data_log.append({
                        'time': datetime.now().strftime('%Y-%m-%d %H:%M:%S'),
                        'data': data
                    })
                
                # Parse cell data
                self.parse_cell_data(line)
                
            except Exception as e:
                print(f"Error reading data: {e}")
        
        # Update plots if we have data
        if len(timestamps) > 0:
            self.update_temperature_plot()
            self.update_battery_plot()
            self.update_cell_bars()
    
    def update_temperature_plot(self):
        """Update temperature graph"""
        self.axes[0].clear()
        
        t = list(timestamps)
        avg = list(avg_temps)
        maxv = list(max_temps)
        minv = list(min_temps)
        
        self.axes[0].plot(t, avg, 'b-', label='Average', linewidth=2)
        self.axes[0].plot(t, maxv, 'r--', label='Maximum', linewidth=1)
        self.axes[0].plot(t, minv, 'c--', label='Minimum', linewidth=1)
        self.axes[0].fill_between(t, minv, maxv, alpha=0.2, color='gray')
        
        self.axes[0].set_xlabel('Time (seconds)')
        self.axes[0].set_ylabel('Temperature (°C)')
        self.axes[0].set_title('Temperature Sensor Array Monitoring')
        self.axes[0].legend(loc='upper left')
        self.axes[0].grid(True, alpha=0.3)
    
    def update_battery_plot(self):
        """Update battery voltage graph"""
        self.axes[1].clear()
        
        t = list(timestamps)
        bmax = list(batt_maxs)
        bmin = list(batt_mins)
        alert = list(alerts)
        
        self.axes[1].plot(t, bmax, 'g-', label='Max Cell', linewidth=2)
        self.axes[1].plot(t, bmin, 'orange', label='Min Cell', linewidth=2)
        self.axes[1].axhline(y=3.3, color='r', linestyle='--', label='Low Threshold (3.3V)')
        
        # Highlight alert regions
        for i in range(len(t)):
            if alert[i] == 1:
                self.axes[1].axvspan(t[i]-0.5, t[i]+0.5, alpha=0.3, color='red')
        
        self.axes[1].set_xlabel('Time (seconds)')
        self.axes[1].set_ylabel('Voltage (V)')
        self.axes[1].set_title('Battery Cell Voltage Monitoring')
        self.axes[1].legend(loc='upper left')
        self.axes[1].grid(True, alpha=0.3)
        self.axes[1].set_ylim([2.5, 4.0])
    
    def update_cell_bars(self):
        """Update individual cell voltage bars"""
        self.axes[2].clear()
        
        cells = range(1, 9)
        colors = ['green' if v >= 3.3 else 'red' for v in cell_voltages]
        
        bars = self.axes[2].bar(cells, cell_voltages, color=colors, alpha=0.7, edgecolor='black')
        self.axes[2].axhline(y=3.3, color='r', linestyle='--', linewidth=2, label='Low Threshold')
        
        # Add voltage labels on bars
        for i, (cell, voltage) in enumerate(zip(cells, cell_voltages)):
            self.axes[2].text(cell, voltage + 0.05, f'{voltage:.2f}V', 
                            ha='center', va='bottom', fontsize=9, fontweight='bold')
        
        self.axes[2].set_xlabel('Cell Number')
        self.axes[2].set_ylabel('Voltage (V)')
        self.axes[2].set_title('Individual Battery Cell Status')
        self.axes[2].set_ylim([0, 4.0])
        self.axes[2].set_xticks(cells)
        self.axes[2].legend()
        self.axes[2].grid(True, alpha=0.3, axis='y')
    
    def create_plots(self):
        """Initialize plot window"""
        self.fig, self.axes = plt.subplots(3, 1, figsize=(12, 10))
        self.fig.suptitle('FST Telemetry System - Real-Time Monitoring', 
                         fontsize=16, fontweight='bold')
        plt.tight_layout(rect=[0, 0.03, 1, 0.97])
        
        # Configure closing behavior
        self.fig.canvas.mpl_connect('close_event', self.on_close)
    
    def on_close(self, event):
        """Handle window close"""
        self.save_log()
        if self.serial_port and self.serial_port.is_open:
            self.serial_port.close()
        print("\nConnection closed. Data saved to telemetry_log.csv")
    
    def save_log(self):
        """Save collected data to CSV file"""
        if len(self.data_log) == 0:
            return
        
        filename = f"telemetry_log_{datetime.now().strftime('%Y%m%d_%H%M%S')}.csv"
        
        with open(filename, 'w') as f:
            f.write("DateTime,Timestamp(ms),AvgTemp(C),MaxTemp(C),MinTemp(C),BattMin(V),BattMax(V),LowCells,Alert\n")
            for entry in self.data_log:
                data = entry['data']
                f.write(f"{entry['time']},{data['timestamp']},{data['avg_temp']:.2f},"
                       f"{data['max_temp']:.2f},{data['min_temp']:.2f},"
                       f"{data['batt_min']:.2f},{data['batt_max']:.2f},"
                       f"{data['low_cells']},{data['alert']}\n")
        
        print(f"Saved {len(self.data_log)} data points to {filename}")
    
    def run(self):
        """Main execution loop"""
        if not self.connect_serial():
            return
        
        print("\nStarting visualization...")
        print("Close the plot window to stop and save data.\n")
        
        self.create_plots()
        
        # Start animation
        ani = animation.FuncAnimation(self.fig, self.update_plot, 
                                     interval=UPDATE_INTERVAL, blit=False)
        
        try:
            plt.show()
        except KeyboardInterrupt:
            print("\nStopped by user")
        finally:
            self.on_close(None)

if __name__ == "__main__":
    print("=" * 60)
    print("FST Telemetry Visualizer")
    print("=" * 60)
    
    visualizer = TelemetryVisualizer()
    visualizer.run()
