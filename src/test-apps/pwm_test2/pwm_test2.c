/* File:   pwm_test2.c
   Author: M. P. Hayes, UCECE
   Date:   15 April 2013
   Descr:  This example starts two channels simultaneously; one inverted
           with respect to the other.
*/
#include <stdio.h>
#include "pwm.h"
#include "pio.h"
#include "delay.h"
#include "panic.h"
#include "usb_serial.h"
#include <string.h>

#define M2B2_PIO PA0_PIO
#define M1A2_PIO PA1_PIO
#define M1A1_PIO PA2_PIO
#define M2B1_PIO PB14_PIO

#define PWM_FREQ_HZ 1e3

pwm_cfg_t M1A1_cfg =
{
    .pio = M1A1_PIO,
    .period = PWM_PERIOD_DIVISOR (PWM_FREQ_HZ),
    .duty = PWM_DUTY_DIVISOR (PWM_FREQ_HZ, 50),
    .align = PWM_ALIGN_LEFT,
    .polarity = PWM_POLARITY_HIGH,
    .stop_state = PIO_OUTPUT_LOW
};

static const pwm_cfg_t M1A2_cfg =
{
    .pio = M1A2_PIO,
    .period = PWM_PERIOD_DIVISOR (PWM_FREQ_HZ),
    .duty = PWM_DUTY_DIVISOR (PWM_FREQ_HZ, 50),
    .align = PWM_ALIGN_LEFT,
    .polarity = PWM_POLARITY_HIGH,
    .stop_state = PIO_OUTPUT_LOW
};

static const pwm_cfg_t M2B1_cfg =
{
    .pio = M2B1_PIO,
    .period = PWM_PERIOD_DIVISOR (PWM_FREQ_HZ),
    .duty = PWM_DUTY_DIVISOR (PWM_FREQ_HZ, 50),
    .align = PWM_ALIGN_LEFT,
    .polarity = PWM_POLARITY_HIGH,
    .stop_state = PIO_OUTPUT_LOW
};

static const pwm_cfg_t M2B2_cfg =
{
    .pio = M2B2_PIO,
    .period = PWM_PERIOD_DIVISOR (PWM_FREQ_HZ),
    .duty = PWM_DUTY_DIVISOR (PWM_FREQ_HZ, 50),
    .align = PWM_ALIGN_LEFT,
    .polarity = PWM_POLARITY_HIGH,
    .stop_state = PIO_OUTPUT_LOW
};


int
main (void)
{
    pwm_t M1A1_PWM;
    pwm_t M1A2_PWM;
    pwm_t M2B1_PWM;
    pwm_t M2B2_PWM;

    // Redirect stdio to USB serial
    if (usb_serial_stdio_init () < 0)
        panic(LED_ERROR_PIO, 3);
    
    
    pio_config_set (LED_STATUS_PIO, PIO_OUTPUT_HIGH);

    M1A1_PWM = pwm_init (&M1A1_cfg);
    if (! M1A1_PWM)
        panic (LED_ERROR_PIO, 1);

    M1A2_PWM = pwm_init (&M1A2_cfg);
    if (! M1A2_PWM)
        panic (LED_ERROR_PIO, 2);

    M2B1_PWM = pwm_init (&M2B1_cfg);
    if (! M2B1_PWM)
        panic (LED_ERROR_PIO, 3);

    M2B2_PWM = pwm_init (&M2B2_cfg);
    if (! M2B2_PWM)
        panic (LED_ERROR_PIO, 4);

    pwm_channels_start (pwm_channel_mask (M1A1_PWM) | pwm_channel_mask (M1A2_PWM) | pwm_channel_mask (M2B1_PWM) | pwm_channel_mask (M2B2_PWM));

    printf("Motor input format: Motor1_duty Motor1_direction Motor2_duty Motor2_direction");
    while (1)
    {
        delay_ms (500);
        pio_output_toggle (LED_STATUS_PIO);

        char buf[256];
        if (fgets(buf, sizeof(buf), stdin)) {
            int duty_setm1;
            int directionm1; // 1 = forward, 0 = backwards
            int duty_setm2;
            int directionm2;

            // sscanf returns the number of input items successfully matched
            if (sscanf(buf, "%d %d %d %d" ,&duty_setm1, &directionm1, &duty_setm2, &directionm2) == 4) {
                switch (directionm1) {
                case 1:
                    printf("%d %d\n", duty_setm1, directionm1);
                    pwm_duty_ppt_set(M1A1_PWM, duty_setm1*10);
                    pwm_duty_ppt_set(M1A2_PWM, 0);
                    break;
                case 0:
                    printf("%d %d\n", duty_setm1, directionm1);
                    pwm_duty_ppt_set(M1A2_PWM, duty_setm1*10);
                    pwm_duty_ppt_set(M1A1_PWM, 0);
                    break;
                default:
                    printf("Invalid operator: %d\n", directionm1);
                }

                switch (directionm2) {
                case 1:
                    printf("%d %d\n", duty_setm2, directionm2);
                    pwm_duty_ppt_set(M2B1_PWM, duty_setm2*10);
                    pwm_duty_ppt_set(M2B2_PWM, 0);
                    break;
                case 0:
                    printf("%d %d\n", duty_setm2, directionm2);
                    pwm_duty_ppt_set(M2B2_PWM, duty_setm2*10);
                    pwm_duty_ppt_set(M2B1_PWM, 0);
                    break;
                default:
                    printf("Invalid operator: %d\n", directionm2);
                }
            } else {
                printf("Invalid input\n");
            }
        }
    }

    return 0;
}
