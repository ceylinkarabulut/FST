/* ============================================================================
 * FST Telemetry System Demo - Main Application
 * ============================================================================
 * Author: Ceylin Karabulut
 * Target: STM32 Nucleo Board(s) with ADC, UART, and Timer capabilities
 * Purpose: Professional implementation of sensor monitoring tasks with
 *          real hardware integration for Formula Student Team
 * 
 * Features:
 * - Task 1: Temperature sensor array monitoring with ADC
 * - Task 2: Digital number analyzer (utility function)
 * - Task 3: Battery cell voltage monitoring with alerts
 * - UART telemetry output in CSV format
 * - LED visual alerts for critical battery levels
 * ============================================================================
 */

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "stm32f4xx_hal.h"

/* ============================================================================
 * CONFIGURATION CONSTANTS
 * ============================================================================ */
#define NUM_TEMP_SENSORS 10
#define NUM_BATTERY_CELLS 8
#define BATTERY_LOW_THRESHOLD 3.3f  // Volts
#define ADC_RESOLUTION 4096.0f      // 12-bit ADC
#define VREF 3.3f                   // Reference voltage

#define SAMPLING_RATE_MS 1000       // Sample every 1 second

/* ============================================================================
 * PERIPHERAL HANDLES (Initialize in CubeMX)
 * ============================================================================ */
ADC_HandleTypeDef hadc1;
UART_HandleTypeDef huart2;
TIM_HandleTypeDef htim2;

/* ============================================================================
 * DATA STRUCTURES
 * ============================================================================ */
typedef struct {
    float temperatures[NUM_TEMP_SENSORS];
    float avg_temp;
    float max_temp;
    float min_temp;
    uint8_t valid_readings;
} TempSensorData;

typedef struct {
    float voltages[NUM_BATTERY_CELLS];
    float max_voltage;
    float min_voltage;
    uint8_t low_cell_count;
    uint8_t critical_alert;
} BatteryData;

typedef struct {
    uint32_t number;
    uint32_t digit_sum;
    uint8_t digit_count;
    uint32_t reversed;
} NumberAnalysis;

/* ============================================================================
 * GLOBAL VARIABLES
 * ============================================================================ */
TempSensorData temp_data = {0};
BatteryData battery_data = {0};
char uart_buffer[256];

/* ============================================================================
 * FUNCTION PROTOTYPES
 * ============================================================================ */
void SystemClock_Config(void);
void GPIO_Init(void);
void ADC_Init(void);
void UART_Init(void);
void TIM_Init(void);

// Task implementations
void Task1_ReadTemperatures(void);
void Task1_AnalyzeTemperatures(void);
void Task2_AnalyzeNumber(uint32_t number, NumberAnalysis* result);
void Task3_ReadBatteryVoltages(void);
void Task3_AnalyzeBattery(void);

// Utility functions
float ADC_ToVoltage(uint16_t adc_value);
float Voltage_ToTemperature(float voltage);
void UART_SendTelemetry(void);
void LED_UpdateStatus(void);

/* ============================================================================
 * MAIN FUNCTION
 * ============================================================================ */
int main(void)
{
    /* HAL Initialization */
    HAL_Init();
    SystemClock_Config();
    
    /* Initialize peripherals */
    GPIO_Init();
    ADC_Init();
    UART_Init();
    TIM_Init();
    
    /* Startup message */
    sprintf(uart_buffer, "\r\n=== FST Telemetry System v1.0 ===\r\n");
    HAL_UART_Transmit(&huart2, (uint8_t*)uart_buffer, strlen(uart_buffer), 100);
    sprintf(uart_buffer, "Initializing sensors...\r\n\r\n");
    HAL_UART_Transmit(&huart2, (uint8_t*)uart_buffer, strlen(uart_buffer), 100);
    
    /* Start timer for periodic sampling */
    HAL_TIM_Base_Start_IT(&htim2);
    
    /* Main loop */
    while (1)
    {
        /* Tasks are triggered by timer interrupt */
        /* Main loop handles non-critical processing */
        HAL_Delay(10);
    }
}

/* ============================================================================
 * TASK 1: TEMPERATURE SENSOR MONITORING
 * ============================================================================ */
void Task1_ReadTemperatures(void)
{
    /* In real FST application, this would read from multiple thermocouples
     * For demo, we'll read ADC channels and convert to temperature
     * Simulating LM35 temperature sensor (10mV/°C)
     */
    
    for (int i = 0; i < NUM_TEMP_SENSORS; i++)
    {
        // Select ADC channel (in real hw, you'd cycle through channels)
        // For demo with limited channels, we'll simulate variation
        
        HAL_ADC_Start(&hadc1);
        HAL_ADC_PollForConversion(&hadc1, 100);
        uint16_t adc_value = HAL_ADC_GetValue(&hadc1);
        HAL_ADC_Stop(&hadc1);
        
        // Convert ADC -> Voltage -> Temperature
        float voltage = ADC_ToVoltage(adc_value);
        temp_data.temperatures[i] = Voltage_ToTemperature(voltage);
        
        // Small delay between readings
        HAL_Delay(10);
    }
    
    temp_data.valid_readings = NUM_TEMP_SENSORS;
}

