/* ============================================================================
 * SIMPLIFIED TEST VERSION - No Hardware Required
 * ============================================================================
 * This version simulates sensor data for testing the logic without STM32 hardware
 * Compile and run on PC to verify algorithms before deploying to hardware
 * 
 * Compile: gcc test_version.c -o test_version
 * Run: ./test_version
 * ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#define NUM_TEMP_SENSORS 10
#define NUM_BATTERY_CELLS 8
#define BATTERY_LOW_THRESHOLD 3.3f

typedef struct {
    float temperatures[NUM_TEMP_SENSORS];
    float avg_temp;
    float max_temp;
    float min_temp;
} TempSensorData;

typedef struct {
    float voltages[NUM_BATTERY_CELLS];
    float max_voltage;
    float min_voltage;
    int low_cell_count;
    int critical_alert;
} BatteryData;

typedef struct {
    unsigned int number;
    unsigned int digit_sum;
    int digit_count;
    unsigned int reversed;
} NumberAnalysis;

// Function prototypes
void simulate_temperature_readings(TempSensorData* data);
void analyze_temperatures(TempSensorData* data);
void simulate_battery_readings(BatteryData* data);
void analyze_battery(BatteryData* data);
void analyze_number(unsigned int number, NumberAnalysis* result);
void print_telemetry(TempSensorData* temp, BatteryData* batt, int iteration);

int main(void)
{
    TempSensorData temp_data = {0};
    BatteryData battery_data = {0};
    NumberAnalysis num_analysis = {0};
    
    srand(time(NULL));
    
    printf("\n");
    printf("╔════════════════════════════════════════════════════════════╗\n");
    printf("║     FST Telemetry System - Simulation Mode                ║\n");
    printf("║     Testing algorithms without hardware                   ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n\n");
    
    printf("Starting simulated monitoring...\n");
    printf("Press Ctrl+C to stop\n\n");
    
    sleep(1);
    
    // Print CSV header
    printf("Time(s),AvgTemp(C),MaxTemp(C),MinTemp(C),BattMin(V),BattMax(V),LowCells,Alert\n");
    
    // Run simulation
    for (int i = 0; i < 20; i++)  // 20 iterations for demo
    {
        // Task 1: Temperature monitoring
        simulate_temperature_readings(&temp_data);
        analyze_temperatures(&temp_data);
        
        // Task 3: Battery monitoring
        simulate_battery_readings(&battery_data);
        analyze_battery(&battery_data);
        
        // Task 2: Number analysis (demonstrate with random number)
        if (i % 5 == 0)  // Every 5 iterations
        {
            unsigned int test_number = 12345 + rand() % 10000;
            analyze_number(test_number, &num_analysis);
            
            printf("\n[Number Analysis] Input: %u, Sum: %u, Count: %d, Reversed: %u\n",
                   num_analysis.number,
                   num_analysis.digit_sum,
                   num_analysis.digit_count,
                   num_analysis.reversed);
        }
        
        // Print telemetry
        print_telemetry(&temp_data, &battery_data, i + 1);
        
        // Print cell details
        printf("CELLS: ");
        for (int j = 0; j < NUM_BATTERY_CELLS; j++)
        {
            printf("%.2fV ", battery_data.voltages[j]);
        }
        printf("\n");
        
        // Alert if battery low
        if (battery_data.critical_alert)
        {
            printf("⚠️  ALERT: %d cell(s) below %.1fV threshold!\n", 
                   battery_data.low_cell_count, BATTERY_LOW_THRESHOLD);
        }
        
        printf("\n");
        
        sleep(1);  // 1 second delay between readings
    }
    
    printf("\nSimulation complete!\n");
    printf("\nThis demonstrates the same logic that runs on the STM32 hardware.\n");
    printf("On real hardware, sensor readings come from ADC instead of simulation.\n");
    
    return 0;
}

void simulate_temperature_readings(TempSensorData* data)
{
    // Simulate temperature readings between 20-30°C with some variation
    static float base_temp = 25.0f;
    
    // Gradual drift
    base_temp += ((float)rand() / RAND_MAX - 0.5f) * 0.5f;
    
    // Keep in reasonable range
    if (base_temp < 20.0f) base_temp = 20.0f;
    if (base_temp > 30.0f) base_temp = 30.0f;
    
    for (int i = 0; i < NUM_TEMP_SENSORS; i++)
    {
        // Add individual sensor variation
        float variation = ((float)rand() / RAND_MAX - 0.5f) * 4.0f;
        data->temperatures[i] = base_temp + variation;
    }
}

void analyze_temperatures(TempSensorData* data)
{
    float sum = 0;
    data->max_temp = data->temperatures[0];
    data->min_temp = data->temperatures[0];
    
    for (int i = 0; i < NUM_TEMP_SENSORS; i++)
    {
        sum += data->temperatures[i];
        
        if (data->temperatures[i] > data->max_temp)
            data->max_temp = data->temperatures[i];
            
        if (data->temperatures[i] < data->min_temp)
            data->min_temp = data->temperatures[i];
    }
    
    data->avg_temp = sum / NUM_TEMP_SENSORS;
}

void simulate_battery_readings(BatteryData* data)
{
    // Simulate battery cells between 3.2V - 3.7V
    static float base_voltage = 3.5f;
    
    // Gradual discharge
    base_voltage -= 0.002f;
    
    // Recharge occasionally
    if (base_voltage < 3.3f && rand() % 5 == 0)
    {
        base_voltage = 3.6f;
    }
    
    data->low_cell_count = 0;
    data->critical_alert = 0;
    
    for (int i = 0; i < NUM_BATTERY_CELLS; i++)
    {
        // Add individual cell variation
        float variation = ((float)rand() / RAND_MAX - 0.5f) * 0.2f;
        data->voltages[i] = base_voltage + variation;
        
        // Occasionally make one cell go low
        if (i == 0 && rand() % 10 == 0)
        {
            data->voltages[i] = 3.1f + ((float)rand() / RAND_MAX) * 0.2f;
        }
        
        // Check threshold
        if (data->voltages[i] < BATTERY_LOW_THRESHOLD)
        {
            data->low_cell_count++;
            data->critical_alert = 1;
        }
    }
}

void analyze_battery(BatteryData* data)
{
    data->max_voltage = data->voltages[0];
    data->min_voltage = data->voltages[0];
    
    for (int i = 1; i < NUM_BATTERY_CELLS; i++)
    {
        if (data->voltages[i] > data->max_voltage)
            data->max_voltage = data->voltages[i];
            
        if (data->voltages[i] < data->min_voltage)
            data->min_voltage = data->voltages[i];
    }
}

void analyze_number(unsigned int number, NumberAnalysis* result)
{
    result->number = number;
    result->digit_sum = 0;
    result->digit_count = 0;
    result->reversed = 0;
    
    if (number == 0)
    {
        result->digit_count = 1;
        return;
    }
    
    unsigned int temp = number;
    
    while (temp > 0)
    {
        int digit = temp % 10;
        result->digit_sum += digit;
        result->digit_count++;
        result->reversed = result->reversed * 10 + digit;
        temp /= 10;
    }
}

void print_telemetry(TempSensorData* temp, BatteryData* batt, int iteration)
{
    printf("%d,%.2f,%.2f,%.2f,%.2f,%.2f,%d,%d\n",
           iteration,
           temp->avg_temp,
           temp->max_temp,
           temp->min_temp,
           batt->min_voltage,
           batt->max_voltage,
           batt->low_cell_count,
           batt->critical_alert);
}
