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
 *  file         : ch32v_pwm.c
 *  description  : ch32v pwm library main code
 *
 */

#include "ch32v_pwm.h"

/*********************************************************************
 * @fn      init_pwm_base
 *
 * @brief   Initialize handler for PWM structure. This makes a pin ready for PWM output.
 * 
 * @param   object      Pointer to PWM_handle struct to initialize
 * @param   iTimer      Timer to use for PWM (PWM_TIM1, PWM_TIM2, PWM_TIM3 or PWM_TIM4)
 * @param   iChannel    Channel of time to use for PWM (PWM_CH1, PWM_CH2, PWM_CH3 or PWM_CH4)
 * @param   iPin        Pin for outputting PWM signal (e.g 0xA8 for PA8 ...)
 * @param   iF_base     Base carrier frequency of PWM signal (e.g. 10000 = 10kHz) 
 *                      WARNIGN: Due to integer divison, the actual frequency only roughly follows the specified one.
 * @param   iCount      Base for scaling duty cycle (max = iCount + 1, min = 0) (e.g. 254 for 8-Bit resolution)
 * @param   iPwm_mode   PWM mode selection, changes precision of actual output (PWM_MODE1 or PWM_MODE2)
 *
 * @return  Normally returns 0 on exit, returns -1 if invalid pin specified
 */
int init_pwm_base(PWM_handle *object, uint8_t iTimer, uint8_t iChannel, int iPin, uint32_t iF_base, uint16_t iCount, uint16_t iPwm_mode)
{   
    // --------- Set attributes ----------
    object->pwm_mode = iPwm_mode;
    object->timer = iTimer;
    object->channel = iChannel;
    object->period = iCount;
    object->prescaler = SystemCoreClock / iCount / iF_base;     // Rough frequency match, only integer prescaler possible

    // ---------- Initialize ----------
    GPIO_InitTypeDef GPIO_InitStructure={0};
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure={0};

    // ---------- Set Pin as output ---------
    if (iPin < 0xa0 || iPin > 0xdf) return -1; // invalid pin number
    // based on pinMode() https://gist.github.com/bitbank2/13686b8a153a0b3a06839f4fa00589cb
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 << (iPin & 0xf);
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    switch (iPin & 0xf0) 
    {
        case 0xa0:
            RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
            GPIO_Init(GPIOA, &GPIO_InitStructure);
            break;
        case 0xb0:
            RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
            GPIO_Init(GPIOB, &GPIO_InitStructure);
            break;
        case 0xc0:
            RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
            GPIO_Init(GPIOC, &GPIO_InitStructure);
            break;
        case 0xd0:
            RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);
            GPIO_Init(GPIOD, &GPIO_InitStructure);
            break;
    }
    // ---------- Initialize Timer ----------
    TIM_TimeBaseInitStructure.TIM_Period = object->period;
	TIM_TimeBaseInitStructure.TIM_Prescaler = object->prescaler;
	TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
    switch (object->timer)
    {
        case PWM_TIM1:
            RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);
            TIM_TimeBaseInit( TIM1, &TIM_TimeBaseInitStructure);
            break;
        case PWM_TIM2:
            RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2 , ENABLE);
            TIM_TimeBaseInit( TIM2, &TIM_TimeBaseInitStructure);
            break;
        case PWM_TIM3:
            RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3 , ENABLE);
            TIM_TimeBaseInit( TIM3, &TIM_TimeBaseInitStructure);
            break;
        case PWM_TIM4:
            RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4 , ENABLE);
            TIM_TimeBaseInit( TIM4, &TIM_TimeBaseInitStructure);
            break;
    }
    return 0;
}

int var_init_pwm(init_pwm_args in)
{
    uint16_t iCount_out = in.iCount ? in.iCount : 254;
    uint16_t iPwm_mode_out = in.iPwm_mode ? in.iPwm_mode : PWM_MODE2;
    return init_pwm_base(in.object, in.iTimer, in.iChannel, in.iPin, in.iF_base, iCount_out, iPwm_mode_out);
}

/*********************************************************************
 * @fn      set_pwm_dutycycle
 *
 * @brief   Set duty cycle of PWM object
 * 
 * @param   object      Pointer to PWM_handle struct to control duty cycle of
 * @param   duty        Duty cycle (e.g. 8-Bit resultion -> [0:255])
 *
 * @return  None
 */
