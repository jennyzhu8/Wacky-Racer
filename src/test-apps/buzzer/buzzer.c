/* File:   buzzer.c
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
#include "pwm.h"

#define PACER_RATE 20
#define PWM_FREQ_C 260
#define PWM_FREQ_D 294
#define PWM_FREQ_E 330

#ifndef LED_ACTIVE
#define LED_ACTIVE PIO_OUTPUT_LOW
#endif

static const pwm_cfg_t PIEZO_cfgC =
{
    .pio = PIEZO_ADDRESS,
    .period = PWM_PERIOD_DIVISOR (PWM_FREQ_C),
    .duty = PWM_DUTY_DIVISOR (PWM_FREQ_C, 0),
    .align = PWM_ALIGN_LEFT,
    .polarity = PWM_POLARITY_HIGH,
    .stop_state = PIO_OUTPUT_LOW
};

static const pwm_cfg_t PIEZO_cfgD =
{
    .pio = PIEZO_ADDRESS,
    .period = PWM_PERIOD_DIVISOR (PWM_FREQ_D),
    .duty = PWM_DUTY_DIVISOR (PWM_FREQ_D, 0),
    .align = PWM_ALIGN_LEFT,
    .polarity = PWM_POLARITY_HIGH,
    .stop_state = PIO_OUTPUT_LOW
};

static const pwm_cfg_t PIEZO_cfgE =
{
    .pio = PIEZO_ADDRESS,
    .period = PWM_PERIOD_DIVISOR (PWM_FREQ_E),
    .duty = PWM_DUTY_DIVISOR (PWM_FREQ_E, 0),
    .align = PWM_ALIGN_LEFT,
    .polarity = PWM_POLARITY_HIGH,
    .stop_state = PIO_OUTPUT_LOW
};

int main(void)
{
    pwm_t PIEZO_PWMC;
    pwm_t PIEZO_PWMD;
    pwm_t PIEZO_PWME;

    PIEZO_PWMC = pwm_init (&PIEZO_cfgC);
    if (! PIEZO_PWMC)
        panic (LED_ERROR_PIO, 1);

    PIEZO_PWMD = pwm_init (&PIEZO_cfgD);
    if (! PIEZO_PWMD)
        panic (LED_ERROR_PIO, 1);
    
    PIEZO_PWME = pwm_init (&PIEZO_cfgE);
    if (! PIEZO_PWME)
        panic (LED_ERROR_PIO, 1);
    
    pwm_channels_start (pwm_channel_mask (PIEZO_PWMC));
    pwm_channels_start (pwm_channel_mask (PIEZO_PWMD));
    pwm_channels_start (pwm_channel_mask (PIEZO_PWME));

    while (1)
    {
        pwm_duty_ppt_set(PIEZO_PWMC, PWM_FREQ_C);
        delay_ms(500);
        pwm_duty_ppt_set(PIEZO_PWMD, PWM_FREQ_D);
        delay_ms(500);
        pwm_duty_ppt_set(PIEZO_PWME, PWM_FREQ_E);
        delay_ms(500);
    }
}
