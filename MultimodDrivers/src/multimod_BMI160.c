// multimod_BMI160.c
// Date Created: 2023-07-25
// Date Updated: 2023-07-27
// Defines for BMI160 functions

/************************************Includes***************************************/

#include "../multimod_BMI160.h"

#include <stdint.h>
#include "../multimod_i2c.h"


#include <driverlib/uartstdio.h>
#include <driverlib/gpio.h>
#include <driverlib/uart.h>
#include <driverlib/sysctl.h>
#include <driverlib/pin_map.h>
#include <driverlib/interrupt.h>
#include <driverlib/timer.h>

/************************************Includes***************************************/

/********************************Public Functions***********************************/

// BMI160_Init
// Initializes the BMI160. Currently enables the accelerometer
// in full-power mode.
// Return: void
void BMI160_Init() {
    I2C_Init(I2C_A_BASE);
    // Power on accelerometer
    uint8_t ConfigValue = 0x11; // Example configuration value
    BMI160_WriteRegister(BMI160_CMD_ADDR, ConfigValue);
    BMI160_WriteRegister(BMI160_CMD_ADDR, 0x15);
    return;
}

// BMI160_WriteRegister
// Writes to a register address in the BMI160.
// Param uint8_t "addr": Register address
// Param uint8_t "data": data to write
// Return: void
void BMI160_WriteRegister(uint8_t addr, uint8_t data) {
    // Complete this function

    //set I2C addr for BMI160
    uint8_t bmi160_addr = 0x69;

    uint8_t regData[2] = { addr, data };

    // write register addr and data to BMI160
    I2C_WriteMultiple(I2C1_BASE, bmi160_addr, regData, 2);
}

// BMI160_ReadRegister
// Reads from a register address in the BMI160.
// Param uint8_t "addr": Register address
// Return: void
uint8_t BMI160_ReadRegister(uint8_t addr) {
    // Complete this function
    uint8_t data;

    //set I2C addr for BMI160
    uint8_t bmi160_addr = 0x69;

    // Write register addr to BMI160 (1 byte)
    I2C_WriteSingle(I2C1_BASE, bmi160_addr, addr);

    // read data from BMI160 register (1 byte)
    data = I2C_ReadSingle(I2C1_BASE, bmi160_addr);

    return data;
}

// BMI160_MultiReadRegister
// Uses the BMI160 auto-increment function to read from multiple registers.
// Param uint8_t "addr": beginning register address
// Param uint8_t* "data": pointer to an array to store data in
// Param uint8_t "num_bytes": number of bytes to read
// Return: void
void BMI160_MultiReadRegister(uint8_t addr, uint8_t* data, uint8_t num_bytes) {
    // Complete this function
    //set I2C addr for BMI160
    uint8_t bmi160_addr = 0x69;

    // Write register addr to BMI160 (1 byte)
    I2C_WriteSingle(I2C1_BASE, bmi160_addr, addr);

    // read data from BMI160 register (1 byte)
    I2C_ReadMultiple(I2C1_BASE, bmi160_addr, data, num_bytes);
}

// BMI160_AccelXGetResult
// Gets the 16-bit x-axis acceleration result.
// Return: uint16_t
int16_t BMI160_AccelXGetResult() {
    while (!(BMI160_GetDataStatus() & BMI160_STATUS_DRDY_ACC));
    uint8_t bytes[2];

    BMI160_MultiReadRegister(BMI160_DATA_O + ACCELX_O, bytes, 2);

    return (bytes[1] << 8 | bytes[0]);
}

// BMI160_AccelYGetResult
// Gets the 16-bit y-axis acceleration result.
// Return: uint16_t
int16_t BMI160_AccelYGetResult() {
    while (!(BMI160_GetDataStatus() & BMI160_STATUS_DRDY_ACC));

    uint8_t bytes[2];

    BMI160_MultiReadRegister(BMI160_DATA_O + ACCELY_O, bytes, 2);

    return (bytes[1] << 8 | bytes[0]);
}

// BMI160_AccelZGetResult
// Gets the 16-bit z-axis acceleration result.
// Return: uint16_t
int16_t BMI160_AccelZGetResult() {
    while (!(BMI160_GetDataStatus() & BMI160_STATUS_DRDY_GYR));

    uint8_t bytes[2];

    BMI160_MultiReadRegister(BMI160_DATA_O + ACCELZ_O, bytes, 2);

    return (bytes[1] << 8 | bytes[0]);
}

// BMI160_GyroXGetResult
// Gets the 16-bit x-axis gyroscope result.
// Return: uint16_t
int16_t BMI160_GyroXGetResult() {
    while (!(BMI160_GetDataStatus() & BMI160_STATUS_DRDY_GYR));

    uint8_t bytes[2];

    BMI160_MultiReadRegister(BMI160_DATA_O + GYROX_O, bytes, 2);

    return (bytes[1] << 8 | bytes[0]);
}

