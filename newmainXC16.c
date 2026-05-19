// PIC24FJ128GA010 Configuration Bit Settings

// 'C' source line config statements

// CONFIG2
#pragma config POSCMOD = XT             // Primary Oscillator Select (XT Oscillator mode selected)
#pragma config OSCIOFNC = ON            // Primary Oscillator Output Function (OSC2/CLKO/RC15 functions as port I/O (RC15))
#pragma config FCKSM = CSDCMD           // Clock Switching and Monitor (Clock switching and Fail-Safe Clock Monitor are disabled)
#pragma config FNOSC = PRI              // Oscillator Select (Primary Oscillator (XT, HS, EC))
#pragma config IESO = ON                // Internal External Switch Over Mode (IESO mode (Two-Speed Start-up) enabled)

// CONFIG1
#pragma config WDTPS = PS32768          // Watchdog Timer Postscaler (1:32,768)
#pragma config FWPSA = PR128            // WDT Prescaler (Prescaler ratio of 1:128)
#pragma config WINDIS = ON              // Watchdog Timer Window (Standard Watchdog Timer enabled,(Windowed-mode is disabled))
#pragma config FWDTEN = ON              // Watchdog Timer Enable (Watchdog Timer is enabled)
#pragma config ICS = PGx2               // Comm Channel Select (Emulator/debugger uses EMUC2/EMUD2)
#pragma config GWRP = OFF               // General Code Segment Write Protect (Writes to program memory are allowed)
#pragma config GCP = OFF                // General Code Segment Code Protect (Code protection is disabled)
#pragma config JTAGEN = OFF             // JTAG Port Enable (JTAG port is disabled)

// #pragma config statements should precede project file includes.
// Use project enums instead of #define for ON and OFF.

// Zadanie 5 czyli Zegar

#include <xc.h>
#include <stdint.h>
#include <stdbool.h>
#include "libpic30.h"
#include "buttons.h"
#include "adc.h"

#define BUTTON_PLAYER1 BUTTON_S3    // RB3
#define BUTTON_PLAYER2 BUTTON_S5    // RB5

// Time settings in seconds
#define TIME_5MIN 300
#define TIME_3MIN 180
#define TIME_1MIN 60

// ADC thresholds for time selection
#define THRESHOLD_5MIN 341  // 0-341: 5min
#define THRESHOLD_3MIN 682  // 342-682: 3min, 683-1023: 1min

typedef struct {
    uint16_t remainingTime;  // in seconds
    bool isActive;
} Player;

Player player1 = {0, false};
Player player2 = {0, false};
bool gameActive = false;

uint16_t wartoscPotencjometru(void) {
    return ADC_Read10bit(ADC_CHANNEL_POTENTIOMETER);
}

uint16_t getTimeSetting(void) {
    uint16_t adcValue = wartoscPotencjometru();
    if (adcValue <= THRESHOLD_5MIN) return TIME_5MIN;
    if (adcValue <= THRESHOLD_3MIN) return TIME_3MIN;
    return TIME_1MIN;
}

void init(void) {
    TRISA = 0x0000;    
    LATA = 0x0000;     
    BUTTON_Enable(BUTTON_PLAYER1);
    BUTTON_Enable(BUTTON_PLAYER2);
    
    // Konfiguracja ADC
    ADC_SetConfiguration(ADC_CONFIGURATION_DEFAULT);
    ADC_ChannelEnable(ADC_CHANNEL_POTENTIOMETER);
    
    // Initialize game
    uint16_t initialTime = getTimeSetting();
    player1.remainingTime = initialTime;
    player2.remainingTime = initialTime;
    player1.isActive = true;
    player2.isActive = false;
    gameActive = true;
}

void updateDisplay(void) {
    // Display player 1 time on lower 4 LEDs
    LATA = (player1.remainingTime & 0x0F) | ((player2.remainingTime & 0x0F) << 4);
}

void checkTimeout(void) {
    if (player1.remainingTime == 0) {
        // Player 1 timeout
        LATA = 0x0F;  // All lower LEDs on
        gameActive = false;
    }
    else if (player2.remainingTime == 0) {
        // Player 2 timeout
        LATA = 0xF0;  // All upper LEDs on
        gameActive = false;
    }
}

void handleButtons(void) {
    if (!gameActive) return;
    
    if (BUTTON_IsPressed(BUTTON_PLAYER1)) {
        player1.isActive = false;
        player2.isActive = true;
    }
    else if (BUTTON_IsPressed(BUTTON_PLAYER2)) {
        player1.isActive = true;
        player2.isActive = false;
    }
}

void updateTime(void) {
    if (!gameActive) return;
    
    static uint32_t timeCounter = 0;
    timeCounter++;
    
    // Update time every second (assuming 1MHz clock)
    if (timeCounter >= 1000000) {
        timeCounter = 0;
        
        if (player1.isActive && player1.remainingTime > 0) {
            player1.remainingTime--;
        }
        else if (player2.isActive && player2.remainingTime > 0) {
            player2.remainingTime--;
        }
        
        checkTimeout();
    }
}

int main(void) {
    init();
    while (1) {
        handleButtons();
        updateTime();
        updateDisplay();
        ClrWdt();
    }
    return 0;
}
