
#include "stm32f4xx.h"
#include <stdio.h> // Keep for printf

// Define System Clock Frequency (assuming 16MHz HSI for F401 by default)
#define F_CPU 16000000UL 

// Global variables to store timer data
volatile uint32_t SysTick_count = 0;
volatile uint32_t Reaction_Time_Ticks = 0;
volatile uint8_t  Reaction_Measured = 0; // Flag to indicate measurement is complete

// Simple software delay function
void delay_ms(volatile uint32_t ms) {
    // This software delay is highly inefficient and only for demonstration.
    // In production code, use a timer or SysTick based delay.
    for (uint32_t i = 0; i < ms * 1000; i++);
}

// Function to initialize UART/USART for printf output in Keil
void Debug_Output_Init(void) {
    // Initialization for debug output (e.g., Keil Semihosting/ITM)
}

// Interrupt Handlers

// SysTick Handler (Fired every 1ms for this setup)
void SysTick_Handler(void) {
    SysTick_count++; // Increment our 1ms counter
}

// EXTI Line 6 Handler (for PA6)
void EXTI9_5_IRQHandler(void) {
    // Check if the interrupt pending bit for Line 6 is set
    if (EXTI->PR & (1U << 6)) {
        // Clear the interrupt pending flag by writing 1
        EXTI->PR = (1U << 6); 

        // 1. Stop SysTick Timer
        SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk; // Clear the ENABLE bit
        
        // 2. Store Reaction Time (in milliseconds)
        Reaction_Time_Ticks = SysTick_count;
        Reaction_Measured = 1; // Set flag
        
        // Indicate successful interrupt using PA11 (new LED pin)
        GPIOA->ODR |= (1U << 11); 
    }
}


// Initialization Functions

void SysTick_Init(void) {
    // Set reload value for 1ms interrupt (@ 16MHz)
    SysTick->LOAD = (F_CPU / 1000) - 1; 
    SysTick->VAL = 0;
    SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | 
                    SysTick_CTRL_TICKINT_Msk |   
                    SysTick_CTRL_ENABLE_Msk;     
    // Initially disable SysTick until needed
    SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk; 
}

void GPIO_Init(void) {
    // Enable Clock for GPIOA (LEDs and Touch Button)
    // We no longer need GPIOB or GPIOC clock enabled for LEDs
    RCC->AHB1ENR |= (1U << 0);  // GPIOA clock enable
    
    // --- Configure LED Pins (PA9, PA10, PA11) as General Purpose Output ---
    // PA9 (bits 18:19)
    GPIOA->MODER &= ~(3U << 18); // Clear mode bits
    GPIOA->MODER |= (1U << 18);  // Set to Output mode (01)
    
    // PA10 (bits 20:21)
    GPIOA->MODER &= ~(3U << 20); // Clear mode bits
    GPIOA->MODER |= (1U << 20);  // Set to Output mode (01)
    
    // PA11 (bits 22:23)
    GPIOA->MODER &= ~(3U << 22); // Clear mode bits
    GPIOA->MODER |= (1U << 22);  // Set to Output mode (01)

    // Ensure all LED pins are low initially (LEDs OFF)
    GPIOA->ODR &= ~((1U << 9) | (1U << 10) | (1U << 11));

    // Configure PA6 for Touch Button Input (EXTI) - REMAINS THE SAME
    GPIOA->MODER &= ~(3U << 12); 
    GPIOA->PUPDR &= ~(3U << 12); // No Pull-up/Pull-down
}

void EXTI_Init(void) {
    // Enable SYSCFG Clock
    RCC->APB2ENR |= (1U << 14); 

    // Select PA6 as the source for EXTI Line 6 - REMAINS THE SAME
    SYSCFG->EXTICR[1] &= ~(0xF << 4); 

    // Configure EXTI Line 6 - REMAINS THE SAME
    EXTI->RTSR |= (1U << 6); 
    EXTI->FTSR &= ~(1U << 6);
    EXTI->IMR |= (1U << 6); 

    // Enable the EXTI 9_5 vector in the NVIC - REMAINS THE SAME
    NVIC_EnableIRQ(EXTI9_5_IRQn);
}


// Main Function


int main(void) {
    
    // Initialize peripherals
    GPIO_Init();
    SysTick_Init();
    EXTI_Init();
    Debug_Output_Init(); // Initialize debug output (Keil's Serial/ITM/Semihosting)

    // Display welcome message via printf
    printf("\n--- Reaction Time Game ---\n");
    printf("Game Ready! Follow the lights...\n");
    delay_ms(2000);
    
    printf("Get Ready...\n");
    delay_ms(1000);
    
    // 1. Turn ON PA9
    GPIOA->ODR |= (1U << 9); 
    delay_ms(1000); 
    
    // 2. Turn ON PA10
    GPIOA->ODR |= (1U << 10); 
    delay_ms(1000); 

    // 3. Turn ON PA11
    GPIOA->ODR |= (1U << 11); 
    delay_ms(1500); // Wait longer while all are on
    
    // 4. Turn ALL OFF at once
    GPIOA->ODR &= ~((1U << 9) | (1U << 10) | (1U << 11));
    
    // Display "GO!" message
    printf("!!! GO! TOUCH PA6 NOW !!!\n");
    
    // Reset counter for measurement
    SysTick_count = 0;
    
    // Start SysTick Timer
    SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk; 

    while (1) {
        // Wait for the interrupt to fire
        
        if (Reaction_Measured) {
            // Display reaction time using printf
            printf("\n--- RESULTS ---\n");
            printf("Reaction Time: %lu ms\n", (unsigned long)Reaction_Time_Ticks);
            printf("-----------------\n");
            
            // Clear flag to prevent repeated updates
            Reaction_Measured = 0; 
            
            // Keep the result displayed
            while(1);
        }
    }
}
