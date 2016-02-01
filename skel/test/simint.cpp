#include "test/simint.hpp"
#include "test/common.hpp"
#include "eri/eri.h"
#include "simint_init.h"

void Simint_Init(void)
{
    simint_init();
}

TimerType Simint_Integral(struct multishell_pair const P, struct multishell_pair const Q,
                          double * const restrict contwork,
                          double * const restrict integrals)
{
    TimerType ticks0, ticks1;

    CLOCK(ticks0);
    simint_compute_eri_sharedwork[P.am1][P.am2][Q.am1][Q.am2](P, Q, contwork, integrals);
    CLOCK(ticks1);
    return ticks1 - ticks0;
}

