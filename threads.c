// G8RTOS_Threads.c
// Date Created: 2023-07-25
// Date Updated: 2023-07-27
// Defines for thread functions.

/************************************Includes***************************************/

#include "./threads.h"

#include "./MultimodDrivers/multimod.h"
#include "./MiscFunctions/Shapes/inc/cube.h"
#include "./MiscFunctions/LinAlg/inc/linalg.h"
#include "./MiscFunctions/LinAlg/inc/quaternions.h"
#include "./MiscFunctions/LinAlg/inc/vect3d.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Change this to change the number of points that make up each line of a cube.
// Note that if you set this too high you will have a stack overflow!
#define Num_Interpolated_Points 10

// sizeof(float) * num_lines * (Num_Interpolated_Points + 2) = ?

#define MAX_NUM_CUBES           (MAX_THREADS - 3)


/*********************************Global Variables**********************************/
Quat_t world_camera_pos = {0, 0, 0, 50};
Quat_t world_camera_frame_offset = {0, 0, 0, 50};
Quat_t world_camera_frame_rot_offset;
Quat_t world_view_rot = {1, 0, 0, 0};
Quat_t world_view_rot_inverse = {1, 0, 0, 0};

// How many cubes?
uint8_t num_cubes = 0;

// y-axis controls z or y
uint8_t joystick_y = 1;

// Kill a cube?
uint8_t kill_cube = 0;

/*********************************Global Variables**********************************/

/*************************************Threads***************************************/

void Idle_Thread(void) {
    time_t t;
    srand((unsigned) time(&t));
    while(1);
}

void CamMove_Thread(void) {
    // Initialize / declare any variables here
    int32_t joy_x, joy_y = 0;
    int32_t joy_x_n, joy_y_n = 0;

    while(1) {
        // Get result from joystick
        joy_x = G8RTOS_ReadFIFO(JOYSTICK_FIFO);
        joy_y = G8RTOS_ReadFIFO(JOYSTICK_FIFO);
        
        // If joystick axis within deadzone, set to 0. Otherwise normalize it.
        if (joy_x > 100 || joy_x < -100)
        {
            joy_x_n = (joy_x * 1000) / 4095;
        }
        else
            joy_x_n = 0;
        if (joy_y > 100 || joy_y < -100)
                {
                    joy_y_n = (joy_y * 1000) / 4095;
                }
        else
            joy_y_n = 0;

        // Update world camera position. Update y/z coordinates depending on the joystick toggle.
        if (joystick_y)
        {
            world_camera_pos.y += joy_y_n;
            world_camera_pos.x += joy_x_n;
        }
        else
        {
            world_camera_pos.z += joy_y_n;
            world_camera_pos.x += joy_x_n;
        }

        // sleep
        sleep(200);
    }
}

void Cube_Thread(void) {
    cube_t cube;

    /*************YOUR CODE HERE*************/
    // Get spawn coordinates from FIFO, set cube.x, cube.y, cube.z


    cube.width = 50;
    cube.height = 50;
    cube.length = 50;

    Quat_t v[8];
    Quat_t v_relative[8];

    Cube_Generate(v, &cube);

    uint32_t m = Num_Interpolated_Points + 1;
    Vect3D_t interpolated_points[12][Num_Interpolated_Points + 2];
    Vect3D_t projected_point;

    Quat_t camera_pos;
    Quat_t camera_frame_offset;
    Quat_t view_rot_inverse;

    uint8_t kill = 0;

    while(1) {
        /*************YOUR CODE HERE*************/
        // Check if kill ball flag is set.

        camera_pos.x = world_camera_pos.x;
        camera_pos.y = world_camera_pos.y;
        camera_pos.z = world_camera_pos.z;

        camera_frame_offset.x = world_camera_frame_offset.x;
        camera_frame_offset.y = world_camera_frame_offset.y;
        camera_frame_offset.z = world_camera_frame_offset.z;

        view_rot_inverse.w = world_view_rot_inverse.w;
        view_rot_inverse.x = world_view_rot_inverse.x;
        view_rot_inverse.y = world_view_rot_inverse.y;
        view_rot_inverse.z = world_view_rot_inverse.z;

        // Clears cube from screen
        for (int i = 0; i < 12; i++) {
            for (int j = 0; j < m+1; j++) {
                getViewOnScreen(&projected_point, &camera_frame_offset, &(interpolated_points[i][j]));
                /*************YOUR CODE HERE*************/
                // Wait on SPI bus

                ST7789_DrawPixel(projected_point.x, projected_point.y, ST7789_BLACK);

                /*************YOUR CODE HERE*************/
                // Signal that SPI bus is available

            }
        }

        /*************YOUR CODE HERE*************/
        // If ball marked for termination, kill the thread.

        // Calculates view relative to camera position / orientation
        for (int i = 0; i < 8; i++) {
            getViewRelative(&(v_relative[i]), &camera_pos, &(v[i]), &view_rot_inverse);
        }

        // Interpolates points between vertices
        interpolatePoints(interpolated_points[0], &v_relative[0], &v_relative[1], m);
        interpolatePoints(interpolated_points[1], &v_relative[1], &v_relative[2], m);
        interpolatePoints(interpolated_points[2], &v_relative[2], &v_relative[3], m);
        interpolatePoints(interpolated_points[3], &v_relative[3], &v_relative[0], m);
        interpolatePoints(interpolated_points[4], &v_relative[0], &v_relative[4], m);
        interpolatePoints(interpolated_points[5], &v_relative[1], &v_relative[5], m);
        interpolatePoints(interpolated_points[6], &v_relative[2], &v_relative[6], m);
        interpolatePoints(interpolated_points[7], &v_relative[3], &v_relative[7], m);
        interpolatePoints(interpolated_points[8], &v_relative[4], &v_relative[5], m);
        interpolatePoints(interpolated_points[9], &v_relative[5], &v_relative[6], m);
        interpolatePoints(interpolated_points[10], &v_relative[6], &v_relative[7], m);
        interpolatePoints(interpolated_points[11], &v_relative[7], &v_relative[4], m);

        for (int i = 0; i < 12; i++) {
            for (int j = 0; j < m+1; j++) {
                getViewOnScreen(&projected_point, &camera_frame_offset, &(interpolated_points[i][j]));

                if (interpolated_points[i][j].z < 0) {
                    /*************YOUR CODE HERE*************/
                    // Wait on SPI bus

                    ST7789_DrawPixel(projected_point.x, projected_point.y, ST7789_BLUE);

                    /*************YOUR CODE HERE*************/
                    // Signal that SPI bus is available
                }
            }
        }

        /*************YOUR CODE HERE*************/
        // Sleep

    }
}