void set_pwm_dutycycle(PWM_handle *object, uint16_t duty)
{
    // ---------- Initialize base structure ----------
    TIM_OCInitTypeDef TIM_OCInitStructure={0};

    if (object->pwm_mode == PWM_MODE1)
    {
        TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
    }
    else
    {
        TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM2;
    }
    // ---------- Set Timer PWM duty cycle ----------
    if (duty > (object->period + 1))
    {
        object->duty_cycle = object->period + 1; // Clip invalid duty cycle to maximum (e.g. 255 for count = 254 -> always off)
    } 
    else 
    {
        object->duty_cycle = duty;
    }
    // invert for 255 = full on, 0 = full off
    object->duty_cycle = -(object->duty_cycle - (object->period + 1));
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCInitStructure.TIM_Pulse = object->duty_cycle;
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
    // ---------- Configure Timer and Channel ----------
    // This extensive selection matrix, should get optimized to few statements by compiler
    switch (object->timer)
    {
        case PWM_TIM1:
            switch (object->channel)
            {
                case PWM_CH1:
                    TIM_OC1Init( TIM1, &TIM_OCInitStructure );
                    TIM_CtrlPWMOutputs( TIM1, ENABLE );
                    TIM_OC1PreloadConfig( TIM1, TIM_OCPreload_Disable );
                    TIM_ARRPreloadConfig( TIM1, ENABLE );
                    break;
                case PWM_CH2:
                    TIM_OC2Init( TIM1, &TIM_OCInitStructure );
                    TIM_CtrlPWMOutputs( TIM1, ENABLE );
                    TIM_OC2PreloadConfig( TIM1, TIM_OCPreload_Disable );
                    TIM_ARRPreloadConfig( TIM1, ENABLE );
                    break;
                case PWM_CH3:
                    TIM_OC3Init( TIM1, &TIM_OCInitStructure );
                    TIM_CtrlPWMOutputs( TIM1, ENABLE );
                    TIM_OC3PreloadConfig( TIM1, TIM_OCPreload_Disable );
                    TIM_ARRPreloadConfig( TIM1, ENABLE );
                    break;
                case PWM_CH4:
                    TIM_OC4Init( TIM1, &TIM_OCInitStructure );
                    TIM_CtrlPWMOutputs( TIM1, ENABLE );
                    TIM_OC4PreloadConfig( TIM1, TIM_OCPreload_Disable );
                    TIM_ARRPreloadConfig( TIM1, ENABLE );
                    break;
            }
            break;
        case PWM_TIM2:
            switch (object->channel)
            {
                case PWM_CH1:
                    TIM_OC1Init( TIM2, &TIM_OCInitStructure );
                    TIM_CtrlPWMOutputs( TIM2, ENABLE );
                    TIM_OC1PreloadConfig( TIM2, TIM_OCPreload_Disable );
                    TIM_ARRPreloadConfig( TIM2, ENABLE );
                    break;
                case PWM_CH2:
                    TIM_OC2Init( TIM2, &TIM_OCInitStructure );
                    TIM_CtrlPWMOutputs( TIM2, ENABLE );
                    TIM_OC2PreloadConfig( TIM2, TIM_OCPreload_Disable );
                    TIM_ARRPreloadConfig( TIM2, ENABLE );
                    break;
                case PWM_CH3:
                    TIM_OC3Init( TIM2, &TIM_OCInitStructure );
                    TIM_CtrlPWMOutputs( TIM2, ENABLE );
                    TIM_OC3PreloadConfig( TIM2, TIM_OCPreload_Disable );
                    TIM_ARRPreloadConfig( TIM2, ENABLE );
                    break;
                case PWM_CH4:
                    TIM_OC4Init( TIM2, &TIM_OCInitStructure );
                    TIM_CtrlPWMOutputs( TIM2, ENABLE );
                    TIM_OC4PreloadConfig( TIM2, TIM_OCPreload_Disable );
                    TIM_ARRPreloadConfig( TIM2, ENABLE );
                    break;
            }
            break;
        case PWM_TIM3:
            switch (object->channel)
            {
                case PWM_CH1:
                    TIM_OC1Init( TIM3, &TIM_OCInitStructure );
                    TIM_CtrlPWMOutputs( TIM3, ENABLE );
                    TIM_OC1PreloadConfig( TIM3, TIM_OCPreload_Disable );
                    TIM_ARRPreloadConfig( TIM3, ENABLE );
                    break;
                case PWM_CH2:
                    TIM_OC2Init( TIM3, &TIM_OCInitStructure );
                    TIM_CtrlPWMOutputs( TIM3, ENABLE );
                    TIM_OC2PreloadConfig( TIM3, TIM_OCPreload_Disable );
                    TIM_ARRPreloadConfig( TIM3, ENABLE );
                    break;
                case PWM_CH3:
                    TIM_OC3Init( TIM3, &TIM_OCInitStructure );
                    TIM_CtrlPWMOutputs( TIM3, ENABLE );
                    TIM_OC3PreloadConfig( TIM3, TIM_OCPreload_Disable );
                    TIM_ARRPreloadConfig( TIM3, ENABLE );
                    break;
                case PWM_CH4:
                    TIM_OC4Init( TIM3, &TIM_OCInitStructure );
                    TIM_CtrlPWMOutputs( TIM3, ENABLE );
                    TIM_OC4PreloadConfig( TIM3, TIM_OCPreload_Disable );
                    TIM_ARRPreloadConfig( TIM3, ENABLE );
                    break;
            }
            break;
        case PWM_TIM4:
            switch (object->channel)
            {
                case PWM_CH1:
                    TIM_OC1Init( TIM4, &TIM_OCInitStructure );
                    TIM_CtrlPWMOutputs( TIM4, ENABLE );
                    TIM_OC1PreloadConfig( TIM4, TIM_OCPreload_Disable );
                    TIM_ARRPreloadConfig( TIM4, ENABLE );
                    break;
                case PWM_CH2:
                    TIM_OC2Init( TIM4, &TIM_OCInitStructure );
                    TIM_CtrlPWMOutputs( TIM4, ENABLE );
                    TIM_OC2PreloadConfig( TIM4, TIM_OCPreload_Disable );
                    TIM_ARRPreloadConfig( TIM4, ENABLE );
                    break;
                case PWM_CH3:
                    TIM_OC3Init( TIM4, &TIM_OCInitStructure );
                    TIM_CtrlPWMOutputs( TIM4, ENABLE );
                    TIM_OC3PreloadConfig( TIM4, TIM_OCPreload_Disable );
                    TIM_ARRPreloadConfig( TIM4, ENABLE );
                    break;
                case PWM_CH4:
                    TIM_OC4Init( TIM4, &TIM_OCInitStructure );
                    TIM_CtrlPWMOutputs( TIM4, ENABLE );
                    TIM_OC4PreloadConfig( TIM4, TIM_OCPreload_Disable );
                    TIM_ARRPreloadConfig( TIM4, ENABLE );
                    break;
            }
            break;
    }
}

