#include <iostream>
#include <omp.h>

#include <cstdlib>

#define NUM_THREADS 8
#define ITER_COUNT 1000000

using namespace std;

int main()
{
    double tstart = 0.0;
    double tend = 0.0;
    double ttaken = 0.0;

    tstart = omp_get_wtime(); //getting the start time

    int d = 0;
    for(int i = 0; i<ITER_COUNT; ++i)
    {
        int a = 1;
        int b = 2;
        int c = a + b;
        d = a*c + b;
        d += i;
    }

    // cout << "Output: " << d << "\n";

    tend = omp_get_wtime(); // getting the end time
    ttaken = tend - tstart; // calculating the time taken

    cout << "Time taken for the main part: " << ttaken << "\n";

    return 0;

}