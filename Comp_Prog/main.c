#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include "CommonData.h"
    

/*
* To use the program it's needed to type the next parameters:
* <filename> [test [t][u][s][i][a][r]] 
* t - Timer
* u - UART/USART
* s - SPI
* i - I2C
* a - ADC
* or to type h to see the help message.
*/

int main(int argc, char *argv[])
 {
    //ansi clear screen
    printf("\033[2J\033[H");
    
    //code
    printf("printing arguments...\r\n");
    int i;
    for(i = 0; i < argc; i++)
     {
      printf("%3d: %s\r\n", i, argv[i]);
     }

    
    return 0;
 }