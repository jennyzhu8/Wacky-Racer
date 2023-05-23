// /* File:   main.c
//    Author: JYZ RTAM
//    Date:   today
// */
// #include <stdio.h>
// #include "usb_serial.h"
// #include "adxl345.h"
// #include "nrf24.h"
// #include "pio.h"
// #include "pacer.h"
// #include "delay.h"
// #include "panic.h"

// #define RADIO_CHANNEL 4
// #define RADIO_ADDRESS 0x0123456789LL
// #define RADIO_PAYLOAD_SIZE 32

// #define PACER_RATE 20
// #define ACCEL_POLL_RATE 1

// #ifndef LED_ACTIVE
// #define LED_ACTIVE PIO_OUTPUT_LOW
// #endif

// static twi_cfg_t adxl345_twi_cfg =
// {
//     .channel = TWI_CHANNEL_0,
//     .period = TWI_PERIOD_DIVISOR (100000), // 100 kHz
//     .slave_addr = 0x53
// };

// void send_acc_data(adxl345_t *adxl345, int16_t* accel) //, nrf24_t *nrf
// {
//     adxl345_accel_read(adxl345, accel);
// }


// //initialise all pio values
// void pio_config(void)
// {
//     pio_config_set (LED_ERROR_PIO, LED_ACTIVE);
//     pio_output_set (LED_ERROR_PIO, 1);
//     pio_config_set (LED_STATUS_PIO, LED_ACTIVE);
//     pio_output_set (LED_STATUS_PIO, 1);
//     // pio_config_set (LED_GREEN_PIO, LED_ACTIVE);
//     // pio_output_set (LED_GREEN_PIO, ! LED_ACTIVE);

//     // pio_config_set (DIP_SWITCH_1, PIO_INPUT);
//     // pio_config_set (BUTTON_SLEEP_PIO, PIO_INPUT);
//     // pio_config_set (DIP_SWITCH_2, PIO_INPUT);
// }

// int main (void)
// {
//     // Configure LED PIO as output.
//     pio_config();

//     //initialise micro

//     // Initialise transmitter
//     spi_cfg_t spi_cfg =
//         {
//             .channel = 0,
//             .clock_speed_kHz = 1000,
//             .cs = RADIO_CS_PIO,
//             .mode = SPI_MODE_0,
//             .cs_mode = SPI_CS_MODE_FRAME,
//             .bits = 8
//         };


//     nrf24_cfg_t nrf24_cfg =
//         {
//             .channel = RADIO_CHANNEL,
//             .address = RADIO_ADDRESS,
//             .payload_size = RADIO_PAYLOAD_SIZE,
//             .ce_pio = RADIO_CE_PIO,
//             .irq_pio = RADIO_IRQ_PIO,
//             .spi = spi_cfg,
//         };
//     uint8_t count_tx = 0;
//     nrf24_t *nrf;

//     #ifdef RADIO_POWER_ENABLE_PIO
//     // Enable radio regulator if present.
//     pio_config_set (RADIO_POWER_ENABLE_PIO, PIO_OUTPUT_HIGH);
//     delay_ms (10);
//     #endif

//     nrf = nrf24_init (&nrf24_cfg);
//     if (! nrf)
//         panic (LED_GREEN_PIO, 2);

//     // Initialise accelerometer
//     twi_t adxl345_twi;
//     adxl345_t *adxl345;
//     int ticks = 0;
//     int count = 0;
//     int16_t accel[3];
//     int16_t motor[3];

//     // Initialise the TWI (I2C) bus for the ADXL345
//     adxl345_twi = twi_init (&adxl345_twi_cfg);

//     if (! adxl345_twi)
//         //panic (LED_GREEN_PIO, 1);

//     // Initialise the ADXL345
//     adxl345 = adxl345_init (adxl345_twi, ADXL345_ADDRESS);

//     if (! adxl345)
//         panic (LED_STATUS_PIO, 1);

//     //pacer_init (PACER_RATE);
//     pacer_init(PACER_RATE);
//     // Redirect stdio to USB serial
//     usb_serial_stdio_init ();
    
//     printf("printing\n");

//     if (pio_input_get(DIP_SWITCH_1)){
//         printf("1\n");
//     }
//     if(pio_input_get(DIP_SWITCH_2)){
//         printf("2\n");
//     }

//     while (1)
//     {
//         /* Wait until next clock tick.  */
//         char buffer[RADIO_PAYLOAD_SIZE + 1];
//         pacer_wait ();
//         pio_output_toggle(LED_GREEN_PIO);

//         //int16_t acc_data = get_acc_data(count_tx, adxl345);
//         send_acc_data(adxl345, accel);
//         int motor_L_dir = 0;
//         int motor_R_dir = 0;


//         int16_t x_acc = accel[0];
//         int16_t y_acc = accel[1];
//         int16_t z_acc = accel[2];
//         int16_t motor_L = motor[0];
//         int16_t motor_R = motor[1];
//         int z_dir = 0;

