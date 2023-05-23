/* File:   dip_switch.c
   Author: Jenny & Rohan
   Date:   
   Descr:  Simple dip switch test-> to change frequencies
*/
#include "mcu.h"
#include "pio.h"
#include "pacer.h"
#include "usb_serial.h"



#define PACER_RATE 100

int
main (void)
{
    /* Configure LED PIO as output.  */
    pio_config_set (LED_STATUS_PIO, PIO_OUTPUT_HIGH);
    pio_config_set (LED_GREEN_PIO, PIO_OUTPUT_HIGH);

    /* Configure button PIO as input with pullup.  */
    pio_config_set (DIP_SWITCH_1, PIO_INPUT);
    pio_config_set (BUTTON_SLEEP_PIO, PIO_INPUT);
    pio_config_set (DIP_SWITCH_2, PIO_INPUT);

    pacer_init (PACER_RATE);

    while (1)
    {
        /* Wait until next clock tick.  */
        pacer_wait ();

        // if (pio_input_get (DIP_SWITCH_1))
        //     pio_output_low (LED_STATUS_PIO);
        // else
        //     pio_output_high (LED_STATUS_PIO);

        // if (pio_input_get (BUTTON_SLEEP_PIO)) {
        //     printf("sleep button pressed");
        //     pio_output_low (LED_STATUS_PIO);
        // }
        // else {
        //     pio_output_high (LED_STATUS_PIO);

        // }
        if (pio_input_get (DIP_SWITCH_2)) {
            pio_output_high (LED_GREEN_PIO); 
            printf("dip switch pressed");
        }
        else {
            pio_output_low (LED_STATUS_PIO);
        }

        if (pio_input_get (DIP_SWITCH_1)) {
            pio_output_high (LED_STATUS_PIO); 

        }  
        else {
            pio_output_low (LED_ERROR_PIO);
        }

    }
}