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
 *  file         : main.c
 *  description  : example main code
 *
 */
#if defined(CH32V00X)
#include <ch32v00x.h>
#elif defined(CH32V10X)
#include <ch32v10x.h>
#elif defined(CH32V20X)
#include <ch32v20x.h>
#elif defined(CH32V30X)
#include <ch32v30x.h>
#elif defined(CH32X035) || defined(CH32X033)
#include <ch32x035.h>
#endif

#include "ch32v_usb_serial.h"
#include "ch32v_pwm.h"

#define TRUE 1
#define FALSE 0


/*
This example program provides a PWM output on 3 pins of the CH32V203C8T6-EVT-R0 (PA8, PA6, PB8).
Additonally, it periodically prints out the current duty cycle of PA8 via USBD serial port
and by typing '+' or '-' into the serial console, the duty cycle of PA8 can be increased or reduced accordingly (8-Bit resolution).
Note:
    - The actual PWM frequency might differ from the specified frequency due to rough integer divison.
    - The last two arguments (iCount, iPwm_mode) of init_pwm() are optional
*/

int main(void)
{
    // ---------- Initialization Code ----------
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
    SystemCoreClockUpdate();
    Delay_Init();

    // Initialize PWM on PA8 (TIM1_CH1) with 10kHz Base frequency
    PWM_handle PWM_A8={0};
    init_pwm(&PWM_A8, PWM_TIM1, PWM_CH1, 0xA8, 10000);
    set_pwm_dutycycle(&PWM_A8, 191);              // set  PWM duty cycle to ~75% (191/255) initially, setting dutycycle automatically starts PWM output
    // Initialize PWM on PA6 (TIM3_CH1) with 20kHz Base frequency
    PWM_handle PWM_A6={0};
    init_pwm(&PWM_A6, PWM_TIM3, PWM_CH1, 0xA6, 20000);
    set_pwm_dutycycle(&PWM_A6, 128);              // set  PWM duty cycle to 50% (128/255) initially, setting dutycycle automatically starts PWM output
    // Initialize PWM on PA7 (TIM3_CH2) with 20kHz Base frequency
    PWM_handle PWM_A7={0};
    init_pwm(&PWM_A7, PWM_TIM3, PWM_CH2, 0xA7, 20000);
    set_pwm_dutycycle(&PWM_A7, 128);              // set  PWM duty cycle to 50% (128/255) initially, setting dutycycle automatically starts PWM output
    disable_pwm_output(&PWM_A7);                  // Disable output, demonstrate independant channels
    // Initialize PWM on PB8 (TIM4_CH3) with 40kHz Base frequency
    PWM_handle PWM_B8={0};
    init_pwm(&PWM_B8, PWM_TIM4, PWM_CH3, 0xB8, 40000);
    set_pwm_dutycycle(&PWM_B8, 51);              // set  PWM duty cycle to ~20% (51/255) initially, setting dutycycle automatically starts PWM output

    // Comment out if PA9/PA10 wanted as PWM Output
    USART_Printf_Init(115200);
    printf("CH32V203_EVT PWM Demo - Starting ...\r\n");

    // Setup Code USB Serial
    USB_Serial_initialize();
    // Variables for testing CDC
    uint32_t counter1s = 0;
    uint8_t power = 128;
    // ---------- Endless Loop Code ----------
    while ( 1 )
    {
        if(counter1s >= 200000)
        {
            // Call roughly every ~second
            counter1s = 0;
            USB_Serial_printf( "Power: %d \n", power);
        }
        if(counter1s % 10000)
        {
            // Call roughly every 100 milliseconds
            // Handle user input (either + or -)
            char input = (char)getch();
            if(input)
            {
                if(input == '+')
                {
                    USB_Serial_printf( "Power: %d \n", ++power);
                    //pwmHandle_A8.set_dutycycle( power );
                    set_pwm_dutycycle(&PWM_A8, power);
                }
                else if(input == '-')
                {
                    USB_Serial_printf( "Power: %d \n", --power);
                    //pwmHandle_A8.set_dutycycle( power );
                    set_pwm_dutycycle(&PWM_A8, power);
                }
            }
        }
        // Call on every loop
        counter1s++;
        //USB_Tx_runner();
    }
}
