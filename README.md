# Real-Time Quality Monitoring System

This project is a **Real-Time Quality Monitoring System** designed for monitoring data from multiple sensors, logging the data into a CSV file, and performing visualization and analysis in MATLAB. The system supports multi-threaded data acquisition from serial ports and can handle multiple sensor types dynamically.

---

## Features

### 1. Sensor Data Acquisition
- Multi-threaded support for reading data from multiple serial ports (`COM3`, `COM4`, `COM5`).
- Real-time validation of sensor data to ensure accuracy and consistency.
- Logging of sensor readings into a `CSV` file for permanent storage.

### 2. Data Logging
- Stores sensor readings in a structured `sensor_data.csv` file.
- Each record includes:
  - **Port**: Serial port where the sensor is connected.
  - **SensorID**: Unique identifier for the sensor (e.g., `TEMP`, `PH`, `HUMIDITY`).
  - **Value**: Measured value.
  - **Timestamp**: Time of the measurement.

### 3. MATLAB Visualization
- Dynamically detects all unique sensor types in the dataset.
- Creates time-series plots for each sensor showing value trends over time.
- Highlights:
  - Average values for each sensor using horizontal lines.
  - Outliers that exceed predefined acceptable ranges.
- Saves all generated plots into a single image file (`sensor_plots.png`).

---

## File Structure

```plaintext
.
├── sensor_data.csv       # Logged sensor data (created by the program)
├── Quality_Monitoring.m  # MATLAB script for visualization and analysis
├── QualityMonitoring.c   # C code for real-time data acquisition and logging
├── README.md             # Project documentation
├── sensor_plots.png      # Saved visualization from MATLAB (output)
