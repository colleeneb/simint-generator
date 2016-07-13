#include "simint/simint_init.h"
#include "simint/eri/eri_init.h"

void simint_init(void)
{
    simint_eri_init();
}


void simint_finalize(void)
{
    simint_eri_finalize();
}