/*********************************************************************
 * @fn      enable_pwm_output
 *
 * @brief   Enable PWM output
 * 
 * @param   object      Pointer to PWM_handle struct to enable output of
 *
 * @return  None
 */
void enable_pwm_output(PWM_handle *object)
{
    // ---------- Enable timer ----------
    switch (object->timer)
    {
        case PWM_TIM1:
            TIM_Cmd( TIM1, ENABLE );
            break;
        case PWM_TIM2:
            TIM_Cmd( TIM2, ENABLE );
            break;
        case PWM_TIM3:
            TIM_Cmd( TIM3, ENABLE );
            break;
        case PWM_TIM4:
            TIM_Cmd( TIM4, ENABLE );
            break;
    }
}

/*********************************************************************
 * @fn      disable_pwm_output
 *
 * @brief   Disable PWM output
 * 
 * @param   object      Pointer to PWM_handle struct to disable output of
 *
 * @return  None
 */
void disable_pwm_output(PWM_handle *object)
{
    // ---------- Disable timer ----------
    switch (object->timer)
    {
        case PWM_TIM1:
            TIM_Cmd( TIM1, DISABLE );
            break;
        case PWM_TIM2:
            TIM_Cmd( TIM2, DISABLE );
            break;
        case PWM_TIM3:
            TIM_Cmd( TIM3, DISABLE );
            break;
        case PWM_TIM4:
            TIM_Cmd( TIM4, DISABLE );
            break;
    }
}