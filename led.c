/*******************************************************************************
 *
 * Copyright (C) 2019 by Shilpi Gupta
 *
 ******************************************************************************/

/*
 * @file    led.c
 * @brief   Library definitions for blue led on the FRDM KL25Z MCU.
 * @vertion Project 2
 * @date    April 13, 2019
 */

#include "led.h"
#include "MKL25Z4.h"

void led_blue_init()
{
    // The blue LED is connected to PTD1.

    // Enable the clock to PORTD = bit 12 in the SCGC5 (System Clock Gate
    // Control) Register. SCGC5 used to enable/disable all PORT clocks.
    // Following same as: SIM->SCGC5 |= 0x1000;
    SIM->SCGC5 |= SIM_SCGC5_PORTD(1);

    // Make the PORTD pin 1 a GPIO pin.
    // For pin name PTD1, look for ALT# associated with PTD1. ALT# is 1.
    // In PORTx_PRCn register, bits 10-8 define Pin MUX Control functions.
    // ALT1 = MUX 0x001 = GPIO, PCR[1] = bit 1
    // Following same as: PORTD->PCR[1] = 0x100; (i.e. 0001 0000 0000)
    PORTD->PCR[1] = PORT_PCR_MUX(0x001);

    // Set direction of PORTD's pin 1 to output. PDDR (Port Data Direction) 
    // Register. Pin 1 = bit 1 in PDDR register (each port has a direction reg).
    // Following same as: GPIOD->PDDR |= 0x02 and same as PTD->PDDR |= 0x02;
    GPIOD->PDDR |= GPIO_PDDR_PDD(0x02);

    // Turn off LED.  PSOR (Port Set Output Register)
    //PTD->PSOR |= 0x02;
}

void set_led_blue_on()
{
    // Write to data register PORTD's PDOR (Port Data Output).
    // PCOR (Port Clear Output) register. Corresponding bit in PDORn is
    // set/cleared to logic 0. Same as: GPIOD->PDOR &= ~0x02.
    GPIOD->PCOR = 0x02;
}

void set_led_blue_off()
{
    // Write to data register PORTD's PDOR (Port Data Output) register.
    // PSOR (Port Set Output) register. Corresponding bit in PDORn is set to
    // logic 1. Same as: GPIOD->PDOR |= 0x02.
    GPIOD->PSOR = 0x02;
}

void delay(int ms)
{
    int i, j;
    for (i = 0; i < ms; i++)
    {
        for (j = 0; j < 7000; j++)
        {}
    }
}
