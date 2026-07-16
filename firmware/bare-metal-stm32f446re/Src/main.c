/**
 ******************************************************************************
 * @file           : main.c
 * @author         : Kelly Dianga
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2026 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */

#include <stdint.h>

#if !defined(__SOFT_FP__) && defined(__ARM_FP)
  #warning "FPU is not initialized, but the project is compiling for an FPU. Please initialize the FPU before use."
#endif
/* ========================
   Base addresses
   ======================== */
#define PERIPH_BASE   		(0x40000000UL)

#define AHB1_OFFSET	  		(0x00020000UL)
#define AHB1_BASE	    	(PERIPH_BASE + AHB1_OFFSET)

#define APB1_OFFSET			(0x00000000UL)
#define APB1_BASE			(PERIPH_BASE + APB1_OFFSET)

/* =========================
   Peripheral base addresses
   ========================= */
#define GPIOA_OFFSET    		(0x00000000UL)
#define GPIOA_BASE	  	    	(AHB1_BASE + GPIOA_OFFSET)

#define GPIOB_OFFSET			(0x00000400UL)
#define GPIOB_BASE				(AHB1_BASE + GPIOB_OFFSET)

#define RCC_OFFSET	    		(0x00003800UL)
#define RCC_BASE  	    		(AHB1_BASE + RCC_OFFSET)

#define TIM3_OFFSET				(0x00000400UL)
#define TIM3_BASE				(APB1_BASE + TIM3_OFFSET)

/* =========================
   Register Access Helper
   ========================= */
#define REG32(addr)         	(*(volatile uint32_t *)(addr))

/* =========================
   RCC Register Offsets
   ========================= */
#define RCC_AHB1ENR_OFFSET     (0x30UL)
#define RCC_APB1ENR_OFFSET     (0x40UL)

/* =========================
   RCC Register Definitions
   ========================= */
#define RCC_AHB1ENR            REG32(RCC_BASE + RCC_AHB1ENR_OFFSET)
#define RCC_APB1ENR            REG32(RCC_BASE + RCC_APB1ENR_OFFSET)

/* =========================
   GPIO Register Offsets
   ========================= */
#define GPIO_MODER_OFFSET      (0x00UL)
#define GPIO_ODR_OFFSET        (0x14UL)
#define GPIO_AFRL_OFFSET       (0x20UL)

/* =========================
   GPIO Register Definitions
   ========================= */
#define GPIOA_MODER            REG32(GPIOA_BASE + GPIO_MODER_OFFSET)
#define GPIOA_ODR              REG32(GPIOA_BASE + GPIO_ODR_OFFSET)

#define GPIOB_MODER            REG32(GPIOB_BASE + GPIO_MODER_OFFSET)
#define GPIOB_AFRL             REG32(GPIOB_BASE + GPIO_AFRL_OFFSET)


/* =========================
   TIM3 Register Offsets
   ========================= */
#define TIM3_CR1_OFFSET       (0x00UL)
#define TIM3_EGR_OFFSET		  (0x14UL)
#define TIM3_CCMR1_OFFSET	  (0x18UL)
#define TIM3_CCER_OFFSET	  (0x20UL)
#define TIM3_CNT_OFFSET        (0x24UL)
#define TIM3_PSC_OFFSET		  (0x28UL)
#define TIM3_ARR_OFFSET		  (0x2CUL)
#define TIM3_CCR1_OFFSET	  (0x34UL)


/* =========================
   TIM3 Register Definitions
   ========================= */
#define TIM3_CR1               REG32(TIM3_BASE + TIM3_CR1_OFFSET)
#define TIM3_EGR               REG32(TIM3_BASE + TIM3_EGR_OFFSET)
#define TIM3_CCMR1             REG32(TIM3_BASE + TIM3_CCMR1_OFFSET)
#define TIM3_CCER              REG32(TIM3_BASE + TIM3_CCER_OFFSET)
#define TIM3_CNT               REG32(TIM3_BASE + TIM3_CNT_OFFSET)
#define TIM3_PSC               REG32(TIM3_BASE + TIM3_PSC_OFFSET)
#define TIM3_ARR               REG32(TIM3_BASE + TIM3_ARR_OFFSET)
#define TIM3_CCR1              REG32(TIM3_BASE + TIM3_CCR1_OFFSET)


