// multimod_buttons.c
// Date Created: 2023-07-25
// Date Updated: 2023-07-27
// Defines for button functions

/************************************Includes***************************************/

#include "../multimod_buttons.h"

#include "../multimod_i2c.h"

#include <driverlib/gpio.h>
#include <driverlib/sysctl.h>
#include <driverlib/pin_map.h>

#include <inc/tm4c123gh6pm.h>
#include <inc/hw_i2c.h>

/********************************Public Functions***********************************/

// Buttons_Init
// Initializes buttons on the multimod by configuring the I2C module and
// relevant interrupt pin.
// Return: void
void MultimodButtons_Init() {
    // Initialize this function & the relevant interrupt pin
    //G8RTOS_WaitSemaphore(&sem_I2CA);

    I2C_Init(I2C_A_BASE);
    GPIOIntTypeSet(GPIO_PORTE_BASE, GPIO_PIN_4, GPIO_FALLING_EDGE);
    GPIOIntEnable(GPIO_PORTE_BASE, GPIO_INT_PIN_4);
    //G8RTOS_SignalSemaphore(&sem_I2CA);

/*

I2CBUS_A_SDA = PA7 (I2C1SDA)
I2CBUS_A_SCL = PA6 (I2C1SCL)

#define BUTTONS_PCA9555_GPIO_ADDR   0x23
#define BUTTONS_INT_GPIO_BASE       GPIO_PORTE_BASE
#define BUTTONS_INT_PIN             GPIO_PIN_4

#define SW1                         0x02
#define SW2                         0x04
#define SW3                         0x08
#define SW4                         0x10

#define PCA9555_BUTTONS_ADDR        0x21
 */
}

// MultimodButtons_Get
// Gets the input to GPIO bank 1, [0..7].
// Return: uint8_t 
uint8_t MultimodButtons_Get() {
    // complete this function
    uint8_t read_data = 0;


    I2C_WriteSingle(I2C1_BASE, BUTTONS_PCA9555_GPIO_ADDR, 0x00);
    read_data = I2C_ReadSingle(I2C1_BASE, BUTTONS_PCA9555_GPIO_ADDR);
    //I2C_ReadMultiple(I2C1_BASE, BUTTONS_PCA9555_GPIO_ADDR, read_data, 2);

    return read_data;
}

