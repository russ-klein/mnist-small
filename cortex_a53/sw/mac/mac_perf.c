
#include "mac.h"
#include <stdio.h>

#define DONE (* (unsigned long *) 0xFF000000)

#define TIMER (* (volatile unsigned long *) 0x90000000)

#define printf console_out
void console_out(char *s, ...);

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

int run_perf_test(int iterations)
{
   int f1, f2, t1;
   unsigned long mark;
   unsigned int start, end;
   int running_total = 0;

   printf("MAC performance test \n");
   printf("Starting SW run... \n");
   
   start = TIMER;

   for (f1=0; f1<iterations; f1++) {
      for (f2=0; f2<iterations; f2++) {
         for (t1=0; t1<iterations; t1++) {
            running_total += sw_mac(f1, f2, t1);
         }
      }
   }

   end = TIMER;

   printf("SW elapsed time: %d clocks \n", end-start);
   printf("Starting HW run... \n");

   start = TIMER;

   for (f1=0; f1<iterations; f1++) {
      for (f2=0; f2<iterations; f2++) {
         for (t1=0; t1<iterations; t1++) {
            running_total += mac(f1, f2, t1);
         }
      }
   }

   end = TIMER;

   printf("HW elapsed time: %d clocks \n", end-start);
   return running_total;
}
   
int main()
{
   int running_total;
   int one = 1;

   running_total = run_perf_test(7);

   printf("running_total: %d this is one: %d \n", running_total, one);
   DONE = 1;
   return running_total;
}