void Task1_AnalyzeTemperatures(void)
{
    if (temp_data.valid_readings == 0) return;
    
    float sum = 0;
    temp_data.max_temp = temp_data.temperatures[0];
    temp_data.min_temp = temp_data.temperatures[0];
    
    for (int i = 0; i < temp_data.valid_readings; i++)
    {
        sum += temp_data.temperatures[i];
        
        if (temp_data.temperatures[i] > temp_data.max_temp)
            temp_data.max_temp = temp_data.temperatures[i];
            
        if (temp_data.temperatures[i] < temp_data.min_temp)
            temp_data.min_temp = temp_data.temperatures[i];
    }
    
    temp_data.avg_temp = sum / temp_data.valid_readings;
}

/* ============================================================================
 * TASK 2: NUMBER ANALYZER (UTILITY FUNCTION)
 * ============================================================================ */
void Task2_AnalyzeNumber(uint32_t number, NumberAnalysis* result)
{
    result->number = number;
    result->digit_sum = 0;
    result->digit_count = 0;
    result->reversed = 0;
    
    uint32_t temp = number;
    
    // Handle special case of 0
    if (number == 0)
    {
        result->digit_count = 1;
        result->digit_sum = 0;
        result->reversed = 0;
        return;
    }
    
    // Calculate sum, count, and reverse
    while (temp > 0)
    {
        uint8_t digit = temp % 10;
        result->digit_sum += digit;
        result->digit_count++;
        result->reversed = result->reversed * 10 + digit;
        temp /= 10;
    }
}

/* ============================================================================
 * TASK 3: BATTERY CELL MONITORING
 * ============================================================================ */
void Task3_ReadBatteryVoltages(void)
{
    /* In real FST car, this would read from battery management system
     * via CAN bus or direct ADC from voltage dividers
     * For demo, reading from ADC channels
     */
    
    battery_data.low_cell_count = 0;
    battery_data.critical_alert = 0;
    
    for (int i = 0; i < NUM_BATTERY_CELLS; i++)
    {
        HAL_ADC_Start(&hadc1);
        HAL_ADC_PollForConversion(&hadc1, 100);
        uint16_t adc_value = HAL_ADC_GetValue(&hadc1);
        HAL_ADC_Stop(&hadc1);
        
        // Convert to voltage (with voltage divider compensation if needed)
        // Assuming direct measurement for demo (0-3.3V range)
        battery_data.voltages[i] = ADC_ToVoltage(adc_value);
        
        // Check for low voltage cells
        if (battery_data.voltages[i] < BATTERY_LOW_THRESHOLD)
        {
            battery_data.low_cell_count++;
            battery_data.critical_alert = 1;
        }
        
        HAL_Delay(5);
    }
}

void Task3_AnalyzeBattery(void)
{
    battery_data.max_voltage = battery_data.voltages[0];
    battery_data.min_voltage = battery_data.voltages[0];
    
    for (int i = 1; i < NUM_BATTERY_CELLS; i++)
    {
        if (battery_data.voltages[i] > battery_data.max_voltage)
            battery_data.max_voltage = battery_data.voltages[i];
            
        if (battery_data.voltages[i] < battery_data.min_voltage)
            battery_data.min_voltage = battery_data.voltages[i];
    }
}

/* ============================================================================
 * UTILITY FUNCTIONS
 * ============================================================================ */
float ADC_ToVoltage(uint16_t adc_value)
{
    return ((float)adc_value / ADC_RESOLUTION) * VREF;
}

float Voltage_ToTemperature(float voltage)
{
    /* LM35 conversion: 10mV per degree Celsius
     * Temperature (°C) = Voltage (mV) / 10
     * For demo purposes - adjust based on actual sensor
     */
    return (voltage * 1000.0f) / 10.0f;
}

