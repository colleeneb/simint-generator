#include <cstdio>

#include "simint/vectorization/vectorization.h"
#include "simint/eri/eri.h"
#include "test/Common.hpp"
#include "test/ERD.hpp"
#include "test/Timer.h"


#ifdef BENCHMARK_VALIDATE
#include "test/ValeevRef.hpp"
#endif


int main(int argc, char ** argv)
{
    if(argc != 2)
    {
        printf("Give me 1 argument! I got %d\n", argc-1);
        return 1;
    }

    // files to read
    std::string basfile(argv[1]);


    // read in the shell info
    ShellMap shellmap_erd = ReadBasis(basfile);

    // normalize
    #ifdef BENCHMARK_VALIDATE
    ShellMap shellmap = CopyShellMap(shellmap_erd);
    for(auto & it : shellmap)
        simint_normalize_shells(it.second.size(), it.second.data());
    #endif


    for(auto & it : shellmap_erd)
        simint_normalize_shells_erd(it.second.size(), it.second.data());



    // find the max dimensions
    std::array<int, 3> maxparams = FindMapMaxParams(shellmap_erd);
    const int maxam = (maxparams[0] > SIMINT_ERI_MAXAM ? SIMINT_ERI_MAXAM : maxparams[0]);
    const int maxnprim = maxparams[1];
    const int maxsize = maxparams[2];

    /* Storage of integrals */
    double * res_ints = (double *)ALLOC(maxsize * sizeof(double)); 


    // initialize stuff
    ERD_ERI eri(maxam, maxnprim, 1);


    #ifdef BENCHMARK_VALIDATE
    ValeevRef_Init();
    double * res_ref = (double *)ALLOC(maxsize * sizeof(double));
    #endif

    PrintTimingHeader();

    // running totals
    size_t ncont1234_total = 0;
    size_t nprim1234_total = 0;
    size_t nshell1234_total = 0;
    TimeContrib time_total;


    for(int i = 0; i <= maxam; i++)
    for(int j = 0; j <= maxam; j++)
    for(int k = 0; k <= maxam; k++)
    for(int l = 0; l <= maxam; l++)
    {
        if(!ValidQuartet({i,j,k,l}))
            continue;

        // total amount of time for this AM quartet
        TimeContrib time_am;

        const size_t nshell3 = shellmap_erd[k].size();
        const size_t nshell4 = shellmap_erd[l].size();

        simint_shell const * const C = &shellmap_erd[k][0];
        simint_shell const * const D = &shellmap_erd[l][0];


        #ifdef BENCHMARK_VALIDATE
        std::pair<double, double> err{0.0, 0.0};
        #endif

        // running totals for this am
        size_t nprim1234_am = 0;
        size_t nshell1234_am = 0;
        size_t ncont1234_am = 0;

        // do one shell pair at a time on the bra side
        for(size_t a = 0; a < shellmap_erd[i].size(); a++)
        for(size_t b = 0; b < shellmap_erd[j].size(); b++)
        {
            const size_t nshell1 = 1;
            const size_t nshell2 = 1;

            simint_shell const * const A = &shellmap_erd[i][a];
            simint_shell const * const B = &shellmap_erd[j][b];



            // actually calculate
            time_am += eri.Integrals(A, nshell1, B, nshell2,
                                     C, nshell3, D, nshell4, res_ints);

            // acutal number of primitives and shells calculated
            // TODO - replace with return values from Integrals
            size_t nprim1234 = 0;
            for(size_t p = 0; p < nshell1; p++)
            for(size_t q = 0; q < nshell2; q++)
            for(size_t r = 0; r < nshell3; r++)
            for(size_t s = 0; s < nshell4; s++)
                nprim1234 += A[p].nprim * B[q].nprim * C[r].nprim * D[s].nprim;

            const size_t nshell1234 = nshell1 * nshell2 * nshell3 * nshell4;
            const size_t ncart1234 = NCART(i) * NCART(j) * NCART(k) * NCART(l);
            const size_t ncont1234 = nshell1234 * ncart1234;

            #ifdef BENCHMARK_VALIDATE
            ValeevRef_Integrals(A, nshell1,
                                B, nshell2,
                                C, nshell3,
                                D, nshell4,
                                res_ref, false);
            std::pair<double, double> err2 = CalcError(res_ints, res_ref, ncont1234);

            err.first = std::max(err.first, err2.first);
            err.second = std::max(err.second, err2.second);
            #endif


            // add primitive and shell count to running totals for this am
            ncont1234_am += ncont1234;
            nprim1234_am += nprim1234;
            nshell1234_am += nshell1234;
        }

        // add primitive and shell count to overall running totals
        ncont1234_total += ncont1234_am;
        nprim1234_total += nprim1234_am;
        nshell1234_total += nshell1234_am;
        time_total += time_am;

        PrintAMTimingInfo(i, j, k, l, nshell1234_am, nprim1234_am, time_am);


        #ifdef BENCHMARK_VALIDATE
        printf("( %d %d | %d %d ) MaxAbsErr: %10.3e   MaxRelErr: %10.3e\n", i, j, k, l, err.first, err.second);
        #endif
    }

    printf("\n");
    printf("Calculated %ld contracted integrals\n", static_cast<long>(ncont1234_total));
    printf("Calculated %ld contracted shells\n", static_cast<long>(nshell1234_total));
    printf("Calculated %ld primitive integrals\n", static_cast<long>(nprim1234_total));
    printf("Total ticks to calculate all:  %llu\n", time_total.TotalTime());
    printf("\n");

    FreeShellMap(shellmap_erd);
    FREE(res_ints);


    #ifdef BENCHMARK_VALIDATE
    FreeShellMap(shellmap);
    FREE(res_ref);
    ValeevRef_Finalize();
    #endif
   
    return 0;
}
