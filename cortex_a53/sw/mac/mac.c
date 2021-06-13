#include "mac.h"
#include <stdio.h>

#define DONE (* (volatile unsigned long *) 0xFF000000)
#define TIME (* (volatile unsigned long *) 0x90000000)

void console_out(char *s, ...);

#define printf console_out

int mac(int factor1, int factor2, int term1) {
   volatile int r;
   FACTOR1 = factor1;
   FACTOR2 = factor2;
   TERM1 = term1;
   r = RETURN;
   return RETURN;
}

int sw_mac(int factor1, int factor2, int term1) {
   return (factor1 * factor2) + term1;
}


int main()
{
   int f1;
   int f2;
   int t1;
   int factor1;
   int factor2;
   int term1;

   int hw_result;
   int sw_result;

   int errors = 0;

   for (f1=0; f1<5; f1++) {
      for (f2=0; f2<5; f2++) {
         for (t1=0; t1<5; t1++) {

            factor1 = f1 * 3;
            factor2 = f2 * 5;
            term1   = t1 * 7;

            hw_result = mac(factor1, factor2, term1);
            sw_result = sw_mac(factor1, factor2, term1);

            if (sw_result != hw_result) {
               printf("factor1 = %d factor2 = %d term1 = %d \n", 
                          factor1, factor2, term1);
               printf("expected = %d got = %d \n",
                          sw_result, hw_result);
               errors++;
            }
         }
      }
   }

   printf("Errors: %d \n", errors);
   DONE = 1;
   return 0;
}

