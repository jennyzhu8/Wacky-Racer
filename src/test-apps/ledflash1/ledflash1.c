/* File:   ledflash1.c
   Author: M. P. Hayes, UCECE
   Date:   15 May 2007
   Descr:  Flash an LED
*/
#include <pio.h>
#include "target.h"
#include "pacer.h"

/* Define LED flash rate in Hz.  */
#define LED_FLASH_RATE 2

/*
    This test app is the faithful blinky program.  It works as follows:
    1. set the LED pin as an output low (turns on the LED if LED active low).
    2. initialize a pacer for 200 Hz.
    3. wait for the next pacer tick.
    4. toggle the LED.
    5. go to step 3.

    Suggestions:
    * Add more LEDs.
    * Blink interesting patterns (S-O-S for example).
    * Make two LEDs blink at two separate frequencies.
*/

int
main (void)
{

    // needed to use the green LED, as it is using a JTAG Pin, bit must be put high to be used as a PIO
    REG_CCFG_SYSIO |= CCFG_SYSIO_SYSIO4;

    /* Configure STATUS LED PIO as output.  */
    pio_config_set (LED_STATUS_PIO, PIO_OUTPUT_LOW);
    pio_config_set (LED_ERROR_PIO, PIO_OUTPUT_LOW); 
    pio_config_set (LED_LOW_POWER_PIO, PIO_OUTPUT_LOW);

    pacer_init (LED_FLASH_RATE * 2);

    while (1)
    {
        /* Wait until next clock tick.  */
        pacer_wait ();

        // pio_config_set (LED_STATUS_PIO, PIO_OUTPUT_LOW);
        /* Toggle LED.  */
        pio_output_toggle (LED_STATUS_PIO);
<<<<<<< HEAD
        //pio_output_toggle (LED_ERROR_PIO); 
        pio_output_toggle (LED_GREEN_PIO);
=======
        pio_output_toggle (LED_ERROR_PIO); 
        pio_output_toggle (LED_LOW_POWER_PIO);
>>>>>>> f69a6f8a55b86c93db34fc66e180398045a945c5
    }
}
