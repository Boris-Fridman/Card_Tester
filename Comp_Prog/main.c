#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <sqlite3.h>
#include "CommonData.h"
    


void PrinthelpMessage();


int main(int argc, char *argv[])
 {
    //ansi clear screen
    printf("\033[2J\033[H");
    
    //code
    printf("printing arguments...\n\r");
    int i;
    for(i = 0; i < argc; i++)
     {
      printf("%3d: %s\r\n", i, argv[i]);
     }

    
    return 0;
 }

int CheckArgs(int argc, char *argv[], int *NInt,PeriphBitField_s *BFResult)
 {
  int i;
  size_t j;
  char *st, ch;
  int nintres;
  bool nintloaded = false;
  
  for(i = 0; i < argc; i++)
   {
    st = argv[i];
    for(j = 0; j < strlen(st); j++)
     {
      ch = st[j];
      switch(ch)
       {
         case 't':
           BFResult->Timer = 1;
          break;
         case 'u':
           BFResult->UART = 1;
          break;
         case 's':
           BFResult->SPI = 1;
          break;
         case 'i':
           BFResult->I2c = 1;
          break;
         case 'a':
           BFResult->ADC = 1;
          break;
         case 'n':
           nintres = sscanf(&st[j], "%d", NInt);
           nintloaded |= (nintres != EOF);
          break;
         case 'h':         
          break;
       }
     }
   }
  if(!nintloaded)
   {
    if(nintres ==  EOF)
     fprintf(stderr, "The parameter nxx is incorrect.");
    else
     fprintf(stderr, "Was not loaded.");
    fprintf(stderr, "Using default number inteructions that is \"1\"\n\r");
    *NInt = 1;
   }            

  return 0;
 }

void PrinthelpMessage()
 {
  printf("To use the program it's needed to type the next parameters:\n\r");
  printf("<filename> [test [t][u][s][i][a][r]] [n] \n\r");
  printf("t  - Timer\n\r");
  printf("u  - UART/USART\n\r");
  printf("s  - SPI\n\r");
  printf("i  - I2C\n\r");
  printf("a  - ADC\n\r");
  printf("nx - Number of interactions. Where x is number. Default is 1.");
  printf("Or to type h to see the help message.\n\r");
 }

