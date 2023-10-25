// multimod_i2c.c
// Date Created: 2023-07-25
// Date Updated: 2023-07-27
// Defines for I2C functions

/************************************Includes***************************************/

#include "../multimod_i2c.h"

#include <driverlib/gpio.h>
#include <driverlib/sysctl.h>
#include <driverlib/pin_map.h>

#include <inc/tm4c123gh6pm.h>
#include <inc/hw_i2c.h>

/************************************Includes***************************************/

/********************************Public Functions***********************************/

// I2C_Init
// Initializes specified I2C module
// Param uint32_t "mod": base address of module
// Return: void
void I2C_Init(uint32_t mod) {
    // Note:    The multimod board uses multiple I2C modules
//              to communicate with different devices. You can use
    //          the 'mod' parameter to choose which module to initialize
    //          and use.

    /*  // Here we can implement a switch later using mod as the I2C base addr
     // but for now we just use SCL1 and SDA1
    switch(mod)
    {
    //case IC20_BASE:
    }
    */

    SysCtlPeripheralEnable(SYSCTL_PERIPH_I2C1);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);

    // Enable clock to relevant I2C and GPIO modules
    //SYSCTL_RCGCI2C_R |= 0x00000002; // we enable I2C module 1 clock
    //SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R0; //enable clock port A


    // Configure pins for I2C module
    //GPIO_PORTA_AFSEL_R |=

    GPIOPinConfigure(GPIO_PA6_I2C1SCL);
    GPIOPinConfigure(GPIO_PA7_I2C1SDA);
    GPIOPinTypeI2CSCL(GPIO_PORTA_BASE, GPIO_PIN_6);
    GPIOPinTypeI2C(GPIO_PORTA_BASE, GPIO_PIN_7);

    // Configure I2C SCL speed, set as master
    I2CMasterInitExpClk(mod, SysCtlClockGet(), true);
    I2CMasterEnable(mod);

}

// I2C_WriteSingle
// Writes a single byte to an address.
// Param uint32_t "mod": base address of module
// Param uint8_t "addr": address to device
// Param uint8_t "byte": byte to send
// Return: void
void I2C_WriteSingle(uint32_t mod, uint8_t addr, uint8_t byte) {
    // Set the address in the slave address register
    I2CMasterSlaveAddrSet(mod, addr, false);
    // Input data into I2C module
    I2CMasterDataPut(mod, byte);
    // Trigger I2C module send
    I2CMasterControl(mod, I2C_MASTER_CMD_SINGLE_SEND);
    // Wait until I2C module is no longer busy
    while ( I2CMasterBusy(mod) ) {}

    return;
}

// I2C_ReadSingle
// Reads a single byte from address.
// Param uint32_t "mod": base address of module
// Param uint8_t "addr": address to device
// Return: uint8_t
uint8_t I2C_ReadSingle(uint32_t mod, uint8_t addr) {
    uint8_t data;

    // Set the address in the slave address register
    I2CMasterSlaveAddrSet(mod, addr, true); // true is read

    // Trigger I2C module receive
    I2CMasterControl(mod, I2C_MASTER_CMD_SINGLE_RECEIVE);

    // Wait until I2C module is no longer busy
    while (I2CMasterBusy(mod)) {}

    // Return received data
    data = I2CMasterDataGet(mod);
    return data;
}

// I2C_WriteMultiple
// Write multiple bytes to a device.
// Param uint32_t "mod": base address of module
// Param uint8_t "addr": address to device
// Param uint8_t* "data": pointer to an array of bytes
// Param uint8_t "num_bytes": number of bytes to transmit
// Return: void
void I2C_WriteMultiple(uint32_t mod, uint8_t addr, uint8_t* data, uint8_t num_bytes) {
    // Set the address in the slave address register
    I2CMasterSlaveAddrSet(mod, addr, false);
    // Input data into I2C module
    for (uint8_t i=0; i<num_bytes; i++)
    {
        I2CMasterDataPut(mod, data[i]);

        if (i==0)
            I2CMasterControl(mod, I2C_MASTER_CMD_BURST_SEND_START);
        else if (i == num_bytes-1)
            I2CMasterControl(mod, I2C_MASTER_CMD_BURST_SEND_FINISH);
        else
            I2CMasterControl(mod, I2C_MASTER_CMD_BURST_SEND_CONT);

        // wait until I2C module no longer busy
        while (I2CMasterBusy(mod)) {}

    }

    // Trigger I2C module send

    // Wait until I2C module is no longer busy

    // While num_bytes > 1
        // Input data into I2C module
        // Trigger I2C module send
        // Wait until I2C module is no longer busy

    // Input last byte into I2C module

    // Trigger I2C module send

    // Wait until I2C module is no longer busy


    return;
}

// I2C_ReadMultiple
// Read multiple bytes from a device.
// Param uint32_t "mod": base address of module
// Param uint8_t "addr": address to device
// Param uint8_t* "data": pointer to an array of bytes
// Param uint8_t "num_bytes": number of bytes to read
// Return: void
void I2C_ReadMultiple(uint32_t mod, uint8_t addr, uint8_t* data, uint8_t num_bytes) {
    // Set the address in the slave address register
    I2CMasterSlaveAddrSet(mod, addr, true);
    // Trigger I2C module receive
    I2CMasterControl(mod, I2C_MASTER_CMD_BURST_RECEIVE_START);
    // Wait until I2C module is no longer busy
    while (I2CMasterBusy(mod)) {}
    // Read received data
    data[0] = I2CMasterDataGet(mod);

    // While num_bytes > 1
    for (uint8_t i=1; i<num_bytes; i++)
    {
        // Trigger I2C module receive
        I2CMasterControl(mod, I2C_MASTER_CMD_BURST_RECEIVE_CONT);
        // Wait until I2C module is no longer busy
        while(I2CMasterBusy(mod)) {}
        // Read received data
        data[i] = I2CMasterDataGet(mod);
    }

    // Trigger I2C module receive
    I2CMasterControl(mod, I2C_MASTER_CMD_BURST_RECEIVE_FINISH);
    // Wait until I2C module is no longer busy
    while(I2CMasterBusy(mod)) {}
    // Read last byte
    //data[num_bytes-1] = I2CMasterDataGet(mod);


    return;
}

/********************************Public Functions***********************************/

