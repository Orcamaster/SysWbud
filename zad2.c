//Zadanie 2

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

#include <xc.h>
#include <stdint.h>
#include <stdbool.h>
#include "libpic30.h"
#include "adc.h"
#include "buttons.h"

    // Zadanie 2 czyli 2 programy

#define BUTTON_NEXT BUTTON_S3 // RB3
#define BUTTON_PREV BUTTON_S4 // RB4
#define DEBOUNCE_TIME 100000

unsigned long DELAY_TIME = 500000;
unsigned char currentProgram = 1;
bool prevNextState = false;
bool prevPrevState = false;
bool buttonPressProcessed = false;

void init(void) {
    // ADC
    ADC_SetConfiguration(ADC_CONFIGURATION_DEFAULT);
    ADC_ChannelEnable(ADC_CHANNEL_5); // AN5 - potencjometr

    // PORTY
    TRISA = 0x0000;
    LATA = 0x0000;

    // Przyciski
    BUTTON_Enable(BUTTON_NEXT); // RB3
    BUTTON_Enable(BUTTON_PREV); // RB4
}

void update_speed_from_pot() {
    uint16_t pot_value = ADC_Read10bit(ADC_CHANNEL_5);

    if (pot_value < 205)
        DELAY_TIME = 800000;
    else if (pot_value < 410)
        DELAY_TIME = 600000;
    else if (pot_value < 615)
        DELAY_TIME = 400000;
    else if (pot_value < 820)
        DELAY_TIME = 200000;
    else
        DELAY_TIME = 100000;
}

// Program 1: Wezyk
void wezyk(void) {
    static unsigned char wezyk_pattern = 0x07;
    static unsigned char kierunek = 1;

    LATA = wezyk_pattern;

    if (kierunek)
        wezyk_pattern <<= 1;
    else
        wezyk_pattern >>= 1;

    if (wezyk_pattern == 0xE0) kierunek = 0;
    if (wezyk_pattern == 0x07) kierunek = 1;

    __delay32(DELAY_TIME);
}



void queue(void) {
    static int phase = 0;         // Numer fazy 
    static int position = 0;      
    static int locked = 0x00;     
    int output;

    // Zapalanie kolejnych diod w "kolejce"
    if (position < (8 - phase)) {
        output = locked | (1 << position); // Dodaj nowa przesuwana diode
        position++;
    } else {

        locked |= (1 << (7 - phase));
        phase++;
        position = 0;
        output = locked;

        // Reset po zapelnieniu wszystkich diod
        if (phase == 8) {
            phase = 0;
            locked = 0x00;
        }
    }

    LATA = output;
    __delay32(DELAY_TIME);
}

// Obsluga przycisków
void checkButtons(void) {
    bool currNextState = BUTTON_IsPressed(BUTTON_NEXT);
    bool currPrevState = BUTTON_IsPressed(BUTTON_PREV);

    if ((currNextState || currPrevState) && !buttonPressProcessed) {
        buttonPressProcessed = true;

        if (currNextState) {
            currentProgram = (currentProgram == 2) ? 1 : 2;
        } else if (currPrevState) {
            currentProgram = (currentProgram == 1) ? 2 : 1;
        }

        LATA = 0x00; // reset ledów
        __delay32(DEBOUNCE_TIME);
    } else if (!currNextState && !currPrevState) {
        buttonPressProcessed = false;
    }

    prevNextState = currNextState;
    prevPrevState = currPrevState;
}

int main(void) {
    init();

    while (1) {
        checkButtons();
        update_speed_from_pot();

        if (currentProgram == 1) {
            wezyk();
        } else {
            queue();
        }

        ClrWdt();
    }

    return 0;
}
