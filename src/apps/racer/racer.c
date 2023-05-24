
#include "target.h"
#include "nrf24.h"
#include "pio.h"
#include "pacer.h"
#include "ledbuffer.h"
#include "ledtape.h"
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
#define DIP_SW1 PA23_PIO
#define DIP_SW2 PA20_PIO

#define PACER_RATE 2
#define PWM_FREQ_HZ 1e3
#define NUM_LEDS 20

#define RADIO_CHANNEL1 1
#define RADIO_CHANNEL2 2
#define RADIO_CHANNEL3 3
#define RADIO_CHANNEL4 4
#define RADIO_ADDRESS 0x2777777777LL //0x0123456789LL 0x2777777777LL
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
    //printf("in radio int");

    int radio_channel;
    bool dip1 = pio_input_get(DIP_SW1);
    bool dip2 = pio_input_get(DIP_SW2);
    printf("%d %d\n", dip1, dip2);
    
    switch (dip1){
        case 0:
            switch (dip2){
                case 0:
                    radio_channel = RADIO_CHANNEL1;

                    break;
                case 1:
                    radio_channel = RADIO_CHANNEL2;
                    break;
            }
            break;

        case 1:
            switch (dip2){
                case 0:
                    radio_channel = RADIO_CHANNEL3;
                    break;
                case 1:
                    radio_channel = RADIO_CHANNEL4;
                    break;  
             }
             break;
    }
    printf("radio channel: %d\n", radio_channel);

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
            .channel = radio_channel,
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
    pio_config_set (LED_STATUS_PIO, PIO_OUTPUT_LOW);
    pio_config_set (LED_LOW_POWER_PIO, PIO_OUTPUT_HIGH);
    pio_config_set (Bumper_PIO, PIO_INPUT);
    pio_config_set (HSLEEP_PIO, PIO_OUTPUT_HIGH);
    pio_config_set (BATTERY_VOLTAGE, PIO_OUTPUT_HIGH);
    pio_config_set (BUTTON_SLEEP_PIO, PIO_INPUT);
    pio_config_set (DIP_SW1, PIO_INPUT);
    pio_config_set (DIP_SW2, PIO_INPUT);
}




void check_sleep(bool *is_asleep)
{
    static bool prev_sleep_button_high = false;
    bool sleep_button_high = pio_input_get(BUTTON_SLEEP_PIO);

    if (!prev_sleep_button_high && sleep_button_high){ //button rising edge get
        if (!*is_asleep) //enter sleep mode
        {
            *is_asleep = true;
            //turn LEDs off
        } else if (*is_asleep) //exit sleep mode
        {
            *is_asleep = false;
        }
    }
    prev_sleep_button_high = sleep_button_high;
}


void ledtape(void)
{
    bool blue = false;
    int count = 0;

    ledbuffer_t *leds = ledbuffer_init (LEDTAPE_PIO, NUM_LEDS);

    pacer_init(10);


    pacer_wait();

    if (count++ == NUM_LEDS)
    {
        // wait for a revolution
        ledbuffer_clear(leds);
        if (blue)
        {
            ledbuffer_set(leds, 0, 0, 0, 255);
            ledbuffer_set(leds, NUM_LEDS / 2, 0, 0, 255);
        }
        else
        {
            ledbuffer_set(leds, 0, 255, 0, 0);
            ledbuffer_set(leds, NUM_LEDS / 2, 255, 0, 0);
        }
        blue = !blue;
        count = 0;
    }

    ledbuffer_write (leds);
    ledbuffer_advance (leds, 1);
    
}

//void battery_measure(adc_t *adc, bool *is_asleep)
int battery_measure(adc_t *adc)
{
    // 4096 = 7.7V
    // value/4096*7.7 = Voltage
    uint16_t data[1];
    static int count_adc=0;
    int low_power = 0;

    adc_read (*adc, data, sizeof (data));
    //printf ("%3d: %d\n", count_adc, data[0]);
    count_adc ++;
    if (data[0] < 2660) 
    {
        pacer_wait();
        pio_output_toggle (LED_LOW_POWER_PIO);
        //*is_asleep = true;
        printf("low \n");
        low_power = 1;
    } else if (data[0] < 2926) {
        pio_output_low(LED_LOW_POWER_PIO);
        printf("getting low \n");
    } else if (data[0] > 2950) {
        pio_output_high (LED_LOW_POWER_PIO);
        low_power =0;
    }
    return low_power;

}

