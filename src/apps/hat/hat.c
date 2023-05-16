/* File:   main.c
   Author: JYZ RTAM
   Date:   today
*/

#include "target.h"
#include "usb_serial.h"
#include "adxl345.h"
#include "nrf24.h"
#include "pio.h"
#include "pacer.h"
#include "stdio.h"
#include "delay.h"
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

void get_accel_data(adxl345_t *adxl345, int16_t accel[3])
{
    adxl345_accel_read(adxl345, accel);
    
}

void send_acc_data(adxl345_t *adxl345, int16_t* accel) //, nrf24_t *nrf
{
    char buffer[RADIO_PAYLOAD_SIZE + 1];

    //get_accel_data(adxl345, accel);
    adxl345_accel_read(adxl345, accel);
    // printf ("x: %5d  y: %5d  z: %5d\n", accel[0], accel[1], accel[2]);
    // snprintf(buffer, sizeof(buffer), "x: %5hd  y: %5hd  z: %5hd\r\n", accel[0], accel[1], accel[2]);
    // // snprintf(buffer, sizeof(buffer), "x: %5hd  y: %5hd  z: %5hd\r\n", *accel, *(accel + 1), *(accel + 2));

    // if (!nrf24_write(nrf, buffer, RADIO_PAYLOAD_SIZE))
    // {
    //     printf("Failed to send data\n");
    // }
    // else
    // {
    //     printf("Sent data: %s\n", buffer);
    // }
}

// int16_t get_acc_data(uint8_t count_tx, adxl345_t *adxl345)
// {
//     // Initialise acceleration data
//     int16_t* accel[3];

//     // Print accelerometer status to hat serial port
//     if (! adxl345_is_ready (adxl345)) 
//         {
//             count_tx++;
//             printf ("Waiting for accelerometer to be ready... %d\n", count_tx);
//             // accel = [0 0 0];
//         }
//     else
//         {
//             if (adxl345_accel_read (adxl345, accel))
//             {
//                 printf ("x: %5d  y: %5d  z: %5d\n", accel[0], accel[1], accel[2]);
//             }
//             else
//             {
//                 printf ("ERROR: failed to read acceleration\n");
//             }
//         }

//     return accel;
// }

// void send_acc_data(int16_t accel, int count)
// {
//     char buffer[RADIO_PAYLOAD_SIZE + 1];

//         snprintf (buffer, sizeof (buffer), "x: %5d  y: %5d  z: %5d\r\n", accel[0], accel[1], accel[2], count++);

//         if (! nrf24_write (nrf, buffer, RADIO_PAYLOAD_SIZE))
//             pio_output_set (LED_ERROR_PIO, 0);
//         else
//             pio_output_set (LED_ERROR_PIO, 1);
// }

int
main (void)
{
    // Configure LED PIO as output.
    pio_config_set (LED_ERROR_PIO, LED_ACTIVE);
    pio_output_set (LED_ERROR_PIO, ! LED_ACTIVE);
    pio_config_set (LED_STATUS_PIO, LED_ACTIVE);
    pio_output_set (LED_STATUS_PIO, ! LED_ACTIVE);
    pio_config_set (LED_GREEN_PIO, LED_ACTIVE);
    pio_output_set (LED_GREEN_PIO, ! LED_ACTIVE);


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
            .address = RADIO_ADDRESS,
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

    // Initialise accelerometer
    twi_t adxl345_twi;
    adxl345_t *adxl345;
    int ticks = 0;
    int count = 0;
    int16_t accel[3];

    // Initialise the TWI (I2C) bus for the ADXL345
    adxl345_twi = twi_init (&adxl345_twi_cfg);

    if (! adxl345_twi)
        panic (LED_ERROR_PIO, 1);

    // Initialise the ADXL345
    adxl345 = adxl345_init (adxl345_twi, ADXL345_ADDRESS);

    if (! adxl345)
        panic (LED_STATUS_PIO, 2);

    pacer_init (PACER_RATE);

    // Redirect stdio to USB serial
    usb_serial_stdio_init ();

    while (1)
    {
        /* Wait until next clock tick.  */
        char buffer[RADIO_PAYLOAD_SIZE + 1];

        pacer_wait ();
        pio_output_toggle(LED_GREEN_PIO);

        //int16_t acc_data = get_acc_data(count_tx, adxl345);
        send_acc_data(adxl345, accel);



        snprintf (buffer, sizeof (buffer), "%5d %5d %5d\n", accel[0], accel[1], accel[2]);
        //snprintf (buffer, sizeof (buffer), "Group 13 Test\n");

        if (!nrf24_write(nrf, buffer, RADIO_PAYLOAD_SIZE))
            {
                printf("Failed to send data\n");
                printf("%d, %d, %d\n", accel[0], accel[1], accel[2]);
            }
        else
            {
                printf("Sent data: %s\n", buffer);
            }

        // ticks++;
        // if (ticks < PACER_RATE / ACCEL_POLL_RATE)
        //     continue;
        // ticks = 0;
    }

}