void Read_Buttons() {
    // Initialize / declare any variables here
    uint8_t buttons_read = 0;
    
    while(1) {
        // Wait for a signal to read the buttons on the Multimod board.
        G8RTOS_WaitSemaphore(&sem_PCA9555_Debounce);
        // Sleep to debounce
        sleep(100);

        // Read the buttons status on the Multimod board.
        buttons_read = MultimodButtons_Get();

        // Process the buttons and determine what actions need to be performed.
        UARTprintf("Buttons pressed: %d\n", buttons_read);

        // Clear the interrupt
        GPIOIntClear(GPIO_PORTE_BASE, GPIO_PIN_4);
        // Re-enable the interrupt so it can occur again.
        GPIOIntEnable(GPIO_PORTE_BASE, GPIO_PIN_4);
    
    }
}

void Read_JoystickPress() {
    // Initialize / declare any variables here
    
    while(1) {
        // Wait for a signal to read the joystick press
        G8RTOS_WaitSemaphore(&sem_Joystick_Debounce);
        // Sleep to debounce
        sleep(100);

        // Read the joystick switch status on the Multimod board.
        // Why do I need to do this? Can I just flip joystick_y? If we are here means joy was pressed

        // Toggle the joystick_y flag.
        joystick_y = !joystick_y;

        // Clear the interrupt
        GPIOIntClear(GPIO_PORTD_BASE, GPIO_PIN_2);
        // Re-enable the interrupt so it can occur again.
        GPIOIntEnable(GPIO_PORTD_BASE, GPIO_PIN_2);
    }
}



/********************************Periodic Threads***********************************/

void Print_WorldCoords(void) {
    // Print the camera position through UART to display on console.
    G8RTOS_WaitSemaphore(&sem_UART);
    UARTprintf("Cam Pos, X: %d, Y: %d, Z:, %d\n", world_camera_pos.x, world_camera_pos.y, world_camera_pos.z);
    G8RTOS_SignalSemaphore(&sem_UART);
}

void Get_Joystick(void) {
    // Read the joystick
    // Trigger the ADC conversion
    ADC0_PSSI_R = ADC_PSSI_SS2;

    // Wait for the conversion to complete
    while ((ADC0_RIS_R & ADC_RIS_INR2) == 0);
    uint32_t joy_x =  ADC0_SSFIFO2_R;
    uint32_t joy_y = ADC0_SSFIFO2_R;
    // Send through FIFO.
    G8RTOS_WriteFIFO(JOYSTICK_FIFO, joy_x);
    G8RTOS_WriteFIFO(JOYSTICK_FIFO, joy_y);
}



/*******************************Aperiodic Threads***********************************/

void GPIOE_Handler() {
    // Disable interrupt

    GPIOIntDisable(GPIO_PORTE_BASE, GPIO_PIN_4);
    // Signal relevant semaphore
    G8RTOS_SignalSemaphore(&sem_PCA9555_Debounce);
}

void GPIOD_Handler() {
    // Disable interrupt
    GPIOIntDisable(GPIO_PORTD_BASE, GPIO_PIN_2);
    // Signal relevant semaphore
    G8RTOS_SignalSemaphore(&sem_Joystick_Debounce);
}
