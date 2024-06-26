/* File:   adc_usb_serial_test1.c
   Author: M. P. Hayes, UCECE
   Date:   3 May 2021
   Descr:  This reads from ADC channel 0.
           It triggers ADC conversions as each sample is read.
*/
#include <stdio.h>
#include "usb_serial.h"
#include "adc.h"
#include "pacer.h"
#include "panic.h"

#define PACER_RATE 2

static const adc_cfg_t adc_cfg =
{
    .bits = 12,
    .channels = BIT (ADC_CHANNEL_3),
    .trigger = ADC_TRIGGER_SW,
    .clock_speed_kHz = 1000
};


int main (void)
{
    //adc_t adc;
    int count = 0;
    int count_run = 0;
    char *c;
    usb_serial_t usb1;
    adc_t adc;

    pio_config_set (LED_STATUS_PIO, PIO_OUTPUT_LOW);
    // Redirect stdio to USB serial
    usb_serial_stdio_init ();


    adc = adc_init (&adc_cfg);
    if (! adc)
        panic (LED_ERROR_PIO, 1);

    pacer_init (PACER_RATE);

    while (1)
    {
        uint16_t data[1];

        pacer_wait ();
        
        adc_read (adc, data, sizeof (data));
        
        //usb_cdc_read(&usb1,c, sizeof(c) );
        //printf ("%s, count = %d\n", c, count++);
        
        printf ("%3d: %d\n", count++, data[0]);
        pio_output_toggle (LED_STATUS_PIO);
    }
}
