#include "stm32f4xx.h"
#include <stdio.h>
#include <string.h>

#define F_CPU 16000000UL

// LED Pins
#define LED1_PIN 6  
#define LED2_PIN 7  
#define LED3_PIN 8  

// Touch Button Pin
#define TOUCH_PIN 6  // PA6

// GAME CONFIGURATION 
#define ROUNDS_PER_GAME 1 

// Global variables
volatile uint32_t SysTick_count = 0;
volatile uint32_t Reaction_Time_Ticks = 0;
volatile uint8_t Reaction_Measured = 0;
volatile uint8_t Game_Started = 0;
volatile uint8_t Current_Player = 1;  // Player 1 or 2

// Game session tracking
uint8_t Game_Active = 0;  
uint8_t Current_Round = 0;  
uint32_t Player1_Times[10];  
uint32_t Player2_Times[10];
uint8_t Player1_Turn_Count = 0;
uint8_t Player2_Turn_Count = 0;

// UART receive buffer
volatile char UART_RX_Buffer[20];
volatile uint8_t UART_RX_Index = 0;
volatile uint8_t Command_Ready = 0;

// Function Prototypes
void delay_ms(volatile uint32_t ms);
void SysTick_Init(void);
void GPIO_Init(void);
void EXTI_Init(void);
void USART1_Init(void);
void USART1_SendChar(char c);
void USART1_SendString(const char* str);
void LED_Sequence(void);
void Reset_Game(void);
void Reset_Full_Game(void);
uint32_t Calculate_Average(uint32_t times[], uint8_t count);
void Declare_Winner(void);

// Simple software delay function
void delay_ms(volatile uint32_t ms) {
    for (uint32_t i = 0; i < ms * 1000; i++);
}

// SysTick Handler (1ms interrupt)
void SysTick_Handler(void) {
    SysTick_count++;
}

// EXTI Line 6 Handler (for PA6 - Touch Button)
void EXTI9_5_IRQHandler(void) {
    if (EXTI->PR & (1U << 6)) {
        EXTI->PR = (1U << 6);  
        
        if (Game_Started && !Reaction_Measured) {
            
            SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;
            
          
            Reaction_Time_Ticks = SysTick_count;
            Reaction_Measured = 1;
        }
    }
}

// USART1 IRQ Handler
void USART1_IRQHandler(void) {
    if (USART1->SR & USART_SR_RXNE) {
        char received = USART1->DR;
        
        if (received == '\n' || received == '\r') {
            if (UART_RX_Index > 0) {
                UART_RX_Buffer[UART_RX_Index] = '\0';
                Command_Ready = 1;
            }
        } else if (UART_RX_Index < sizeof(UART_RX_Buffer) - 1) {
            UART_RX_Buffer[UART_RX_Index++] = received;
        }
    }
}

// SysTick Initialization
void SysTick_Init(void) {
    SysTick->LOAD = (F_CPU / 1000) - 1;  // 1ms interrupt
    SysTick->VAL = 0;
    SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_TICKINT_Msk;
    // Initially disabled
    SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;
}

// GPIO Initialization
void GPIO_Init(void) {
   
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN | RCC_AHB1ENR_GPIOBEN;
    
    
    GPIOB->MODER &= ~((3U << (LED1_PIN * 2)) | (3U << (LED2_PIN * 2)) | (3U << (LED3_PIN * 2)));
    GPIOB->MODER |= (1U << (LED1_PIN * 2)) | (1U << (LED2_PIN * 2)) | (1U << (LED3_PIN * 2));
    
    
    GPIOB->OTYPER &= ~((1U << LED1_PIN) | (1U << LED2_PIN) | (1U << LED3_PIN));
    GPIOB->OSPEEDR &= ~((3U << (LED1_PIN * 2)) | (3U << (LED2_PIN * 2)) | (3U << (LED3_PIN * 2)));
    
  
    GPIOB->ODR &= ~((1U << LED1_PIN) | (1U << LED2_PIN) | (1U << LED3_PIN));
    
    GPIOA->MODER &= ~(3U << (TOUCH_PIN * 2));
    GPIOA->PUPDR &= ~(3U << (TOUCH_PIN * 2));  // No pull-up/pull-down
}

