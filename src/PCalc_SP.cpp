#include "PCalc_SP.h"
#include <math.h>

PCalc_SP::PCalc_SP(unsigned int n_array) : PCalc(n_array) {
}

void PCalc_SP::markNonPrimes() {
    at(0) = false;
    at(1) = false;
    for (int p=2; p*p<=array_size(); p++)
    {
        if (at(p) == true)
        {
            for (int i=p*p; i<=array_size(); i += p) {
                at(i) = false;
            }
        }
    }
}