
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
#include "adc.h"

#define M2B2_PIO PA0_PIO
#define M1A2_PIO PA1_PIO
#define M1A1_PIO PA2_PIO
#define M2B1_PIO PB14_PIO
#define Bumper_PIO PB0_PIO
#define Extra_PIO PA7_PIO
#define HSLEEP_PIO PA28_PIO
#define BATTERY_VOLTAGE PA22_PIO

#define PACER_RATE 2
#define PWM_FREQ_HZ 1e3

#define RADIO_CHANNEL 2
#define RADIO_ADDRESS 0x7222222222LL
#define RADIO_PAYLOAD_SIZE 32

static const adc_cfg_t adc_cfg =
{
    .bits = 12,
    .channels = BIT (ADC_CHANNEL_9),
    .trigger = ADC_TRIGGER_SW,
    .clock_speed_kHz = 1000
};

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

void initPIO (void)
{
    pio_config_set (LED_ERROR_PIO, PIO_OUTPUT_HIGH);
    pio_config_set (LED_STATUS_PIO, PIO_OUTPUT_HIGH);
    pio_config_set (LED_LOW_POWER_PIO, PIO_OUTPUT_HIGH);
    pio_config_set (Bumper_PIO, PIO_INPUT);
    pio_config_set (HSLEEP_PIO, PIO_OUTPUT_HIGH);
    pio_config_set (BATTERY_VOLTAGE, PIO_OUTPUT_HIGH);
}

void bumperhit(void)
{
    char buffer[RADIO_PAYLOAD_SIZE + 1];

    if (! (pio_input_get(Bumper_PIO)))
    {
        printf("Hit \n");
        snprintf (buffer, sizeof (buffer), "Hit\n");
        pio_output_low (HSLEEP_PIO);
        delay_ms (10000);
        pio_output_high (HSLEEP_PIO);
        
    } else {
        printf("Not Hit \n");
    }

}

void battery_measure(adc_t *adc)
{
    // 4096 = 7.7V
    // value/4096*7.7 = Voltage
    uint16_t data[1];
    static int count_adc=0;
    adc_read (*adc, data, sizeof (data));
    printf ("%3d: %d\n", count_adc, data[0]);
    count_adc ++;
    if (data[0] < 2660) 
    {
        pacer_wait();
        pio_output_toggle (LED_LOW_POWER_PIO);
        printf("low \n");
    } else if (data[0] < 2926) {
        pio_output_low(LED_LOW_POWER_PIO);
    } else if (data[0] > 2950) {
        pio_output_high (LED_LOW_POWER_PIO);
    }

}

int main (void)
{
    adc_t adc;
    nrf24_t *nrf;
    pwm_t M1A1_PWM;
    pwm_t M1A2_PWM;
    pwm_t M2B1_PWM;
    pwm_t M2B2_PWM;

    int duty_setm1;
    int directionm1; // 1 = forward, 0 = backwards
    int duty_setm2;
    int directionm2;
    int zvalue;
    int zdirection;

    pacer_init (PACER_RATE);

    adc = adc_init (&adc_cfg);
    if (! adc)
        panic (LED_ERROR_PIO, 1);


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
    initPIO();
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

        bumperhit();
        battery_measure(&adc);
        bytes = nrf24_read (nrf, buffer, RADIO_PAYLOAD_SIZE);
        if (bytes != 0)
        {
            buffer[bytes] = 0;
            printf ("%s\n", buffer);
            pio_output_toggle (LED_STATUS_PIO);
        }
        
        if (sscanf(buffer, "%d %d %d %d %d %d", &duty_setm1, &directionm1, &duty_setm2, &directionm2, &zvalue, &zdirection) == 6){
            switch (directionm1) {
                case 1:
                    printf("%d %d \n", duty_setm1, directionm1);
                    pwm_duty_ppt_set(M1A1_PWM, duty_setm1*10);
                    pwm_duty_ppt_set(M1A2_PWM, 0);
                    break;
                case 0:
                    printf("%d %d \n", duty_setm1, directionm1);
                    pwm_duty_ppt_set(M1A2_PWM, duty_setm1*10);
                    pwm_duty_ppt_set(M1A1_PWM, 0);
                    break;
                default:
                    printf("Invalid operator: %d\n", directionm1);
                }

            switch (directionm2) {
                case 1:
                    printf("%d %d \n", duty_setm2, directionm2);
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
        //} else {
          //  printf("Invalid input\n");
        }
       // pio_output_toggle (LED_LOW_POWER_PIO);
        
    
    }
}


