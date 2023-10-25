// multimod_OPT3001.c
// Date Created: 2023-07-25
// Date Updated: 2023-07-27
// Defines for OPT3001 functions

/************************************Includes***************************************/

#include "../multimod_LaunchpadLED.h"

#include <stdint.h>
#include <stdbool.h>

#include <inc/tm4c123gh6pm.h>
#include <inc/hw_types.h>
#include <inc/hw_memmap.h>
#include <inc/hw_gpio.h>

#include <driverlib/pin_map.h>
#include <driverlib/pwm.h>
#include <driverlib/gpio.h>
#include <driverlib/sysctl.h>

/************************************Includes***************************************/

/********************************Public Functions***********************************/

// LaunchpadButtons_Init
// Initializes the GPIO port & pins necessary for the button switches on the
// launchpad. Also configures it so that the LEDs are controlled via PWM signal.
// Initial default period of 400.
// Return: void
void LaunchpadLED_Init() {
    // Enable clock to port F
    SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R5; // does this enable clock only to GPIO and not other peripherals?

    GPIO_PORTF_DIR_R |= (1 << 1) | (1 << 2) | (1 << 3);
    GPIO_PORTF_DEN_R |= (1 << 1) | (1 << 2) | (1 << 3);

    // Enable PWM module
    SYSCTL_RCGCPWM_R |= SYSCTL_RCGCPWM_R1; // Clock Gating Control enable PWM1
    SYSCTL_RCGC0_R |= SYSCTL_RCGC0_PWM0; //0x00100000

    // Configure necessary pins as PWM
    SYSCTL_RCGC2_R |= SYSCTL_RCGC2_GPIOF; //0x00000020 // this also enables clock (to AHB?)
    GPIO_PORTF_AFSEL_R |= 0x0000000E; // select alt function on PF1, PF2, PF3
    // when do we use these AHB ports: GPIO_PORTF_AHB_AFSEL_R ?
    GPIO_PORTF_PCTL_R |= 0x00005550; // select alternate function PWM on PF1, PF2, PF3

    // Configure necessary PWM generators in count down mode, no sync
    //SYSCTL_RCC_R |= 0x00100000; // USEPWMDIV enabled and set to 19:17 to 000 (divide by 2)
    PWM1_3_GENA_R |= 0x000000C8; // PWM1 Generator 3 (signals: M1PWM6, M1PWM7)
    PWM1_3_GENB_R |= 0x00000C08; // these 2 lines sets the 2 pwms in countdown mode

    //PWM1_2_GENA_R |= 0x000000C8; // PWM1 Generator 3 (signals: M1PWM6, M1PWM7)
    PWM1_2_GENB_R |= 0x00000C08; // these 2 lines sets the 2 pwms in countdown mode

    // Set generator periods
    uint32_t sysClock = SysCtlClockGet();

    /*
    // lets set it at 1000Hz
    PWM1_3_LOAD_R |= (uint16_t)((sysClock/2)*(0.001)); // 0.001s = 1KHz. We divide sysClock by 2 because we set RCC that way
    PWM1_2_LOAD_R |= (uint16_t)((sysClock/2)*(0.001)); // 0.001s = 1KHz. We divide sysClock by 2 because we set RCC that way

    // Set the default pulse width (duty cycles).
    PWM1_3_CMPA_R |= (uint16_t)((sysClock/2)*(0.001) / 2);
    PWM1_3_CMPB_R |= (uint16_t)((sysClock/2)*(0.001) / 4);
    */
    PWM1_3_LOAD_R = 400; // 0.001s = 1KHz. We divide sysClock by 2 because we set RCC that way
    PWM1_2_LOAD_R = 400; // 0.001s = 1KHz. We divide sysClock by 2 because we set RCC that way

    // Set the default pulse width (duty cycles).
    PWM1_3_CMPA_R = 1;
    PWM1_3_CMPB_R = 1;



    //PWM1_2_CMPA_R |= (uint16_t)((sysClock/2)*(0.001) / 2);
    PWM1_2_CMPB_R = 1;


    // Enable the PWM generators
    PWM1_3_CTL_R |= PWM_1_CTL_ENABLE; //0x00000001
    PWM1_2_CTL_R |= PWM_1_CTL_ENABLE;
    // Enable PWM output
    PWM1_ENABLE_R |= PWM_ENABLE_PWM5EN | PWM_ENABLE_PWM6EN | PWM_ENABLE_PWM7EN;
}

// LaunchpadLED_PWMSetDuty
// Sets the duty cycle of the PWM generator associated with the LED.
// Return: void
void LaunchpadLED_PWMSetDuty(LED_Color_t LED, float duty) {
    uint32_t pulse_width = (duty * PWM_Per);

    // If pulse width < 1, set as 1
    if (pulse_width < 1) {
        pulse_width = 1;
    }

    // If pulse width > set period, cap it
    if (pulse_width > PWM_Per) {
        pulse_width = PWM_Per - 1;
    }

    // Depending on chosen LED(s), adjust corresponding duty cycle of the PWM output
    // Your code below.
    switch(LED)
    {
    case RED:
        PWM1_2_CMPB_R = pulse_width;
        break;
    case GREEN:
        PWM1_3_CMPB_R = pulse_width;
        break;
    case BLUE:
        PWM1_3_CMPA_R = pulse_width;
        break;
    default:
    }



    return;
}

/********************************Public Functions***********************************/