//         if ((x_acc >= -40) && (x_acc <= 60))
//         {
//             if (y_acc > 60) {
//                 motor_L = 0;
//                 motor_R = abs(y_acc * 100/260);
//                 motor_R_dir = 1;
//             }
//             else if (y_acc < -40) {
//                 motor_L = abs(y_acc * 100/230);
//                 motor_R = 0;
//                 motor_L_dir = 1;
//             }
//             else {
//                 motor_L = 0;
//                 motor_R = 0;
//             }
//         }
//         else if (x_acc > 60)
//         {
//             motor_L_dir = 1;
//             motor_R_dir = 1;
//             if (y_acc > 60) {
//                 motor_L = 40;
//                 motor_R = abs(y_acc * 100/260);
//             }
//             else if (y_acc < -40) {
//                 motor_L = abs(y_acc * 100/230);
//                 motor_R = 40;
//             }
//             else {
//                 // motor_L = 100; // max speed forward
//                 // motor_R = 100;
//                 motor_L = abs(x_acc * 100/270);
//                 motor_R = abs(x_acc * 100/270);

//             }
//         } 
//         else if (x_acc < -40)
//         {
//             // if (y_acc < -40) {
//             //     motor_L = 40;
//             //     motor_R = abs(y_acc * 100/260);
//             // }
//             // else if (y_acc > 60) {
//             //     motor_L = abs(x_acc * 100/280);
//             //     motor_R = 40;
//             // }
//             // motor_L = 100; // max speed backward
//             // motor_R = 100;

//             motor_L = abs(x_acc * 100/270);
//             motor_R = abs(x_acc * 100/270);
//             motor_L_dir = 0;
//             motor_R_dir = 0;
            
//         } 

//         snprintf (buffer, sizeof (buffer), "%5d %1d %5d %1d %5d %1d\n", motor_L, motor_L_dir, motor_R, motor_R_dir, z_acc, z_dir);

//         if (!nrf24_write(nrf, buffer, RADIO_PAYLOAD_SIZE))
//             {
//                 printf("Failed to send data\n");
//                 printf("%5d %1d %5d %1d %5d %1d\n", motor_L, motor_L_dir, motor_R, motor_R_dir, z_acc, z_dir);
//                 printf("%5d %5d %5d\n", x_acc, y_acc, z_acc);
//             }
//         else
//             {
//                 printf("Sent data: %s\n", buffer);
//                 printf("%5d %5d %5d\n", x_acc, y_acc, z_acc);
//             }

//         // ticks++;
//         // if (ticks < PACER_RATE / ACCEL_POLL_RATE)
//         //     continue;
//         // ticks = 0;
//     }

// }

/* File:   adxl345_test1.c
   Author: M. P. Hayes, UCECE
   Date:   3 December 2022
   Descr:  Read from an ADXL345 accelerometer and write its output to the USB serial.
*/
#include "pio.h"
#include "nrf24.h"
#include <stdio.h>
#include "delay.h"
#include "target.h"
#include "pacer.h"
#include "usb_serial.h"
#include "adxl345.h"
#include "panic.h"

#define RADIO_CHANNEL 4
#define RADIO_ADDRESS 0x0123456789LL
#define RADIO_PAYLOAD_SIZE 32

#define PACER_RATE 20
#define ACCEL_POLL_RATE 1

#ifndef LED_ACTIVE
#define LED_ACTIVE PIO_OUTPUT_LOW
#endif

static twi_cfg_t adxl345_twi_cfg =
{
    .channel = TWI_CHANNEL_0,
    .period = TWI_PERIOD_DIVISOR (100000), // 100 kHz
    .slave_addr = 0x53
};

int pio_config(void)
{
    int address;
    pio_config_set (LED_ERROR_PIO, LED_ACTIVE);
    pio_output_set (LED_ERROR_PIO, ! LED_ACTIVE);
    pio_config_set (LED_STATUS_PIO, LED_ACTIVE);
    pio_output_set (LED_STATUS_PIO, ! LED_ACTIVE);

    pio_config_set (DIP_SWITCH_1, PIO_INPUT);
    pio_config_set (BUTTON_SLEEP_PIO, PIO_INPUT);
    pio_config_set (DIP_SWITCH_2, PIO_INPUT);

    address = 0x0123456789LL;
    // if (pio_input_get(DIP_SWITCH_1))
    // {
    //     if (pio_input_get(DIP_SWITCH_2))
    //     {
    //         pio_output_set(LED_STATUS_PIO, LED_ACTIVE);
    //         address = 0x0123456789LL;
    //     } else {
    //         pio_output_set(LED_STATUS_PIO, ! LED_ACTIVE);
    //         address = 0x2345678901LL;
    //     }
    // } else {
    //     if (pio_input_get(DIP_SWITCH_2))
    //     {
    //         pio_output_set(LED_ERROR_PIO, LED_ACTIVE);
    //         address = 0x3456789012LL;
    //     } else {
    //         pio_output_set(LED_ERROR_PIO, ! LED_ACTIVE);
    //         address = 0x4567890123;
    //     }
    // }
    return (address);
}

