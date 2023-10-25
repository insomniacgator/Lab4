// multimod_OPT3001.c
// Date Created: 2023-07-25
// Date Updated: 2023-07-27
// Defines for OPT3001 functions

/************************************Includes***************************************/

#include "../multimod_LaunchpadButtons.h"

#include <stdint.h>
#include <stdbool.h>

#include <inc/tm4c123gh6pm.h>
#include <inc/hw_types.h>
#include <inc/hw_memmap.h>
#include <inc/hw_i2c.h>
#include <inc/hw_gpio.h>

#include <driverlib/gpio.h>
#include <driverlib/sysctl.h>
#include <driverlib/uart.h>

/************************************Includes***************************************/

/********************************Public Functions***********************************/

// LaunchpadButtons_Init
// Initializes the GPIO port & pins necessary for the button switches on the
// launchpad.
// Return: void
void LaunchpadButtons_Init() {
    GPIO_PORTF_LOCK_R = GPIO_LOCK_KEY;
    GPIO_PORTF_CR_R |= GPIO_PIN_0;

    // Enable port F for switches
    GPIO_PORTF_DEN_R |= 0x00000001; // digital enable PF0
    GPIO_PORTF_DEN_R |= 0x00000010; // digital enable PF4

    // set up pull up resistors (do I even need this?)
    GPIO_PORTF_PUR_R |= 0x00000011;

    // Use SW1 & SW2, configure as inputs.
    GPIO_PORTF_DIR_R |= ~0x00000011; // set pins 0 and 4 as inputs //I think this is wrong


    // Pay attention to the pin connected to SW2!!! PF0 is a locked pin.

}

// LaunchpadButtons_ReadSW1
// Checks to see if button 1 is pressed. If it is, return 1; else return 0.
// Return: uint8_t
uint8_t LaunchpadButtons_ReadSW1() {
    return ( GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_4) && 0x00000010 );
    //return ( *((volatile uint32_t *)(GPIO_PORTF_BASE + (0x00000010 << 2))) & 0x00000010 );
}

// LaunchpadButtons_ReadSW2
// Checks to see if button 1 is pressed. If it is, return 1; else return 0.
// Return: uint8_t
uint8_t LaunchpadButtons_ReadSW2() {
    return ( GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_0) && 0x00000001 );
    //return ( *((volatile uint32_t *)(GPIO_PORTF_BASE + (0x00000010 << 2))) & 0x00000010 );
}



/********************************Public Functions***********************************/