/* =========================
   RCC Peripheral Enable Bits
   ========================= */
#define GPIOAEN				   (1U << 0)
#define GPIOBEN				   (1U << 1)
#define TIM3EN				   (1U << 1)

/* =========================
   TIM3 Control Bits
   ========================= */
#define TIM_CR1_CEN     	  (1U << 0)
#define TIM_CR1_ARPE     	  (1U << 7)

/* =========================
   TIM3 Event Generation Bits
   ========================= */
#define TIM_EGR_UG            (1U << 0)    // Update generation


/* =========================
   TIM3 Capture/Compare Mode Bits
   ========================= */
#define TIM_CCMR1_OC1PE      (1U << 3)    // Output compare 1 preload enable
#define TIM_CCMR1_OC1M_PWM1  (0x6U << 4)  // PWM mode 1
#define TIM_CCMR1_OC1M_MASK  (0x7U << 4)  // OC1M bit mask
#define TIM_CCMR1_CC1S_MASK  (0x3U << 0)  // Channel 1 selection mask

/* =========================
   TIM3 PWM Output Bits
   ========================= */
#define TIM_CCER_CC1E   	 (1U << 0)

/* =========================
   GPIOB PB4 TIM3_CH1 Pin Mapping
   ========================= */
#define PB4_MODE_BIT0        (1U << 8)
#define PB4_MODE_BIT1        (1U << 9)

#define PB4_AFRL_SHIFT       (16U)
#define GPIO_AF2_TIM3        (0x2U)
#define GPIO_AFR_MASK        (0xFU)

/* =========================
   Output Pins
   ========================= */
#define WARNING_LED			 (1U << 8) // PA8
#define SECURITY_LIGHT		 (1U << 4) // PB4


void security_light_pwm_init(void)
{
    /* 1. Enable GPIOB clock */
    RCC_AHB1ENR |= GPIOBEN;

    /* 2. Configure PB4 as alternate function mode */
    GPIOB_MODER &= ~PB4_MODE_BIT0;
    GPIOB_MODER |=  PB4_MODE_BIT1;

    /* 3. Select AF2 for PB4: PB4 -> TIM3_CH1 */
    GPIOB_AFRL &= ~(GPIO_AFR_MASK << PB4_AFRL_SHIFT);  // Clear PB4 AF bits
    GPIOB_AFRL |=  (GPIO_AF2_TIM3 << PB4_AFRL_SHIFT);  // Set AF2

    /* 4. Enable TIM3 clock */
    RCC_APB1ENR |= TIM3EN;

    /* 5. Set TIM3 prescaler */
    TIM3_PSC = 160 - 1;

    /* 6. Set PWM period */
    TIM3_ARR = 100 - 1;

    /* 7. Set duty cycle */
    TIM3_CCR1 = 50;  // 50% duty cycle

    /* 8. Configure Channel 1 as PWM mode 1 */
    TIM3_CCMR1 &= ~TIM_CCMR1_CC1S_MASK;  // CC1S = 00, output mode
    TIM3_CCMR1 &= ~TIM_CCMR1_OC1M_MASK;  // Clear OC1M bits
    TIM3_CCMR1 |=  TIM_CCMR1_OC1M_PWM1;  // Set OC1M = 110, PWM mode 1

    /* 9. Enable preload for Channel 1 */
    TIM3_CCMR1 |= TIM_CCMR1_OC1PE;

    /* 10. Enable TIM3 Channel 1 output */
    TIM3_CCER |= TIM_CCER_CC1E;

    /* 11. Enable auto-reload preload */
    TIM3_CR1 |= TIM_CR1_ARPE;

    /* 12. Generate update event */
    TIM3_EGR |= TIM_EGR_UG;

    /* 13. Reset counter */
    TIM3_CNT = 0;

    /* 14. Start TIM3 */
    TIM3_CR1 |= TIM_CR1_CEN;
}

int main(void)
{
    security_light_pwm_init();

    /* Enable GPIOA clock for warning LED */
    RCC_AHB1ENR |= GPIOAEN;

    /* Configure PA8 as output mode */
    GPIOA_MODER |=  (1U << 16);
    GPIOA_MODER &= ~(1U << 17);

    while (1)
    {
        /* Toggle warning LED */
        GPIOA_ODR ^= WARNING_LED;

        for (volatile int i = 0; i < 1000000; i++) {}
    }
}
