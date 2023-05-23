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
#define PWM_FREQ_A 440
#define PWM_FREQ_F 349
#define PWM_FREQ_G 391
#define PWM_FREQ_C 261
#define PWM_FREQ_D 294
#define PWM_FREQ_E 330

#ifndef LED_ACTIVE
#define LED_ACTIVE PIO_OUTPUT_LOW
#endif

static const pwm_cfg_t PIEZO_cfgA =
{
    .pio = PIEZO_ADDRESS,
    .period = PWM_PERIOD_DIVISOR (PWM_FREQ_A),
    .duty = PWM_DUTY_DIVISOR (PWM_FREQ_A, 0),
    .align = PWM_ALIGN_LEFT,
    .polarity = PWM_POLARITY_HIGH,
    .stop_state = PIO_OUTPUT_LOW
};

static const pwm_cfg_t PIEZO_cfgF =
{
    .pio = PIEZO_ADDRESS,
    .period = PWM_PERIOD_DIVISOR (PWM_FREQ_F),
    .duty = PWM_DUTY_DIVISOR (PWM_FREQ_F, 0),
    .align = PWM_ALIGN_LEFT,
    .polarity = PWM_POLARITY_HIGH,
    .stop_state = PIO_OUTPUT_LOW
};

static const pwm_cfg_t PIEZO_cfgG =
{
    .pio = PIEZO_ADDRESS,
    .period = PWM_PERIOD_DIVISOR (PWM_FREQ_G),
    .duty = PWM_DUTY_DIVISOR (PWM_FREQ_G, 0),
    .align = PWM_ALIGN_LEFT,
    .polarity = PWM_POLARITY_HIGH,
    .stop_state = PIO_OUTPUT_LOW
};

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

// int main(void)
// {
//     pwm_t PIEZO_PWMA;
//     pwm_t PIEZO_PWMF;
//     pwm_t PIEZO_PWMG;
//     pwm_t PIEZO_PWMC;
//     pwm_t PIEZO_PWMD;
//     pwm_t PIEZO_PWME;

//     PIEZO_PWMA = pwm_init (&PIEZO_cfgA);
//     if (! PIEZO_PWMA)
//         panic (LED_ERROR_PIO, 1);

//     PIEZO_PWMF = pwm_init (&PIEZO_cfgF);
//     if (! PIEZO_PWMF)
//         panic (LED_ERROR_PIO, 1);

//     PIEZO_PWMG = pwm_init (&PIEZO_cfgG);
//     if (! PIEZO_PWMG)
//         panic (LED_ERROR_PIO, 1);

//     PIEZO_PWMC = pwm_init (&PIEZO_cfgC);
//     if (! PIEZO_PWMC)
//         panic (LED_ERROR_PIO, 1);

//     PIEZO_PWMD = pwm_init (&PIEZO_cfgD);
//     if (! PIEZO_PWMD)
//         panic (LED_ERROR_PIO, 1);
    
//     PIEZO_PWME = pwm_init (&PIEZO_cfgE);
//     if (! PIEZO_PWME)
//         panic (LED_ERROR_PIO, 1);
    
//     pwm_channels_start (pwm_channel_mask (PIEZO_PWMA));
//     pwm_channels_start (pwm_channel_mask (PIEZO_PWMF));
//     pwm_channels_start (pwm_channel_mask (PIEZO_PWMG));
//     pwm_channels_start (pwm_channel_mask (PIEZO_PWMC));
//     pwm_channels_start (pwm_channel_mask (PIEZO_PWMD));
//     pwm_channels_start (pwm_channel_mask (PIEZO_PWME));

//     while (1)
//     {
//         pwm_duty_ppt_set(PIEZO_PWMA, PWM_FREQ_A);
//         delay_ms(1000);
//         pwm_duty_ppt_set(PIEZO_PWMF, PWM_FREQ_F);
//         delay_ms(1000);
//         pwm_duty_ppt_set(PIEZO_PWMG, PWM_FREQ_G);
//         delay_ms(1000);
//         pwm_duty_ppt_set(PIEZO_PWMC, PWM_FREQ_C);
//         delay_ms(2000);
//         pwm_duty_ppt_set(PIEZO_PWMF, PWM_FREQ_F);
//         delay_ms(1000);
//         pwm_duty_ppt_set(PIEZO_PWMG, PWM_FREQ_G);
//         delay_ms(1000);
//         pwm_duty_ppt_set(PIEZO_PWMA, PWM_FREQ_A);
//         delay_ms(1000);
//         pwm_duty_ppt_set(PIEZO_PWMF, PWM_FREQ_F);
//         delay_ms(2000);
//     }
// }

pwm_t init_buzzer(pwm_t PIEZO_PWM, pwm_cfg_t PIEZO_cfg)
{
    PIEZO_PWM = pwm_init (&PIEZO_cfg);
    if (! PIEZO_PWM)
        panic (LED_ERROR_PIO, 1);
    
    pwm_channels_start (pwm_channel_mask (PIEZO_PWM));
    
    return PIEZO_PWM;
}

typedef struct channel_struct
{
    pwm_t pwm;
    int freq;
} channel_struct;

void turn_on_buzzer(channel_struct channels_array)
{
    pwm_duty_ppt_set(channels_array[0].pwm, channels_array[0].freq);
    delay_ms(1000);
    pwm_duty_ppt_set(channels_array[0].pwm, channels_array[0].freq);
    delay_ms(1000);
    pwm_duty_ppt_set(channels_array[0].pwm, channels_array[0].freq);
    delay_ms(1000);
    pwm_duty_ppt_set(channels_array[0].pwm, channels_array[0].freq);
    delay_ms(1000);
}

int main(void)
{
    pwm_t PIEZO_PWMC = init_buzzer(PIEZO_PWMC, PIEZO_cfgC);
    pwm_t PIEZO_PWMD = init_buzzer(PIEZO_PWMD, PIEZO_cfgD);
    pwm_t PIEZO_PWME = init_buzzer(PIEZO_PWME, PIEZO_cfgE);
    pwm_t PIEZO_PWMA = init_buzzer(PIEZO_PWMA, PIEZO_cfgA);
    channel_struct channels[4] = {{PIEZO_PWMC, PWM_FREQ_C}, {PIEZO_PWMD, PWM_FREQ_D}, {PIEZO_PWME, PWM_FREQ_E}, {PIEZO_PWMA, PWM_FREQ_A}};

    while (1)
    {
        turn_on_buzzer(channels);
    }
}