// BMI160_GyroYGetResult
// Gets the 16-bit y-axis gyroscope result.
// Return: uint16_t
int16_t BMI160_GyroYGetResult() {
    while (!(BMI160_GetDataStatus() & BMI160_STATUS_DRDY_GYR));

    uint8_t bytes[2];

    BMI160_MultiReadRegister(BMI160_DATA_O + GYROY_O, bytes, 2);

    return (bytes[1] << 8 | bytes[0]);
}

// BMI160_GyroZGetResult
// Gets the 16-bit z-axis gyroscope result.
// Return: uint16_t
int16_t BMI160_GyroZGetResult() {
    while (!(BMI160_GetDataStatus() & BMI160_STATUS_DRDY_GYR));

    uint8_t bytes[2];

    BMI160_MultiReadRegister(BMI160_DATA_O + GYROZ_O, bytes, 2);

    return (bytes[1] << 8 | bytes[0]);
}

// BMI160_MagXGetResult
// Gets the 16-bit x-axis magnetometer result.
// Return: uint16_t
int16_t BMI160_MagXGetResult() {
    while (!(BMI160_GetDataStatus() & BMI160_STATUS_DRDY_MAG));

    uint8_t bytes[2];

    BMI160_MultiReadRegister(BMI160_DATA_O + MAGX_O, bytes, 2);

    return (bytes[1] << 8 | bytes[0]);
}

// BMI160_MagYGetResult
// Gets the 16-bit y-axis magnetometer result.
// Return: uint16_t
int16_t BMI160_MagYGetResult() {
    while (!(BMI160_GetDataStatus() & BMI160_STATUS_DRDY_MAG));

    uint8_t bytes[2];

    BMI160_MultiReadRegister(BMI160_DATA_O + MAGY_O, bytes, 2);

    return (bytes[1] << 8 | bytes[0]);
}

// BMI160_MagZGetResult
// Gets the 16-bit z-axis magnetometer result.
// Return: uint16_t
int16_t BMI160_MagZGetResult() {
    while (!(BMI160_GetDataStatus() & BMI160_STATUS_DRDY_MAG));

    uint8_t bytes[2];

    BMI160_MultiReadRegister(BMI160_DATA_O + MAGZ_O, bytes, 2);

    return (bytes[1] << 8 | bytes[0]);
}

// BMI160_AccelXYZGetResult
// Stores the 16-bit XYZ accelerometer results in an array.
// Param uint16_t* "data": pointer to an array of 16-bit data.
// Return: void
void BMI160_AccelXYZGetResult(uint16_t* data) {
    while (!(BMI160_GetDataStatus() & BMI160_STATUS_DRDY_ACC));

    uint8_t bytes[6];

    BMI160_MultiReadRegister(BMI160_DATA_O + ACCELX_O, bytes, 6);

    *(data++) = (bytes[1] << 8 | bytes[0]);
    *(data++) = (bytes[3] << 8 | bytes[2]);
    *(data++) = (bytes[5] << 8 | bytes[4]);

    return;
}

// BMI160_GyroXYZGetResult
// Stores the 16-bit XYZ gyroscope results in an array.
// Param uint16_t* "data": pointer to an array of 16-bit data.
// Return: void
void BMI160_GyroXYZGetResult(uint16_t* data) {
    while (!(BMI160_GetDataStatus() & BMI160_STATUS_DRDY_GYR));

    uint8_t bytes[6];

    BMI160_MultiReadRegister(BMI160_DATA_O + GYROX_O, bytes, 6);

    *(data++) = (bytes[1] << 8 | bytes[0]);
    *(data++) = (bytes[3] << 8 | bytes[2]);
    *(data++) = (bytes[5] << 8 | bytes[4]);

    return;
}

// BMI160_MagXYZGetResult
// Stores the 16-bit XYZ magnetometer results in an array.
// Param uint16_t* "data": pointer to an array of 16-bit data.
// Return: void
void BMI160_MagXYZGetResult(uint16_t* data) {
    while (!(BMI160_GetDataStatus() & BMI160_STATUS_DRDY_MAG));

    uint8_t bytes[6];

    BMI160_MultiReadRegister(BMI160_DATA_O + MAGX_O, bytes, 6);

    *(data++) = (bytes[1] << 8 | bytes[0]);
    *(data++) = (bytes[3] << 8 | bytes[2]);
    *(data++) = (bytes[5] << 8 | bytes[4]);

    return;
}

// BMI160_GetDataStatus
// Gets the status register to determine if data is ready to read.
// Return: uint8_t
uint8_t BMI160_GetDataStatus() {
    return BMI160_ReadRegister(BMI160_STATUS_ADDR);
}

/********************************Public Functions***********************************/