/*
void sleep_func(bool *is_asleep)
{
    printf("asleep %d\n", *is_asleep);
    if(*is_asleep)
    {
        pio_output_low (HSLEEP_PIO);
        pio_output_low (LED_STATUS_PIO);
    } else if (!*is_asleep)
    {
        pio_output_high (HSLEEP_PIO);
        pio_output_high (LED_STATUS_PIO);
    }
}
*/

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
    int sent_counter = 0;
    int sent = 0;
    int hit = 0;
    int low_power =0;

    bool is_asleep = false;
    uint8_t ledsred[NUM_LEDS * 3];
    uint8_t ledsoff[NUM_LEDS * 3];
    int i;

    for (i = 0; i < NUM_LEDS; i++)
    {
        // Set full green  GRB order
        ledsred[i * 3] = 0;
        ledsred[i * 3 + 1] = 255;
        ledsred[i * 3 + 2] = 0;
        ledsoff[i * 3] = 0;
        ledsoff[i * 3 + 1] = 0;
        ledsoff[i * 3 + 2] = 0;
        
    }

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
        
        check_sleep(&is_asleep);
        low_power = battery_measure(&adc);
        if (low_power || is_asleep)
        {

            pwm_duty_ppt_set(M1A1_PWM, 0);
            pwm_duty_ppt_set(M1A2_PWM, 0);
            pwm_duty_ppt_set(M2B1_PWM, 0);
            pwm_duty_ppt_set(M2B2_PWM, 0);
            ledtape_write (LEDTAPE_PIO, ledsoff, NUM_LEDS * 3);
        } else 
        {
            ledtape_write (LEDTAPE_PIO, ledsred, NUM_LEDS * 3);
        }
        
        //sleep_func(&is_asleep);
        
        // Sending for bumper hit
        char buffert[RADIO_PAYLOAD_SIZE + 1];
        if (hit)
        {
            printf("Hit \n");
            pwm_duty_ppt_set(M1A1_PWM, 0);
            pwm_duty_ppt_set(M1A2_PWM, 0);
            pwm_duty_ppt_set(M2B1_PWM, 0);
            pwm_duty_ppt_set(M2B2_PWM, 0);

            sent = 0;
            snprintf (buffert, sizeof (buffert), "%d\n", 1);
            sent = nrf24_write (nrf, buffert, RADIO_PAYLOAD_SIZE);

            pio_output_toggle(LED_STATUS_PIO);
            printf("Not Sent %d\n", sent_counter);
            sent_counter ++;
            if (sent){
                printf("sent: %d\n", sent_counter);
                hit = 0;
                sent_counter = 0;
                pio_output_high(LED_STATUS_PIO);
                delay_ms (10000); // need to sort delay out, 1 not sending before delay
            }
        } else {
            if (! (pio_input_get(Bumper_PIO)))
            {
                hit = 1;

            } else {
                pio_output_low(LED_STATUS_PIO);
                printf("Not Hit \n");
            }

            
            //ledtape();
            //Receiving Motor info from hat
            
            char buffer[RADIO_PAYLOAD_SIZE + 1];
            uint8_t bytes;
            bytes = nrf24_read (nrf, buffer, RADIO_PAYLOAD_SIZE);
            if (bytes != 0)
            {
                buffer[bytes] = 0;
                printf ("recived buffer %s\n", buffer);
                
            } else{
                printf("not recieved\n");
            }
            if ((sscanf(buffer, "%d %d %d %d %d %d", &duty_setm1, &directionm1, &duty_setm2, &directionm2, &zvalue, &zdirection) == 6) && !(low_power || is_asleep))
            {
                switch (directionm1) {
                    case 1:
                        printf(" duty%d %d \n", duty_setm1, directionm1);
                        pwm_duty_ppt_set(M1A1_PWM, duty_setm1*10);
                        pwm_duty_ppt_set(M1A2_PWM, 0);
                        break;
                    case 0:
                        printf("duty%d %d \n", duty_setm1, directionm1);
                        pwm_duty_ppt_set(M1A2_PWM, duty_setm1*10);
                        pwm_duty_ppt_set(M1A1_PWM, 0);
                        break;
                    default:
                        printf("Invalid operator: %d\n", directionm1);
                    }

                switch (directionm2) {
                    case 1:
                        printf("duty2 %d %d \n", duty_setm2, directionm2);
                        pwm_duty_ppt_set(M2B1_PWM, duty_setm2*10);
                        pwm_duty_ppt_set(M2B2_PWM, 0);
                        break;
                    case 0:
                        printf("duty2 %d %d \n", duty_setm2, directionm2);
                        pwm_duty_ppt_set(M2B2_PWM, duty_setm2*10);
                        pwm_duty_ppt_set(M2B1_PWM, 0);
                        break;
                    default:
                        printf("Invalid operator: %d\n", directionm2);
                    }
            //} else {
                //printf("Invalid input\n");
            }

            
            /*
            if (pio_input_get(BUTTON_SLEEP_PIO))
            {
                //printf("Hit \n");
                pio_output_low(LED_ERROR_PIO);
                
            } else {
                //printf("Not Hit \n");
                pio_output_high(LED_ERROR_PIO);
            }
            */
        }
    }
}