void UART_SendTelemetry(void)
{
    /* Send data in CSV format for easy logging and plotting */
    
    uint32_t timestamp = HAL_GetTick();
    
    // Header (send once or periodically)
    static uint8_t header_sent = 0;
    if (!header_sent)
    {
        sprintf(uart_buffer, "Time(ms),AvgTemp(C),MaxTemp(C),MinTemp(C),BattMin(V),BattMax(V),LowCells,Alert\r\n");
        HAL_UART_Transmit(&huart2, (uint8_t*)uart_buffer, strlen(uart_buffer), 200);
        header_sent = 1;
    }
    
    // Data row
    sprintf(uart_buffer, "%lu,%.2f,%.2f,%.2f,%.2f,%.2f,%d,%d\r\n",
            timestamp,
            temp_data.avg_temp,
            temp_data.max_temp,
            temp_data.min_temp,
            battery_data.min_voltage,
            battery_data.max_voltage,
            battery_data.low_cell_count,
            battery_data.critical_alert);
    
    HAL_UART_Transmit(&huart2, (uint8_t*)uart_buffer, strlen(uart_buffer), 200);
    
    // Also send detailed battery cell data
    sprintf(uart_buffer, "CELLS: ");
    HAL_UART_Transmit(&huart2, (uint8_t*)uart_buffer, strlen(uart_buffer), 100);
    
    for (int i = 0; i < NUM_BATTERY_CELLS; i++)
    {
        sprintf(uart_buffer, "%.2fV ", battery_data.voltages[i]);
        HAL_UART_Transmit(&huart2, (uint8_t*)uart_buffer, strlen(uart_buffer), 100);
    }
    sprintf(uart_buffer, "\r\n\r\n");
    HAL_UART_Transmit(&huart2, (uint8_t*)uart_buffer, strlen(uart_buffer), 100);
}

void LED_UpdateStatus(void)
{
    /* Use onboard LED to show battery status
     * Solid ON = All cells healthy
     * Blinking = Low voltage detected
     */
    
    if (battery_data.critical_alert)
    {
        // Blink LED
        HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5); // Adjust pin for your Nucleo
    }
    else
    {
        // Solid ON
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
    }
}

/* ============================================================================
 * TIMER INTERRUPT - PERIODIC SAMPLING
 * ============================================================================ */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM2)
    {
        // Read all sensors
        Task1_ReadTemperatures();
        Task1_AnalyzeTemperatures();
        
        Task3_ReadBatteryVoltages();
        Task3_AnalyzeBattery();
        
        // Send telemetry
        UART_SendTelemetry();
        
        // Update visual status
        LED_UpdateStatus();
    }
}

/* ============================================================================
 * PERIPHERAL INITIALIZATION (Customize for your Nucleo board)
 * ============================================================================ */
void GPIO_Init(void)
{
    __HAL_RCC_GPIOA_CLK_ENABLE();
    
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    // LED Pin (PA5 for most Nucleo boards)
    GPIO_InitStruct.Pin = GPIO_PIN_5;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}

void ADC_Init(void)
{
    /* Configure ADC for temperature and battery monitoring
     * Customize based on your specific Nucleo model
     * This is a template - adjust for your hardware
     */
    
    __HAL_RCC_ADC1_CLK_ENABLE();
    
    hadc1.Instance = ADC1;
    hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
    hadc1.Init.Resolution = ADC_RESOLUTION_12B;
    hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
    hadc1.Init.ScanConvMode = DISABLE;
    hadc1.Init.ContinuousConvMode = DISABLE;
    hadc1.Init.NbrOfConversion = 1;
    hadc1.Init.DiscontinuousConvMode = DISABLE;
    hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
    
    if (HAL_ADC_Init(&hadc1) != HAL_OK)
    {
        Error_Handler();
    }
}

void UART_Init(void)
{
    /* UART for telemetry output
     * Typically USART2 on Nucleo boards (ST-Link virtual COM port)
     */
    
    __HAL_RCC_USART2_CLK_ENABLE();
    
    huart2.Instance = USART2;
    huart2.Init.BaudRate = 115200;
    huart2.Init.WordLength = UART_WORDLENGTH_8B;
    huart2.Init.StopBits = UART_STOPBITS_1;
    huart2.Init.Parity = UART_PARITY_NONE;
    huart2.Init.Mode = UART_MODE_TX_RX;
    huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart2.Init.OverSampling = UART_OVERSAMPLING_16;
    
    if (HAL_UART_Init(&huart2) != HAL_OK)
    {
        Error_Handler();
    }
}

void TIM_Init(void)
{
    /* Timer for periodic sampling */
    __HAL_RCC_TIM2_CLK_ENABLE();
    
    htim2.Instance = TIM2;
    htim2.Init.Prescaler = 8400 - 1;  // Adjust for your system clock
    htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim2.Init.Period = SAMPLING_RATE_MS * 10 - 1;  // 1 second
    htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    
    if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
    {
        Error_Handler();
    }
    
    HAL_NVIC_SetPriority(TIM2_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(TIM2_IRQn);
}

void SystemClock_Config(void)
{
    /* Configure for your specific Nucleo board
     * This is a placeholder - use CubeMX generated code
     */
}

void Error_Handler(void)
{
    __disable_irq();
    while (1)
    {
        // Blink LED rapidly to indicate error
        HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
        HAL_Delay(100);
    }
}