int
main (void)
{
    twi_t adxl345_twi;
    adxl345_t *adxl345;
    int ticks = 0;
    int count = 0;
    int16_t accel[3];
    int16_t motor[3];

    int address; 

    // Redirect stdio to USB serial
    usb_serial_stdio_init ();

    address = pio_config();
    printf ("%5d\n", address);

        // Initialise transmitter
    spi_cfg_t spi_cfg =
        {
            .channel = 0,
            .clock_speed_kHz = 1000,
            .cs = RADIO_CS_PIO,
            .mode = SPI_MODE_0,
            .cs_mode = SPI_CS_MODE_FRAME,
            .bits = 8
        };

    nrf24_cfg_t nrf24_cfg =
        {
            .channel = RADIO_CHANNEL,
            .address = address,
            .payload_size = RADIO_PAYLOAD_SIZE,
            .ce_pio = RADIO_CE_PIO,
            .irq_pio = RADIO_IRQ_PIO,
            .spi = spi_cfg,
        };
    uint8_t count_tx = 0;
    nrf24_t *nrf;

    #ifdef RADIO_POWER_ENABLE_PIO
    // Enable radio regulator if present.
    pio_config_set (RADIO_POWER_ENABLE_PIO, PIO_OUTPUT_HIGH);
    delay_ms (10);
    #endif

    nrf = nrf24_init (&nrf24_cfg);
    if (! nrf)
        panic (LED_GREEN_PIO, 2);

    // Initialise the TWI (I2C) bus for the ADXL345
    adxl345_twi = twi_init (&adxl345_twi_cfg);

    if (! adxl345_twi)
        panic (LED_ERROR_PIO, 1);

    // Initialise the ADXL345
    adxl345 = adxl345_init (adxl345_twi, ADXL345_ADDRESS);

    if (! adxl345)
        panic (LED_STATUS_PIO, 2);

    pacer_init (PACER_RATE);

    while (1)
    {
        /* Wait until next clock tick.  */
        pacer_wait ();
        char buffer[RADIO_PAYLOAD_SIZE + 1];
        char bufferi[RADIO_PAYLOAD_SIZE + 1];

        adxl345_accel_read (adxl345, accel);
        int motor_L_dir = 0;
        int motor_R_dir = 0;


        int16_t x_acc = accel[0];
        int16_t y_acc = accel[1];
        int16_t z_acc = accel[2];
        int16_t motor_L = motor[0];
        int16_t motor_R = motor[1];
        int z_dir = 0;

        uint8_t bytes;
        int buzz = 0;

        if ((x_acc >= -40) && (x_acc <= 60))
        {
            if (y_acc > 60) {
                motor_L = 0;
                motor_R = abs(y_acc * 100/260);
                motor_R_dir = 1;
            }
            else if (y_acc < -40) {
                motor_L = abs(y_acc * 100/230);
                motor_R = 0;
                motor_L_dir = 1;
            }
            else {
                motor_L = 0;
                motor_R = 0;
            }
        }
        else if (x_acc > 60)
        {
            motor_L_dir = 1;
            motor_R_dir = 1;
            if (y_acc > 60) {
                motor_L = 40;
                motor_R = abs(y_acc * 100/260);
            }
            else if (y_acc < -40) {
                motor_L = abs(y_acc * 100/230);
                motor_R = 40;
            }
            else {
                // motor_L = 100; // max speed forward
                // motor_R = 100;
                motor_L = abs(x_acc * 100/270);
                motor_R = abs(x_acc * 100/270);

            }
        } 
        else if (x_acc < -40)
        {
            // if (y_acc < -40) {
            //     motor_L = 40;
            //     motor_R = abs(y_acc * 100/260);
            // }
            // else if (y_acc > 60) {
            //     motor_L = abs(x_acc * 100/280);
            //     motor_R = 40;
            // }
            // motor_L = 100; // max speed backward
            // motor_R = 100;

            motor_L = abs(x_acc * 100/270);
            motor_R = abs(x_acc * 100/270);
            motor_L_dir = 0;
            motor_R_dir = 0;
            
        } 

        snprintf (buffer, sizeof (buffer), "%5d %1d %5d %1d %5d %1d\n", motor_L, motor_L_dir, motor_R, motor_R_dir, z_acc, z_dir);

        if (!nrf24_write(nrf, buffer, RADIO_PAYLOAD_SIZE))
            {
                //printf("Failed to send data\n");
                //printf("%5d %1d %5d %1d %5d %1d\n", motor_L, motor_L_dir, motor_R, motor_R_dir, z_acc, z_dir);
                //printf("%5d %5d %5d\n", x_acc, y_acc, z_acc);
            }
        else
            {
                //printf("Sent data: %s\n", buffer);
                //printf("%5d %5d %5d\n", x_acc, y_acc, z_acc);
            }   

        bytes = nrf24_read (nrf, bufferi, RADIO_PAYLOAD_SIZE);
        if (bytes != 0)
        {
            bufferi[bytes] = 0;
            printf ("%s\n", buffer);
            pio_output_toggle (LED_STATUS_PIO);
        }     

        if (sscanf(bufferi, "%d", &buzz) == 1)
        {
            //code to activate buzzer
            printf("Car is hit!\n");
        } else {
            printf("No input\n");
        }

    }
}


