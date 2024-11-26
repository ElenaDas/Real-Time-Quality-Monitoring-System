#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <process.h>

// *** SensorData Structure ***
// This structure is used to hold data for a single sensor.
// It includes:
// - `id`: A string that uniquely identifies the sensor (e.g., "TEMP", "HUMIDITY").
// - `value`: A floating-point value representing the sensor's measurement.
typedef struct {
    char id[10];      // Sensor identifier
    float value;      // Measured value
} SensorData;

// *** SensorStats Structure ***
// This structure is used to maintain statistics for a sensor.
// It tracks:
// - `min_limit` and `max_limit`: Define the acceptable range of values for the sensor.
// - `total_value`: Sum of all recorded values for calculating the average.
// - `max_value` and `min_value`: The maximum and minimum values observed.
// - `count`: Number of recorded values, used for calculating the average.
typedef struct {
    float min_limit;   // Minimum acceptable limit
    float max_limit;   // Maximum acceptable limit
    float total_value; // Total value for averaging
    float max_value;   // Maximum recorded value
    float min_value;   // Minimum recorded value
    int count;         // Number of recorded values
} SensorStats;

// *** SerialPortInfo Structure ***
// This structure holds information about a serial port.
// It includes:
// - `port_name`: The name of the serial port (e.g., "COM3").
// - `hSerial`: The handle to the serial port, used for communication.
typedef struct {
    char port_name[10]; // Serial port name
    HANDLE hSerial;     // Handle to the serial port
} SerialPortInfo;

