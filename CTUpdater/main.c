#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
    
int main(void)
 {
  bool NoPiping;
  NoPiping = isatty(STDOUT_FILENO); /* Checking if the output is not redirected to any other program or file to decide if to use colors or not. */

  //ansi clear screen
  if(NoPiping)
   printf("\033[2J\033[H");
  else
   printf("\n\r");
  
  //code
  
  return 0;
 }