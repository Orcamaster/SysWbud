
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

// Zadanie 3 czyli Alarm

#include <xc.h>
#include <stdint.h>
#include <stdbool.h>
#include "libpic30.h"
#include "buttons.h"
#include "adc.h"

#define BUTTON_OFF BUTTON_S3 // RB3
#define LED_ALL 0xFF
#define LED_ONE 0x01
// polowa zakresu 10 bitowego
#define ALARM_THRESHOLD 512

#define BLINK_DELAY 250000 

#define BLINKS 50

uint16_t wartoscPotencjometru(void) {
    return ADC_Read10bit(ADC_CHANNEL_POTENTIOMETER);
}

void init(void) {
    TRISA = 0x0000;    
    LATA = 0x0000;     
    BUTTON_Enable(BUTTON_OFF);  

    ADC_SetConfiguration(ADC_CONFIGURATION_DEFAULT);
    ADC_ChannelEnable(ADC_CHANNEL_POTENTIOMETER);  
}

void alarm(void) {
    static bool alarmOn = false;
    static bool blinking = false;
    static int blink_count = 0;
    static bool ledOn = false;

    int adcValue = wartoscPotencjometru();
    bool buttonOff = BUTTON_IsPressed(BUTTON_OFF);

    if (!alarmOn) {
        if (adcValue > ALARM_THRESHOLD) {
            alarmOn = true;
            blinking = true;
            blink_count = 0;
            ledOn = false;
        } else {
            LATA = 0x00; 
        }
    } else {
        if (blinking) {
            if (blink_count < BLINKS) {
                LATA = ledOn ? LED_ONE : 0x00;
                ledOn = !ledOn;
                blink_count++;
                __delay32(BLINK_DELAY);

                if (adcValue <= ALARM_THRESHOLD || buttonOff) {
                    alarmOn = false;
                    blinking = false;
                    LATA = 0x00;
                }
            } else {

                blinking = false;
                LATA = LED_ALL;
            }
        } else {

            LATA = LED_ALL;

            if (adcValue <= ALARM_THRESHOLD || buttonOff) {
                alarmOn = false;
                LATA = 0x00;
            }
        }
    }
}

int main(void) {
    init();
    while (1) {
        alarm();
        ClrWdt();  
    }
    return 0;
}
