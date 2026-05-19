// PIC24FJ128GA010 Configuration Bit Settings

// CONFIG2
#pragma config POSCMOD = XT             // XT Oscillator selected
#pragma config OSCIOFNC = ON            // OSC2 pin functions as digital I/O
#pragma config FCKSM = CSDCMD           // Clock switching disabled; fail-safe clock monitor disabled
#pragma config FNOSC = PRI              // Primary oscillator selected
#pragma config IESO = ON                // Two-speed start-up enabled

// CONFIG1
#pragma config WDTPS = PS32768          // Watchdog postscaler: 1:32,768
#pragma config FWPSA = PR128            // WDT prescaler: 1:128
#pragma config WINDIS = ON              // Standard WDT mode
#pragma config FWDTEN = ON              // WDT enabled
#pragma config ICS = PGx2               // Debugging on EMUC2/EMUD2
#pragma config GWRP = OFF               // Program memory write protection disabled
#pragma config GCP = OFF                // Code protection disabled
#pragma config JTAGEN = OFF             // JTAG disabled

#include <xc.h>
#include <stdbool.h>
#include <stdio.h>
#include "lcd.h"
#include "buttons.h"
#include "libpic30.h"
#include "adc.h"

typedef enum {
    GAME_SETUP,
    PLAYER1_TURN,
    PLAYER2_TURN,
    GAME_OVER
} GameState;

typedef struct {
    bool wasPressed;
    bool isPressed;
} ButtonState;

// zmienne globalne
GameState currentState = GAME_SETUP;
int timerPlayer1 = 0;
int timerPlayer2 = 0;
char lcdBuffer[17];
bool displayNeedsUpdate = true;
int lastADCValue = 0;

// odczyt z potencjometru
bool adjustGameDurationFromADC(void) {
    int adcValue = ADC_Read10bit(ADC_CHANNEL_POTENTIOMETER);
    bool thresholdCrossed = false;

    if ((lastADCValue < 341 && adcValue >= 341) ||
        (lastADCValue >= 341 && adcValue < 341) ||
        (lastADCValue < 682 && adcValue >= 682) ||
        (lastADCValue >= 682 && adcValue < 682)) {
        thresholdCrossed = true;
    }

    if (adcValue < 341) {
        timerPlayer1 = timerPlayer2 = 60; // 1 minuta
    } else if (adcValue < 682) {
        timerPlayer1 = timerPlayer2 = 180; // 3 minuty
    } else {
        timerPlayer1 = timerPlayer2 = 300; // 5 minut
    }

    lastADCValue = adcValue;
    return thresholdCrossed;
}


void showSetupScreen(void) {
    LCD_ClearScreen();
    LCD_PutString("SELECT TIME:", 12);
    LCD_PutChar('\n');

    int adcVal = ADC_Read10bit(ADC_CHANNEL_POTENTIOMETER);

    if (adcVal < 341) {
        LCD_PutString("1 MINUTE", 8);
    } else if (adcVal < 682) {
        LCD_PutString("3 MINUTES", 9);
    } else {
        LCD_PutString("5 MINUTES", 9);
    }
}

void renderGameScreen(void) {
    int min1 = timerPlayer1 / 60;
    int sec1 = timerPlayer1 % 60;
    int min2 = timerPlayer2 / 60;
    int sec2 = timerPlayer2 % 60;

    LCD_ClearScreen();
    sprintf(lcdBuffer, "P1: %02d:%02d", min1, sec1);
    LCD_PutString(lcdBuffer, 16);
    LCD_PutChar('\n');
    sprintf(lcdBuffer, "P2: %02d:%02d", min2, sec2);
    LCD_PutString(lcdBuffer, 16);
}

void displayEndScreen(int losingPlayer) {
    LCD_ClearScreen();
    if (losingPlayer == 1) {
        LCD_PutString("PLAYER 1 LOST", 14);
    } else {
        LCD_PutString("PLAYER 2 LOST", 14);
    }
    LCD_PutChar('\n');
    LCD_PutString("TIME EXPIRED", 13);
}

void initializeSystem(void) {
    LCD_Initialize();
    BUTTON_Enable(BUTTON_S3);
    BUTTON_Enable(BUTTON_S5);

    ADC_SetConfiguration(ADC_CONFIGURATION_DEFAULT);
    ADC_ChannelEnable(ADC_CHANNEL_POTENTIOMETER);

    currentState = GAME_SETUP;
    adjustGameDurationFromADC();
    showSetupScreen();
}

int main(void) {
    initializeSystem();

    ButtonState buttons[2] = {
        {false, false}, // przycisk_S3
        {false, false}  // przycisk_S5
    };

    while (1) {
        // Odczyt stanu
        buttons[0].isPressed = BUTTON_IsPressed(BUTTON_S3);
        buttons[1].isPressed = BUTTON_IsPressed(BUTTON_S5);

        // Zmiany w odczycie z potencjometru przed rozpoczeciem rozgrywki
        if (currentState == GAME_SETUP && adjustGameDurationFromADC()) {
            showSetupScreen();
        }

        if (displayNeedsUpdate) {
            switch (currentState) {
                case GAME_SETUP:
                    showSetupScreen();
                    break;
                case PLAYER1_TURN:
                case PLAYER2_TURN:
                    renderGameScreen();
                    break;
                case GAME_OVER:
                    // Do not refresh display here
                    break;
            }
            displayNeedsUpdate = false;
        }

        if (buttons[0].isPressed && !buttons[0].wasPressed) {
            if (currentState == GAME_SETUP) {
                currentState = PLAYER1_TURN;
            } else if (currentState == PLAYER1_TURN) {
                currentState = PLAYER2_TURN;
            }
            displayNeedsUpdate = true;
        }

        if (buttons[1].isPressed && !buttons[1].wasPressed) {
            if (currentState == PLAYER2_TURN) {
                currentState = PLAYER1_TURN;
                displayNeedsUpdate = true;
            }
        }

        for (int i = 0; i < 2; i++) {
            buttons[i].wasPressed = buttons[i].isPressed;
        }

        
        if (currentState == PLAYER1_TURN && timerPlayer1 > 0) {
            __delay32(3200000); // 1 sekunda dla zegaru pica
            ClrWdt();
            timerPlayer1--;
            displayNeedsUpdate = true;

            if (timerPlayer1 <= 0) {
                currentState = GAME_OVER;
                displayEndScreen(1);
            }
        } else if (currentState == PLAYER2_TURN && timerPlayer2 > 0) {
            __delay32(3200000);
            ClrWdt();
            timerPlayer2--;
            displayNeedsUpdate = true;

            if (timerPlayer2 <= 0) {
                currentState = GAME_OVER;
                displayEndScreen(2);
            }
        } else {
            __delay32(320000);
            ClrWdt();
        }
    }

    return 0;
}
