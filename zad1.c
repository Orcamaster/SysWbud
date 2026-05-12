#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


// Funkcja pomocnicza do symulowania stanów diod
void decToBinary(int n) {
    int binaryNum[8] = {0};
    int i = 0;
 
    while (n > 0 && i < 8) {
        binaryNum[i] = n % 2;
        n = n / 2;
        i++;
    }
 
    for (int j = 7; j >= 0; j--){
        printf("%d", binaryNum[j]);
    }
 
    printf("\n");
}


void licznik_binarny_gora() {
    int portValue;
    
    for (int i = 0; i <= 255; i++) {
        portValue = i;
        decToBinary(portValue);
        sleep(1);
    }
}


void licznik_binarny_dol() {
    int portValue;
    
    for (int i = 255; i >= 0; i--) {
        portValue = i;
        decToBinary(portValue);
        sleep(1);
    }
}


void licznik_graya_gora() {
    int portValue;

    for (int i = 0; i <= 255; i++) {
        portValue = i ^ (i >> 1);
        decToBinary(portValue);
        sleep(1);
    }
}


void licznik_graya_dol() {
    int portValue;

    for (int i = 255; i >= 0; i--) {
        portValue = i ^ (i >> 1);
        decToBinary(portValue);
        sleep(1);
    }
}


void licznik_BCD_gora() {
    int portValue;
    int dziesiatki, jednosci;

    for (int i = 0; i <= 99; i++) {

        dziesiatki = i / 10;
        jednosci = i % 10;

        portValue = (dziesiatki << 4) | jednosci;

        decToBinary(portValue);
        sleep(1);
    }
}


void licznik_BCD_dol() {
    int portValue;
    int dziesiatki, jednosci;

    for (int i = 99; i >= 0; i--) {

        dziesiatki = i / 10;
        jednosci = i % 10;

        portValue = (dziesiatki << 4) | jednosci;

        decToBinary(portValue);
        sleep(1);
    }
}


void wezyk() {
    int portValue = 0;
 
    while (1) {
        for (int i = 7; i <= 224; i *= 2) {
            portValue = i;
            decToBinary(portValue);
            sleep(1);
        }
        
        for (int i = 112; i >= 14; i /= 2) {
            portValue = i;
            decToBinary(portValue);
            sleep(1);
        }
    }
}


void kolejka() {
    int portValue = 0;
    int j = 128;
    int x = 0;
 
    // osobna iteracja bez dodawania bitu na początku kolejki
    for (int i = 1; i < j; i *= 2) {
        portValue = i;
        decToBinary(portValue);
    }
 
    // pozostałe iteracje
    while (j > 0) {
        x = x + j;
        for (int i = 1; i < j; i *= 2) {  
            portValue = x + i;
            decToBinary(portValue);
        }
        j /= 2;
    }
}


void generator_pseudolosowy() {

    unsigned char lfsr = 0x3F; // stan początkowy 111111
    unsigned char bit;
    int portValue;

    while (1) {

        portValue = lfsr;
        decToBinary(portValue);

        // sprzężenie zwrotne
        bit = ((lfsr >> 5) ^
               (lfsr >> 4) ^
               (lfsr >> 3) ^
               (lfsr >> 0)) & 1;

        // przesunięcie i wpisanie nowego bitu
        lfsr = (lfsr >> 1) | (bit << 5);

        sleep(1);
    }
}


int main() {
    return 0;
}
