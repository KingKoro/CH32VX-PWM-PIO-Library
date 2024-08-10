/**
 *  CH32VX PWM Library
 *
 *  Copyright (c) 2024 Florian Korotschenko aka KingKoro
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to deal
 *  in the Software without restriction, including without limitation the rights
 *  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *  copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in
 *  all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 *  THE SOFTWARE.
 *
 *
 *  file         : ch32v_pwm.h
 *  description  : ch32v pwm library main header
 *
 */

#ifndef __CH32V_PWM_H
#define __CH32V_PWM_H

#ifdef __cplusplus
extern "C" {
#endif

#include "debug.h"

// PWM Timers
#define PWM_TIM1    1
#define PWM_TIM2    2
#define PWM_TIM3    3
#define PWM_TIM4    4

// PWM Channels
#define PWM_CH1     1
#define PWM_CH2     2
#define PWM_CH3     3
#define PWM_CH4     4

// PWM Output Mode Definition
#define PWM_MODE1   0
#define PWM_MODE2   1

// PWM Object handler struct
typedef struct
{
    uint16_t pwm_mode;      // PWM Mode
    uint8_t timer;          // Timer
    uint8_t channel;        // Channel of Timer
    uint16_t prescaler;     // Prescaler of Timer
    uint16_t period;        // Max. counter of Timer PWM output
    uint16_t duty_cycle;    // Duty Cycle of PWM output
} PWM_handle;

// Initializer function for PWM_handle (also let iCount default to 254 and iPwm_mode to PWM_MODE2 if not specified)
int init_pwm_base(PWM_handle *object, uint8_t iTimer, uint8_t iChannel, uint16_t u16Pin, uint32_t iF_base, uint16_t iCount, uint16_t iPwm_mode);
// input structure for variadic args
typedef struct 
{
    PWM_handle *object;
    uint8_t iTimer;
    uint8_t iChannel;
    uint16_t u16Pin;
    uint32_t iF_base;
    uint16_t iCount;
    uint16_t iPwm_mode;
} init_pwm_args;
// placeholder for default args
int var_init_pwm(init_pwm_args in);                                 
/*********************************************************************
 * @fn      init_pwm_base
 *
 * @brief   Initialize handler for PWM structure. This makes a pin ready for PWM output.
 * 
 * @param   object      Pointer to PWM_handle struct to initialize
 * @param   iTimer      Timer to use for PWM (PWM_TIM1, PWM_TIM2, PWM_TIM3 or PWM_TIM4)
 * @param   iChannel    Channel of time to use for PWM (PWM_CH1, PWM_CH2, PWM_CH3 or PWM_CH4)
 * @param   u16Pin      Pin for outputting PWM signal (e.g 0x0A08 for PA8 ...)
 * @param   iF_base     Base carrier frequency of PWM signal (e.g. 40000 = 40kHz)
 * @param   iCount      Base for scaling duty cycle (max = iCount + 1, min = 0) (e.g. 254 for 8-Bit resolution)
 * @param   iPwm_mode   PWM mode selection, changes precision of actual output (PWM_MODE1 or PWM_MODE2)
 *
 * @return  Normally returns 0 on exit, returns -1 if invalid pin specified
 */
#define init_pwm(...) var_init_pwm((init_pwm_args){__VA_ARGS__});
// Function to set/update duty cycle
extern void set_pwm_dutycycle(PWM_handle *object, uint16_t duty);
// Function to enable PWM output
extern void enable_pwm_output(PWM_handle *object);
// Function to disable PWM output
extern void disable_pwm_output(PWM_handle *object);

#ifdef __cplusplus
}
#endif

#endif