

/* File:   adxl345_test1.c
   Author: M. P. Hayes, UCECE
   Date:   3 December 2022
   Descr:  Read from an ADXL345 accelerometer and write its output to the USB serial.
*/
#include "pio.h"
#include "nrf24.h"
#include <stdio.h>
#include "delay.h"
#include "target.h"
#include "pacer.h"
#include "usb_serial.h"
#include "adxl345.h"
#include "panic.h"
#include "ledtape.h"
#include "pwm.h"
#include "adc.h"
#include "ledbuffer.h"

#define RADIO_CHANNEL 4
#define RADIO_ADDRESS 0x2777777777LL
#define channel1 1
#define channel2 2
#define channel3 3
#define channel4 4
#define RADIO_PAYLOAD_SIZE 32

#define PACER_RATE 50
#define ACCEL_POLL_RATE 1

#define NUM_LEDS 21

#define PWM_FREQ_F 349
#define PWM_FREQ_G 391
#define PWM_FREQ_C 261
#define PWM_FREQ_D 294
#define PWM_FREQ_E 330
#define PWM_FREQ_A  220
#define PWM_FREW_B 247

#ifndef LED_ACTIVE
#define LED_ACTIVE PIO_OUTPUT_LOW
#endif

#ifndef LED_OFF
#define LED_OFF PIO_OUTPUT_HIGH
#endif

#define main_freq 240
#define accel_freq 30
#define transmit_freq 80
#define rx_freq 40
#define ledtape_freq

static const adc_cfg_t adc_cfg =
{
    .bits = 12,
    .channels = BIT (ADC_CHANNEL_3),
    .trigger = ADC_TRIGGER_SW,
    .clock_speed_kHz = 1000
};

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

static twi_cfg_t adxl345_twi_cfg =
{
    .channel = TWI_CHANNEL_0,
    .period = TWI_PERIOD_DIVISOR (100000), // 100 kHz
    .slave_addr = 0x53
};

void pio_config(void)
{

    REG_CCFG_SYSIO |= CCFG_SYSIO_SYSIO4;

    pio_config_set (LED_ERROR_PIO, LED_ACTIVE);
    pio_output_set (LED_ERROR_PIO, PIO_OUTPUT_HIGH);
    pio_config_set (LED_STATUS_PIO, LED_ACTIVE);
    pio_output_set (LED_STATUS_PIO, PIO_OUTPUT_HIGH);
    pio_config_set (LED_GREEN_PIO, LED_ACTIVE);
    pio_output_set (LED_GREEN_PIO, PIO_OUTPUT_HIGH);

    pio_config_set (DIP_SWITCH_1, PIO_INPUT);
    pio_config_set (BUTTON_SLEEP_PIO, PIO_INPUT);
    pio_config_set (DIP_SWITCH_2, PIO_INPUT);


}

void turn_led_on(int r, int g, int b)
{

        uint8_t leds[NUM_LEDS * 3];
    int i;

    for (i = 0; i < NUM_LEDS; i++)
    {
        // Set full green  GRB order
        leds[i * 3] = r;
        leds[i * 3 + 1] = g;
        leds[i * 3 + 2] = b;
    }

    ledtape_write (LEDTAPE_PIO, leds, NUM_LEDS * 3);

    // //uint8_t leds[NUM_LEDS * 3];
    // bool blue = false;
    // int count = 0;
    // ledbuffer_t *leds = ledbuffer_init (LEDTAPE_PIO, NUM_LEDS);

    // pacer_wait();
    
    // if (count++ == NUM_LEDS)
    // {
    //     // wait for a revolution
    //     ledbuffer_clear(leds);
    //     if (blue)
    //     {
    //         ledbuffer_set(leds, 0, 0, 0, 255);
    //         ledbuffer_set(leds, NUM_LEDS / 2, 0, 0, 255);
    //     }
    //     else
    //     {
    //         ledbuffer_set(leds, 0, 255, 0, 0);
    //         ledbuffer_set(leds, NUM_LEDS / 2, 255, 0, 0);
    //     }
    //     blue = !blue;
    //     count = 0;
    // }

    // ledbuffer_write (leds);
    // ledbuffer_advance (leds, 1);
}

