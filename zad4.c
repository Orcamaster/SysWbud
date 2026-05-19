
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

// Zadanie 4 czyli mikrofalówka

#include <xc.h>
#include <stdint.h>
#include <stdbool.h>
#include "libpic30.h"
#include "buttons.h"
#include "lcd.h"

// Przyciski
#define BUTTON_POWER BUTTON_S6    // RB5 - Power selection
#define BUTTON_1MIN BUTTON_S5     // RB4 - Add 1 minute
#define BUTTON_10S BUTTON_S4      // RB3 - Add 10 seconds
#define BUTTON_START BUTTON_S3    // RB2 - Start/Stop

// Poziomy mocy
#define POWER_800W 0
#define POWER_600W 1
#define POWER_350W 2
#define POWER_200W 3

static uint8_t currentPower = POWER_800W;
static uint16_t remainingTime = 0;  // czas
static bool isRunning = false;

void formatTime(char* buffer, uint16_t seconds) {
    uint8_t minutes = seconds / 60;
    seconds = seconds % 60;
    sprintf(buffer, "%02d:%02d", minutes, seconds);
}

const char* getPowerString(uint8_t power) {
    switch(power) {
        case POWER_800W: return "800W";
        case POWER_600W: return "600W";
        case POWER_350W: return "350W";
        case POWER_200W: return "200W";
        default: return "ERR";
    }
}

void updateDisplay(void) {
    char timeStr[6];
    char displayStr[16];
    
    LCD_ClearScreen();
    

    formatTime(timeStr, remainingTime);
    

    sprintf(displayStr, "P:%s T:%s", getPowerString(currentPower), timeStr);
    LCD_PutString(displayStr, 16);

    LCD_PutChar('\n');

    if (remainingTime == 0 && !isRunning) {
        LCD_PutString("DONE!", 5);
    } else {
        LCD_PutString(isRunning ? "RUNNING" : "READY", 7);
    }
}

void handlePowerButton(void) {
    if (BUTTON_IsPressed(BUTTON_POWER)) {
        currentPower = (currentPower + 1) % 4;
        updateDisplay();
        while(BUTTON_IsPressed(BUTTON_POWER)); // Wait for button release
    }
}

void handleTimeButtons(void) {
    if (BUTTON_IsPressed(BUTTON_1MIN)) {
        remainingTime += 60;
        if (remainingTime > 9999) remainingTime = 9999;
        updateDisplay();
        while(BUTTON_IsPressed(BUTTON_1MIN));
    }
    
    if (BUTTON_IsPressed(BUTTON_10S)) {
        remainingTime += 10;
        if (remainingTime > 9999) remainingTime = 9999;
        updateDisplay();
        while(BUTTON_IsPressed(BUTTON_10S));
    }
}


void handleStartStop(void) {
    if (BUTTON_IsPressed(BUTTON_START)) {
        if (remainingTime > 0) {
            isRunning = !isRunning;
            updateDisplay();
        }
        while(BUTTON_IsPressed(BUTTON_START));
    }
}


void updateTimer(void) {
    if (isRunning && remainingTime > 0) {
        remainingTime--;
        if (remainingTime == 0) {
            isRunning = false;

            updateDisplay();
        }
        updateDisplay();
    }
}

void init(void) {

    LCD_Initialize();
    LCD_ClearScreen();
    

    BUTTON_Enable(BUTTON_POWER);
    BUTTON_Enable(BUTTON_1MIN);
    BUTTON_Enable(BUTTON_10S);
    BUTTON_Enable(BUTTON_START);

    updateDisplay();
}

int main(void) {
    init();
    
    while(1) {
        handlePowerButton();
        handleTimeButtons();
        handleStartStop();
        updateTimer();
        
        // opoznienie 1 sekundy dla pica
        __delay32(3200000);
        ClrWdt();
    }
    
    return 0;
} 