// EXTI Initialization for PA6
void EXTI_Init(void) {
    RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;
    
    SYSCFG->EXTICR[1] &= ~(0xF << 8);
    SYSCFG->EXTICR[1] |= (0x0 << 8);  // PA6
    
    EXTI->RTSR |= (1U << 6);
    EXTI->FTSR &= ~(1U << 6);
    EXTI->IMR |= (1U << 6);
    
    NVIC_EnableIRQ(EXTI9_5_IRQn);
}

// USART1 Initialization
void USART1_Init(void) {
    RCC->APB2ENR |= RCC_APB2ENR_USART1EN;
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
    
    GPIOA->MODER &= ~((3U << 18) | (3U << 20));
    GPIOA->MODER |= (2U << 18) | (2U << 20);  // Alternate function mode
    
    GPIOA->AFR[1] &= ~((0xF << 4) | (0xF << 8));
    GPIOA->AFR[1] |= (7U << 4) | (7U << 8);
    
    USART1->BRR = 0x683;  
    
    USART1->CR1 = USART_CR1_UE | USART_CR1_TE | USART_CR1_RE | USART_CR1_RXNEIE;
    
    NVIC_EnableIRQ(USART1_IRQn);
}

// Send single character via USART1
void USART1_SendChar(char c) {
    while (!(USART1->SR & USART_SR_TXE));
    USART1->DR = c;
}

// Send string via USART1
void USART1_SendString(const char* str) {
    while (*str) {
        USART1_SendChar(*str++);
    }
}

// LED Sequence: Turn on LEDs one by one, then turn all off
void LED_Sequence(void) {
    GPIOB->ODR |= (1U << LED1_PIN);
    delay_ms(1000);
    
    GPIOB->ODR |= (1U << LED2_PIN);
    delay_ms(1000);
    
    GPIOB->ODR |= (1U << LED3_PIN);
    delay_ms(1500);
    
    GPIOB->ODR &= ~((1U << LED1_PIN) | (1U << LED2_PIN) | (1U << LED3_PIN));
}

// Reset game variables for next turn
void Reset_Game(void) {
    Game_Started = 0;
    Reaction_Measured = 0;
    SysTick_count = 0;
    Reaction_Time_Ticks = 0;
    UART_RX_Index = 0;
    Command_Ready = 0;
    
    GPIOB->ODR &= ~((1U << LED1_PIN) | (1U << LED2_PIN) | (1U << LED3_PIN));
    
    SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;
}

// Reset entire game session
void Reset_Full_Game(void) {
    Reset_Game();
    Game_Active = 0;
    Current_Round = 0;
    Current_Player = 1;
    Player1_Turn_Count = 0;
    Player2_Turn_Count = 0;
    
    for (uint8_t i = 0; i < 10; i++) {
        Player1_Times[i] = 0;
        Player2_Times[i] = 0;
    }
}

// Calculate average reaction time
uint32_t Calculate_Average(uint32_t times[], uint8_t count) {
    if (count == 0) return 0;
    
    uint32_t sum = 0;
    for (uint8_t i = 0; i < count; i++) {
        sum += times[i];
    }
    return sum / count;
}

// Declare the winner
void Declare_Winner(void) {
    char buffer[100];
    uint32_t player1_avg = Calculate_Average(Player1_Times, Player1_Turn_Count);
    uint32_t player2_avg = Calculate_Average(Player2_Times, Player2_Turn_Count);
    
    USART1_SendString("\r\n--------- GAME OVER ----------\r\n");
    
    // Display Player 1 results
    sprintf(buffer, "Player 1 Average: %lu ms\r\n", (unsigned long)player1_avg);
    USART1_SendString(buffer);
    
    // Display individual times
    USART1_SendString("  Times: ");
    for (uint8_t i = 0; i < Player1_Turn_Count; i++) {
        sprintf(buffer, "%lu ms", (unsigned long)Player1_Times[i]);
        USART1_SendString(buffer);
        if (i < Player1_Turn_Count - 1) {
            USART1_SendString(", ");
        }
    }
    USART1_SendString("\r\n\r\n");
    
    // Display Player 2 results
    sprintf(buffer, "Player 2 Average: %lu ms\r\n", (unsigned long)player2_avg);
    USART1_SendString(buffer);
    
    // Display individual times
    USART1_SendString("  Times: ");
    for (uint8_t i = 0; i < Player2_Turn_Count; i++) {
        sprintf(buffer, "%lu ms", (unsigned long)Player2_Times[i]);
        USART1_SendString(buffer);
        if (i < Player2_Turn_Count - 1) {
            USART1_SendString(", ");
        }
    }
    USART1_SendString("\r\n\r\n");
    
    // Declare winner
    if (player1_avg < player2_avg) {
        sprintf(buffer, "*** WINNER: Player 1 *** (%lu ms)\r\n", (unsigned long)player1_avg);
        USART1_SendString(buffer);
    } else if (player2_avg < player1_avg) {
        sprintf(buffer, "*** WINNER: Player 2 *** (%lu ms)\r\n", (unsigned long)player2_avg);
        USART1_SendString(buffer);
    } else {
        USART1_SendString("*** IT'S A TIE! ***\r\n");
    }
    
    USART1_SendString("-----------------------------\r\n");
    USART1_SendString("\r\nSend 'start' to play again\r\n");
}

