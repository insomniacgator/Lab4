// multimod_OPT3001.c
// Date Created: 2023-07-25
// Date Updated: 2023-07-27
// Defines for OPT3001 functions

/************************************Includes***************************************/

#include "../multimod_OPT3001.h"

#include <stdint.h>
#include "../multimod_i2c.h"

/************************************Includes***************************************/

/********************************Public Functions***********************************/

// OPT3001_Init
// Initializes OPT3001, configures it to continuous conversion mode.
// Return: void
void OPT3001_Init(void) {
    // Initialize I2C module
    I2C_Init(I2C_A_BASE);

    // Set the correct configuration byte for continuous conversions
    uint16_t configValue = (0xC610);
    OPT3001_WriteRegister(OPT3001_CONFIG_ADDR, configValue);
    return;
}

// OPT3001_WriteRegister
// Writes to a register in the OPT3001.
// Param uint8_t "addr": Register address of the OPT3001.
// Param uint16_t "data": 16-bit data to write to the register.
// Return: void
void OPT3001_WriteRegister(uint8_t addr, uint16_t data) {
    // Read the datasheet!
    // Complete this function
    // I2C OPT3001 address
    uint8_t opt3001_addr = 0x47;

    uint8_t regData[3] = { addr, (uint8_t)(data >> 8), (uint8_t)data };

    I2C_WriteMultiple(I2C1_BASE, opt3001_addr, regData, 3);

    return;

}

// OPT3001_ReadRegister
// Reads from a register in the OPT3001.
// Param uint8_t "addr": Register address of the OPT3001.
// Return: uint16_t
uint16_t OPT3001_ReadRegister(uint8_t addr) {
    // Complete this function
    uint16_t data;
    // I2C OPT3001 address
    uint8_t opt3001_addr = 0x47;

    // write register addr to read
    I2C_WriteSingle(I2C1_BASE, opt3001_addr, addr);

    // array to store read data
    uint8_t regData[2];
    // read register
    I2C_ReadMultiple(I2C1_BASE, opt3001_addr, regData, 2);
    // merge read data into 16 bit int
    data = (regData[0] << 8 | regData[1]);

    return data;

}

// OPT3001_GetResult
// Gets conversion result, calculates byte result based on datasheet
// and configuration settings.
// Return: uint32_t
uint32_t OPT3001_GetResult(void) {
    // Check if data is ready first

    while(!(OPT3001_CONFIG_CRF & OPT3001_ReadRegister(OPT3001_CONFIG_ADDR)));

    uint16_t result = OPT3001_ReadRegister(OPT3001_RESULT_ADDR);

    result = LUX((result >> 12 & 0xF), (result & 0x0FFF));

    return result;
}

// OPT3001_SetLowLimit
// Sets the low limit register.
// Param uint16_t "exp": Exponential bound
// Param uint16_t "result": Conversion bound
// Return: void
void OPT3001_SetLowLimit(uint16_t exp, uint16_t result) {
    OPT3001_WriteRegister(OPT3001_LOWLIMIT_ADDR, (exp << OPT3001_RESULT_E_S | (result & 0xFFF)));

    return;
}

// OPT3001_SetHighLimit
// Sets the high limit register.
// Param uint16_t "exp": Exponential bound
// Param uint16_t "result": Conversion bound
// Return: void
void OPT3001_SetHighLimit(uint16_t exp, uint16_t result) {
    OPT3001_WriteRegister(OPT3001_HIGHLIMIT_ADDR, (exp << OPT3001_RESULT_E_S | (result & 0xFFF)));

    return;
}

// OPT3001_GetChipID
// Gets the chip ID of the OPT3001.
// Return: uint16_t
uint16_t OPT3001_GetChipID(void) {
    return OPT3001_ReadRegister(OPT3001_DEVICEID_ADDR);
}

/********************************Public Functions***********************************/