nrf24_t * initradio (void)
{
    int channelt;
    channelt = RADIO_CHANNEL;
    if (pio_input_get(DIP_SWITCH_1))
    {
        if (pio_input_get(DIP_SWITCH_2))
        {
            //pio_output_set(LED_STATUS_PIO, LED_ACTIVE);
            channelt = channel1;
        } else {
            //pio_output_set(LED_STATUS_PIO, ! LED_ACTIVE);
            channelt = channel2;
        }
    } else {
        if (pio_input_get(DIP_SWITCH_2))
        {
            //pio_output_set(LED_ERROR_PIO, LED_ACTIVE);
            channelt = channel3;
        } else {
            //pio_output_set(LED_ERROR_PIO, ! LED_ACTIVE);
            channelt = channel4;
        }
    
    }
    printf("%d\n", channelt);

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
            .channel = channelt,
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

void battery_measure(adc_t *adc)
{
    // 4096 = 7.7V
    // value/4096*7.7 = Voltage
    uint16_t data[1];
    static int count_adc=0;
    adc_read (*adc, data, sizeof (data));
    //printf ("%3d: %d\n", count_adc, data[0]);
    count_adc ++;
    //uint8_t leds[NUM_LEDS * 3];
    //int i;

    // for (i = 0; i < NUM_LEDS; i++)
    // {
    //     Set full green  GRB order
    //     leds[i * 3] = 0;
    //     leds[i * 3 + 1] = 255;
    //     leds[i * 3 + 2] = 0;
    // }

    if (data[0] < 2660) 
    {
        pio_output_set (LED_STATUS_PIO, !LED_ACTIVE);
        //printf("low \n");
    } else {
        //printf("good\n");
        pio_output_set(LED_STATUS_PIO, LED_ACTIVE);
        //ledtape_write (LEDTAPE_PIO, leds, NUM_LEDS * 3);
        //pio_output_set (LED_STATUS_PIO, !LED_ACTIVE);
    }
}



int
main (void)
{
    twi_t adxl345_twi;
    adxl345_t *adxl345;
    uint8_t ticks = 0;
    int count = 0;
    int16_t accel[3];
    int16_t motor[3];

    bool flag = false;

    //pwm stuff
    pwm_t PIEZO_PWMA;
    pwm_t PIEZO_PWMF;
    pwm_t PIEZO_PWMG;
    pwm_t PIEZO_PWMC;
    pwm_t PIEZO_PWMD;
    pwm_t PIEZO_PWME;

     adc_t adc;

        // Redirect stdio to USB serial
    usb_serial_stdio_init ();
    pio_config();


    adc = adc_init(&adc_cfg);

    if (! adc){
        panic (LED_GREEN_PIO, 2);
    }
    PIEZO_PWMA = pwm_init (&PIEZO_cfgA);
    if (! PIEZO_PWMA)
        panic (LED_ERROR_PIO, 1);

    PIEZO_PWMF = pwm_init (&PIEZO_cfgF);
    if (! PIEZO_PWMF)
        panic (LED_ERROR_PIO, 1);

    PIEZO_PWMG = pwm_init (&PIEZO_cfgG);
    if (! PIEZO_PWMG)
        panic (LED_ERROR_PIO, 1);

    PIEZO_PWMC = pwm_init (&PIEZO_cfgC);
    if (! PIEZO_PWMC)
        panic (LED_ERROR_PIO, 1);

    PIEZO_PWMD = pwm_init (&PIEZO_cfgD);
    if (! PIEZO_PWMD)
        panic (LED_ERROR_PIO, 1);
    
    PIEZO_PWME = pwm_init (&PIEZO_cfgE);
    if (! PIEZO_PWME)
        panic (LED_ERROR_PIO, 1);
    
    pwm_channels_start (pwm_channel_mask (PIEZO_PWMA));
    pwm_channels_start (pwm_channel_mask (PIEZO_PWMF));
    pwm_channels_start (pwm_channel_mask (PIEZO_PWMG));
    pwm_channels_start (pwm_channel_mask (PIEZO_PWMC));
    pwm_channels_start (pwm_channel_mask (PIEZO_PWMD));
    pwm_channels_start (pwm_channel_mask (PIEZO_PWME));


        // Initialise transmitter
    uint8_t count_tx = 0;
    nrf24_t *nrf;

    nrf = initradio();
    if (! nrf)
        panic (LED_GREEN_PIO, 1);

    // Initialise the TWI (I2C) bus for the ADXL345
    adxl345_twi = twi_init (&adxl345_twi_cfg);

    if (! adxl345_twi)
        panic (LED_ERROR_PIO, 1);

    // Initialise the ADXL345
    adxl345 = adxl345_init (adxl345_twi, ADXL345_ADDRESS);

    if (! adxl345)
        panic (LED_STATUS_PIO, 2);


    // uint8_t leds[NUM_LEDS * 3];
    // int i;

    // for (i = 0; i < NUM_LEDS; i++)
    // {
    //     // Set full green  GRB order
    //     leds[i * 3] = 0;
    //     leds[i * 3 + 1] = 255;
    //     leds[i * 3 + 2] = 0;
    // }

    turn_led_on(255,0,0);

    pacer_init (PACER_RATE);

    while (1)
    {
        /* Wait until next clock tick.  */
        pacer_wait ();

        char buffer[RADIO_PAYLOAD_SIZE + 1];
        char bufferi[RADIO_PAYLOAD_SIZE + 1];
        
        battery_measure(&adc);

        adxl345_accel_read (adxl345, accel);
        int motor_L_dir = 0;
        int motor_R_dir = 0;


        int16_t x_acc = accel[0];
        int16_t y_acc = accel[1];
        int16_t z_acc = accel[2];
        int16_t motor_L = motor[0];
        int16_t motor_R = motor[1];
        int z_dir = 0;

        uint8_t bytes;
        int buzz = 0;

        if ((x_acc >= -40) && (x_acc <= 60))
        {
            if (y_acc > 80) {
                motor_L = 20;
                motor_R = abs(y_acc * 100/230);
                motor_L_dir = 1;
                motor_R_dir = 1;
            }
            else if (y_acc < -60) {
                motor_L = abs(y_acc * 100/200);
                motor_R = 20;
                motor_L_dir = 1;
                motor_R_dir = 1;
            }
            else {
                motor_L = 0;
                motor_R = 0;
            }
        }
        else if (x_acc > 0)
        {
            motor_L_dir = 1;
            motor_R_dir = 1;
            if (y_acc > 60) {
                motor_L = 40;
                motor_R = abs(y_acc * 100/230);
            }
            else if (y_acc < -40) {
                motor_L = abs(y_acc * 100/200);
                motor_R = 40;
            }
            else {
                // motor_L = 100; // max speed forward
                // motor_R = 100;
                motor_L = abs(x_acc * 120/270);
                motor_R = abs(x_acc * 120/270);

            }
        } 
        else if (x_acc < -40)
        {
            // if (y_acc < -40) {
            //     motor_L = 40;
            //     motor_R = abs(y_acc * 100/260);
            // }
            // else if (y_acc > 60) {
            //     motor_L = abs(x_acc * 100/280);
            //     motor_R = 40;
            // }
            // motor_L = 100; // max speed backward
            // motor_R = 100;

            motor_L = abs(x_acc * 100/270);
            motor_R = abs(x_acc * 100/270);
            motor_L_dir = 0;
            motor_R_dir = 0;
            
        } 

        //sending accel data to the car
        if (ticks < 16) 
        {
            //ledtape_write (LEDTAPE_PIO, leds, NUM_LEDS * 3);
            snprintf (buffer, sizeof (buffer), "%5d %1d %5d %1d %5d %1d\n", motor_L, motor_L_dir, motor_R, motor_R_dir, z_acc, z_dir);
            if (!nrf24_write(nrf, buffer, RADIO_PAYLOAD_SIZE))
            {
                printf("Failed to send data\n");
                printf("%5d %1d %5d %1d %5d %1d\n", motor_L, motor_L_dir, motor_R, motor_R_dir, z_acc, z_dir);
                printf("%5d %5d %5d\n", x_acc, y_acc, z_acc);
            }
            else
            {
                printf("Sent data: %s\n", buffer);
                printf("%5d %5d %5d\n", x_acc, y_acc, z_acc);
                pio_output_toggle(LED_ERROR_PIO);
            }   
        }
        else if (ticks <= 20) 
        {
            bytes = nrf24_read (nrf, bufferi, RADIO_PAYLOAD_SIZE);
            if (bytes != 0)
            {
                bufferi[bytes] = 0;
                //printf("rx data: %s\n", buffer);
                pio_output_toggle (LED_STATUS_PIO);
                //printf("Car is hit!\n");
                pwm_duty_ppt_set(PIEZO_PWMA, PWM_FREQ_A);
                //printf("a\n");
                delay_ms(1000);
                //printf("b\n");
                pwm_duty_ppt_set(PIEZO_PWMF, PWM_FREQ_F);
                //printf("c\n");
                delay_ms(1000);
                //printf("d\n");
                pwm_duty_ppt_set(PIEZO_PWMF, 0);
                pwm_duty_ppt_set(PIEZO_PWMG, PWM_FREQ_G);
                delay_ms(500);
                pwm_duty_ppt_set(PIEZO_PWMC, PWM_FREQ_C);
                delay_ms(500);
                pwm_duty_ppt_set(PIEZO_PWMF, PWM_FREQ_F);
                delay_ms(500);
                pwm_duty_ppt_set(PIEZO_PWMG, PWM_FREQ_G);
                delay_ms(500);
                pwm_duty_ppt_set(PIEZO_PWMA, PWM_FREQ_A);
                delay_ms(500);
                pwm_duty_ppt_set(PIEZO_PWMF, PWM_FREQ_F);
                delay_ms(500);
                pwm_duty_ppt_set(PIEZO_PWMF, 0);
                flag = true;

                // else 
                // {
                //     flag = false;
                //     printf("no input\n");
                // }
            }     

            //if (sscanf(bufferi, "%d", &buzz) == 1)
            //{
                //printf("read");
                // if (buzz == ){
                //     if (flag = false) 
                //     {
                //         printf("Car is hit!\n");
                //         pwm_duty_ppt_set(PIEZO_PWMA, PWM_FREQ_A);
                //         printf("a\n");
                //         delay_ms(1000);
                //         printf("b\n");
                //         pwm_duty_ppt_set(PIEZO_PWMF, PWM_FREQ_F);
                //         printf("c\n");
                //         delay_ms(1000);
                //         printf("d\n");
                //         pwm_duty_ppt_set(PIEZO_PWMF, 0);
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
                //         flag = true;
                    //}
                //}
            //} else {
                //flag = false;
                //printf("no input\n");
            //}
        }
        if (ticks >= 20)
        {
            ticks = 0;
        }
        ticks++;
        printf("%d\n", ticks);
    }


}