// Check if received command is "start" or "START"
uint8_t Is_Start_Command(void) {
    if (strcmp((const char*)UART_RX_Buffer, "start") == 0 || 
        strcmp((const char*)UART_RX_Buffer, "START") == 0) {
        return 1;
    }
    return 0;
}

// Main Function
int main(void) {
    char buffer[100];
    
    // Initialize all peripherals
    GPIO_Init();
    SysTick_Init();
    EXTI_Init();
    USART1_Init();
    
    // Send welcome message
    USART1_SendString("\r\n=== Reaction Time Game ===\r\n");
    sprintf(buffer, "Each player plays %d round(s)\r\n", ROUNDS_PER_GAME);
    USART1_SendString(buffer);
    USART1_SendString("Winner determined by average time\r\n");
    USART1_SendString("Send 'start' or 'START' to begin\r\n");
    
    while (1) {
        if (Command_Ready) {
            if (Is_Start_Command()) {
                
                if (!Game_Active) {
                    Reset_Full_Game();
                    Game_Active = 1;
                    USART1_SendString("\r\n*** NEW GAME STARTED ***\r\n");
                    sprintf(buffer, "Playing %d round(s) per player\r\n\r\n", ROUNDS_PER_GAME);
                    USART1_SendString(buffer);
                }
                
                uint8_t player_round_num;
                if (Current_Player == 1) {
                    player_round_num = Player1_Turn_Count + 1;
                } else {
                    player_round_num = Player2_Turn_Count + 1;
                }
                
                sprintf(buffer, "Player %d - Round %d/%d\r\n", 
                        Current_Player, player_round_num, ROUNDS_PER_GAME);
                USART1_SendString(buffer);
                USART1_SendString("Get ready...\r\n");
                
                delay_ms(1000);
                
                LED_Sequence();
                
                USART1_SendString("GO! Touch now!\r\n");
                
                SysTick_count = 0;
                Game_Started = 1;
                SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;
                
                while (!Reaction_Measured) {
                }
                
                if (Current_Player == 1) {
                    Player1_Times[Player1_Turn_Count] = Reaction_Time_Ticks;
                    Player1_Turn_Count++;
                } else {
                    Player2_Times[Player2_Turn_Count] = Reaction_Time_Ticks;
                    Player2_Turn_Count++;
                }
                
                sprintf(buffer, "Player %d Time: %lu ms\r\n\r\n", 
                        Current_Player, (unsigned long)Reaction_Time_Ticks);
                USART1_SendString(buffer);
                
                if (Player1_Turn_Count >= ROUNDS_PER_GAME && 
                    Player2_Turn_Count >= ROUNDS_PER_GAME) {
                    
                    delay_ms(1000);
                    Declare_Winner();
                    Reset_Full_Game();
                    
                } else {
                    Current_Player = (Current_Player == 1) ? 2 : 1;
                    
                    delay_ms(1000);
                    USART1_SendString("Send 'start' for next turn\r\n");
                }
                
                Reset_Game();
                
            } else {
                USART1_SendString("Invalid command. Send 'start' or 'START'\r\n");
                UART_RX_Index = 0;
                Command_Ready = 0;
            }
        }
    }
}






