#include <ac_int.h>

#pragma hls_design top
uint26 mac(uint12 factor1, uint13 factor2, uint25 term1) {
   return (factor1 * factor2) + term1;
}

