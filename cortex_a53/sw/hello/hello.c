#include <stdio.h>

#define printf console_out

void console_out(char *s, ...);

int main() 
{
   int i;
   int response[16];

   for (i=0; i<16; i++) {
      * (((unsigned long *) 0xA0000000) + i) = i;
   }

   for (i=0; i<16; i++) {
      response[i] = * (((unsigned long *) 0xA0000000) + i);
   }

   for (i=0; i<16; i++) printf("%d: %d \n", i, response[i]);

   printf("Hello, World! \n");
}

