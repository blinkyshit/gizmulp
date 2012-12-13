#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <math.h>

#include <stddef.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <avr/pgmspace.h>
#include <stdarg.h>
#include <stdlib.h>

// Bit manipulation macros
#define sbi(a, b) ((a) |= 1 << (b))       //sets bit B in variable A
#define cbi(a, b) ((a) &= ~(1 << (b)))    //clears bit B in variable A
#define tbi(a, b) ((a) ^= 1 << (b))       //toggles bit B in variable A

#define BAUD 38400
#define UBBR (F_CPU / 16 / BAUD - 1)
#define DEBUG 0

#define NUM_LED 2
#define NUM_DATA (NUM_LED * 3)

#define COLOR_LATCH_DURATION 501
#define CLOCK_PERIOD 10
#define CLOCK_PIN  3
#define DATA_PIN   4
#define CLOCK_PORT PORTD
#define DATA_PORT  PORTD

typedef struct
{
        uint8_t red, green, blue;
} color_t;

void delay_ms(int ms) 
{ 
    int i;
    for (i = 0; i < ms; i++) 
        _delay_ms(1); 
}

void delay_us(int us) 
{ 
    int i;
    for (i = 0; i < us; i++) 
        _delay_us(1); 
}

void ledstick_setup(void)
{
    // setting clock and data ports
    DDRD |= (1<<PD3)|(1<<PD4);

    // on board LED
    DDRB |= (1<<PB5);
}

void set_led_bytes(uint8_t *leds)
{
    uint8_t i, l, c, byte;

    for(l = 0; l < NUM_LED; l++)
        for(c = 0; c < 3; c++)
            {
                byte = leds[c];
                for(i = 0; i < 8; i++)
                {
                    if (byte & (1 << (8 - i)))
                        sbi(DATA_PORT, DATA_PIN);
                    else
                        cbi(DATA_PORT, DATA_PIN);
                    delay_us(CLOCK_PERIOD);

                    sbi(CLOCK_PORT, CLOCK_PIN);
                    delay_us(CLOCK_PERIOD);

                    cbi(CLOCK_PORT, CLOCK_PIN);
                }
            }
     delay_us(COLOR_LATCH_DURATION);
}

void set_led_color(color_t *color)
{
    set_led_bytes((uint8_t*)color);
}

void set_led_rgb(uint8_t red, uint8_t green, uint8_t blue)
{
    uint8_t led[3];
    led[0] = red;
    led[1] = green;
    led[2] = blue;
    set_led_bytes(led);
}

void startup(void)
{
    int i;

    for(i = 0; i < 3; i++)
    {
        set_led_rgb(255, 255, 0);
        delay_ms(200);

        set_led_rgb(255, 0, 255);
        delay_ms(200);
    }
}

void panic(uint16_t t, color_t *c)
{
    c->red =  (int)((sin((float)t / 50) + 1.0) * 127);
    c->blue =  (int)((cos((float)t / 50) + 1.0) * 127);
    c->green = 0;
}

void drink_done(uint16_t t, color_t *c)
{
    c->green =  (int)((sin((float)t / M_PI_2) + 1.0) * 127);
    c->blue = c->red = 0;
}

void orange(uint16_t t, color_t *c)
{
    c->red   = (int)((sin((float)t / 30) + 1.0) * 127);
    c->green = (int)((sin((float)t / 30) + 1.0) * 64);
    c->blue  = 0;
}

void plot_function(uint16_t count, uint16_t delay, void (*func)(uint16_t, color_t *))
{
    uint16_t i;
    color_t  c;
 
    for(i = 0; i< count; i++)
    {
        func(i, &c);
        set_led_color(&c);
        delay_ms(delay);
    }
}

void fade(uint16_t steps, uint16_t delay, color_t *from, color_t *to)
{
    float    rstep, gstep, bstep;
    uint16_t i;
    color_t  c;

    rstep = ((float)to->red - (float)from->red) / steps;
    gstep = ((float)to->green - (float)from->green) / steps;
    bstep = ((float)to->blue - (float)from->blue) / steps;

    for(i = 0; i < steps; i++)
    {
        c.red = from->red + (int16_t)(i * rstep);
        c.green = from->green + (int16_t)(i * gstep);
        c.blue = from->blue + (int16_t)(i * bstep);
        set_led_color(&c);
        delay_ms(delay);
    }
}

#define NUM_GIZMULP_COLORS 9
uint8_t gizmulp_colors[9][3] =
{
    { 255, 0, 0},
    { 255, 64, 0 },
    { 255, 128, 0 },
    { 255, 192, 0 },
    { 255, 255, 0 },
    { 192, 255, 0 },
    { 128, 255, 0 },
    { 64, 255, 0 },
    { 0, 255, 0 }
};

int main(void)
{
    uint8_t i, j;

    ledstick_setup();
    sei();
    startup();

    i = random() % NUM_GIZMULP_COLORS;
    for(;;)
    {
        do 
        {
            j = random() % NUM_GIZMULP_COLORS;
        } while(i == j && (abs(i - j < 3)));
        fade(500, 5, (color_t *)gizmulp_colors[i], (color_t *)gizmulp_colors[j]);
        i = j;
    }
}