// *** Function: setup_serial ***
// This function initializes and configures a serial port for communication.
// Steps:
// 1. Open the serial port using the `CreateFile` function.
// 2. Retrieve the current configuration of the serial port.
// 3. Set the baud rate, data bits, stop bits, and parity settings.
// 4. Return a handle to the configured serial port.
//
// Parameters:
// - `port_name`: The name of the serial port to configure (e.g., "COM3").
// - `baud_rate`: The communication speed (e.g., 9600 bits per second).
//
// Returns:
// - A handle to the configured serial port, or NULL if an error occurs.
HANDLE setup_serial(const char *port_name, DWORD baud_rate) {
    HANDLE hSerial = CreateFile(port_name, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
    if (hSerial == INVALID_HANDLE_VALUE) {
        printf("[ERROR] Unable to open serial port %s\n", port_name);
        return NULL;
    }

    // Configure the serial port parameters
    DCB dcbSerialParams = {0};
    dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
    if (!GetCommState(hSerial, &dcbSerialParams)) {
        printf("[ERROR] Failed to get serial port state %s\n", port_name);
        CloseHandle(hSerial);
        return NULL;
    }

    // Setting up the serial port communication parameters
    dcbSerialParams.BaudRate = baud_rate; // Communication speed
    dcbSerialParams.ByteSize = 8;         // 8 data bits per byte
    dcbSerialParams.StopBits = ONESTOPBIT; // Use one stop bit
    dcbSerialParams.Parity = NOPARITY;    // No parity check

    if (!SetCommState(hSerial, &dcbSerialParams)) {
        printf("[ERROR] Failed to set serial port state %s\n", port_name);
        CloseHandle(hSerial);
        return NULL;
    }

    // Return the handle to the configured serial port
    return hSerial;
}

// *** Function: validate_data ***
// This function checks if the sensor data is valid.
// It performs the following checks:
// 1. The sensor ID should not be empty.
// 2. The sensor value should be within a reasonable range (0 to 1000).
//
// Parameters:
// - `sensor`: Pointer to the SensorData structure to validate.
//
// Returns:
// - 1 if the data is valid, 0 otherwise.
int validate_data(SensorData *sensor) {
    if (strlen(sensor->id) == 0) { // Check if the sensor ID is empty
        printf("[ERROR] Sensor ID is empty.\n");
        return 0;
    }
    if (sensor->value < 0 || sensor->value > 1000) { // Ensure the value is within realistic limits
        printf("[ERROR] Sensor value out of realistic range: %.2f\n", sensor->value);
        return 0;
    }
    return 1; // Data is valid
}

// *** Function: log_to_csv ***
// This function logs sensor data to a CSV file.
// Each line in the file represents a single sensor reading, formatted as:
// Port Name, Sensor ID, Sensor Value
//
// Parameters:
// - `filename`: The name of the CSV file to log data.
// - `port_name`: The serial port from which the data was received.
// - `sensor`: Pointer to the SensorData structure containing the data.
void log_to_csv(const char *filename, const char *port_name, SensorData *sensor) {
    FILE *file = fopen(filename, "a");
    if (file == NULL) {
        printf("[ERROR] Unable to open file %s for logging.\n", filename);
        return;
    }

    // Write the sensor data to the CSV file
    fprintf(file, "%s,%s,%.2f\n", port_name, sensor->id, sensor->value);
    fclose(file); // Close the file after writing
}

// *** Function: monitor_quality ***
// This function monitors sensor data to ensure it stays within defined limits.
// It updates sensor statistics (total, min, max) and issues alerts if values are out of range.
//
// Parameters:
// - `sensor`: Pointer to the SensorData structure containing the latest reading.
// - `stats`: Pointer to the SensorStats structure to update statistics.
// - `port_name`: The serial port from which the data was received.
void monitor_quality(SensorData *sensor, SensorStats *stats, const char *port_name) {
    stats->total_value += sensor->value; // Add to total value for averaging
    stats->count++;                      // Increment the count of readings
    if (sensor->value > stats->max_value) stats->max_value = sensor->value; // Update max value
    if (sensor->value < stats->min_value) stats->min_value = sensor->value; // Update min value

    // Check if the value is out of defined limits
    if (sensor->value < stats->min_limit || sensor->value > stats->max_limit) {
        printf("[ALERT] %s out of range on %s! Value: %.2f (Limits: %.2f - %.2f)\n",
               sensor->id, port_name, sensor->value, stats->min_limit, stats->max_limit);
    }
}

// *** Function: read_serial_thread ***
// This function runs as a separate thread to read data from a specific serial port.
// It performs the following steps:
// 1. Continuously reads data from the serial port.
// 2. Parses the data into a SensorData structure.
// 3. Validates the data and logs it to a CSV file.
// 4. Monitors the quality of the sensor data.
//
// Parameters:
// - `args`: Pointer to the SerialPortInfo structure for the serial port.
//
// Returns:
// - 0 when the thread terminates.
unsigned __stdcall read_serial_thread(void *args) {
    SerialPortInfo *port_info = (SerialPortInfo *)args;
    char buffer[256];
    SensorData sensor;

    // Initialize sensor statistics with default limits
    SensorStats stats = {5.0, 25.0, 0.0, -1000.0, 1000.0, 0};

    while (1) {
        DWORD bytes_read;
        if (ReadFile(port_info->hSerial, buffer, sizeof(buffer) - 1, &bytes_read, NULL)) {
            buffer[bytes_read] = '\0'; // Null-terminate the buffer
            if (sscanf(buffer, "%s %f", sensor.id, &sensor.value) == 2 && validate_data(&sensor)) {
                printf("[%s] Sensor: %s, Value: %.2f\n", port_info->port_name, sensor.id, sensor.value);
                log_to_csv("sensor_data.csv", port_info->port_name, &sensor); // Log data to CSV
                monitor_quality(&sensor, &stats, port_info->port_name);      // Monitor quality and issue alerts
            } else {
                printf("[ERROR] Invalid data format: %s\n", buffer);
            }
        } else {
            printf("[ERROR] Failed to read from port %s\n", port_info->port_name);
            break;
        }
        Sleep(1000); // Wait for 1 second before reading again
    }
    CloseHandle(port_info->hSerial); // Close the serial port
    free(port_info);                 // Free allocated memory
    return 0;
}

// *** Function: main ***
// The main function sets up and starts threads for monitoring multiple serial ports.
// Steps:
// 1. Define the serial ports to monitor (e.g., "COM3", "COM4", "COM5").
// 2. Create threads for each port to read sensor data.
// 3. Wait for all threads to finish.
//
// Returns:
// - 0 when the program completes successfully.
int main() {
    const char *ports[] = {"COM3", "COM4", "COM5"}; // List of serial ports
    int num_ports = sizeof(ports) / sizeof(ports[0]); // Number of ports
    HANDLE threads[num_ports];

    // Loop through each port and create a thread for monitoring
    for (int i = 0; i < num_ports; i++) {
        SerialPortInfo *port_info = (SerialPortInfo *)malloc(sizeof(SerialPortInfo));
        strcpy(port_info->port_name, ports[i]);

        // Set up the serial port
        port_info->hSerial = setup_serial(port_info->port_name, CBR_9600);
        if (port_info->hSerial == NULL) {
            free(port_info);
            continue;
        }

        // Create a thread for the port
        threads[i] = (HANDLE)_beginthreadex(NULL, 0, read_serial_thread, port_info, 0, NULL);
        if (threads[i] == NULL) {
            printf("[ERROR] Unable to create thread for port %s\n", ports[i]);
            CloseHandle(port_info->hSerial);
            free(port_info);
        }
    }

    // Wait for all threads to complete
    WaitForMultipleObjects(num_ports, threads, TRUE, INFINITE);
    printf("All threads finished.\n");
    return 0;
}




