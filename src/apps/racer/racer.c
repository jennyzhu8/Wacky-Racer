
#include "target.h"
#include "nrf24.h"
#include "pio.h"
#include "pacer.h"
#include "stdio.h"
#include "delay.h"
#include "panic.h"
#include <stdio.h>
#include "pwm.h"
#include "usb_serial.h"
#include <string.h>

#define M2B2_PIO PA0_PIO
#define M1A2_PIO PA1_PIO
#define M1A1_PIO PA2_PIO
#define M2B1_PIO PB14_PIO

#define PWM_FREQ_HZ 1e3

#define RADIO_CHANNEL 1
#define RADIO_ADDRESS 0x0123456789LL
#define RADIO_PAYLOAD_SIZE 32

nrf24_t * initradio (void)
{
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
#ifdef RADIO_POWER_ENABLE_PIO
    // Enable radio regulator if present.
    pio_config_set (RADIO_POWER_ENABLE_PIO, PIO_OUTPUT_HIGH);
    delay_ms (10);
#endif

return nrf24_init (&nrf24_cfg);
}

/*
pwm_t initpwm (void)
{
    static const pwm_cfg_t M1A1_cfg =
    {
        .pio = M1A1_PIO,
        .period = PWM_PERIOD_DIVISOR (PWM_FREQ_HZ),
        .duty = PWM_DUTY_DIVISOR (PWM_FREQ_HZ, 0),
        .align = PWM_ALIGN_LEFT,
        .polarity = PWM_POLARITY_HIGH,
        .stop_state = PIO_OUTPUT_LOW
    };

    static const pwm_cfg_t M1A2_cfg =
    {
        .pio = M1A2_PIO,
        .period = PWM_PERIOD_DIVISOR (PWM_FREQ_HZ),
        .duty = PWM_DUTY_DIVISOR (PWM_FREQ_HZ, 0),
        .align = PWM_ALIGN_LEFT,
        .polarity = PWM_POLARITY_HIGH,
        .stop_state = PIO_OUTPUT_LOW
    };

    static const pwm_cfg_t M2B1_cfg =
    {
        .pio = M2B1_PIO,
        .period = PWM_PERIOD_DIVISOR (PWM_FREQ_HZ),
        .duty = PWM_DUTY_DIVISOR (PWM_FREQ_HZ, 0),
        .align = PWM_ALIGN_LEFT,
        .polarity = PWM_POLARITY_HIGH,
        .stop_state = PIO_OUTPUT_LOW
    };

    static const pwm_cfg_t M2B2_cfg =
    {
        .pio = M2B2_PIO,
        .period = PWM_PERIOD_DIVISOR (PWM_FREQ_HZ),
        .duty = PWM_DUTY_DIVISOR (PWM_FREQ_HZ, 0),
        .align = PWM_ALIGN_LEFT,
        .polarity = PWM_POLARITY_HIGH,
        .stop_state = PIO_OUTPUT_LOW
    };
}
*/
void initled (void)
{
    pio_config_set (LED_ERROR_PIO, PIO_OUTPUT_HIGH);
    pio_config_set (LED_STATUS_PIO, PIO_OUTPUT_HIGH);
    pio_config_set (LED_LOW_POWER_PIO, PIO_OUTPUT_HIGH);
}

int main (void)
{
    nrf24_t *nrf;
    pwm_t M1A1_PWM;
    pwm_t M1A2_PWM;
    pwm_t M2B1_PWM;
    pwm_t M2B2_PWM;

    //initpwm();
    static const pwm_cfg_t M1A1_cfg =
    {
        .pio = M1A1_PIO,
        .period = PWM_PERIOD_DIVISOR (PWM_FREQ_HZ),
        .duty = PWM_DUTY_DIVISOR (PWM_FREQ_HZ, 0),
        .align = PWM_ALIGN_LEFT,
        .polarity = PWM_POLARITY_HIGH,
        .stop_state = PIO_OUTPUT_LOW
    };

    static const pwm_cfg_t M1A2_cfg =
    {
        .pio = M1A2_PIO,
        .period = PWM_PERIOD_DIVISOR (PWM_FREQ_HZ),
        .duty = PWM_DUTY_DIVISOR (PWM_FREQ_HZ, 0),
        .align = PWM_ALIGN_LEFT,
        .polarity = PWM_POLARITY_HIGH,
        .stop_state = PIO_OUTPUT_LOW
    };

    static const pwm_cfg_t M2B1_cfg =
    {
        .pio = M2B1_PIO,
        .period = PWM_PERIOD_DIVISOR (PWM_FREQ_HZ),
        .duty = PWM_DUTY_DIVISOR (PWM_FREQ_HZ, 0),
        .align = PWM_ALIGN_LEFT,
        .polarity = PWM_POLARITY_HIGH,
        .stop_state = PIO_OUTPUT_LOW
    };

    static const pwm_cfg_t M2B2_cfg =
    {
        .pio = M2B2_PIO,
        .period = PWM_PERIOD_DIVISOR (PWM_FREQ_HZ),
        .duty = PWM_DUTY_DIVISOR (PWM_FREQ_HZ, 0),
        .align = PWM_ALIGN_LEFT,
        .polarity = PWM_POLARITY_HIGH,
        .stop_state = PIO_OUTPUT_LOW
    };
    initled();
    usb_serial_stdio_init ();
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

    nrf = initradio();
    if (! nrf)
        panic (LED_ERROR_PIO, 2);
    while (1) 
    {
        char buffer[RADIO_PAYLOAD_SIZE + 1];
        uint8_t bytes;

        bytes = nrf24_read (nrf, buffer, RADIO_PAYLOAD_SIZE);
        if (bytes != 0)
        {
            buffer[bytes] = 0;
            printf ("%s\n", buffer);
            pio_output_toggle (LED_STATUS_PIO);
        }
        
        pwm_duty_ppt_set(M2B1_PWM, 500);
    }
}


