#include <iostream>
#include <omp.h>

// #include <cstdlib>
#include "custom_mem_alloc.h"

#define NUM_THREADS 8

using namespace std;

int foo(int num)
{
    long long ans = 0.0;
    for(int i = 0; i<1000000; ++i)
    {
        ans = ans + (num*i);
    }

    int ret = (int)(ans);
    return ret;
}

int main()
{
    double tstart = 0.0;
    double tend = 0.0;
    double ttaken = 0.0;

    omp_set_num_threads(NUM_THREADS);

    tstart = omp_get_wtime(); //getting the start time

    #pragma omp parallel
    {
        int thread_no = omp_get_thread_num();

        int *arr = (int*)my_mem_alloc(100*sizeof(int));

        int a = 1;
        int b = 2;
        int c = a + b;
        int d = a*c + b;

        int thread_sum = 0;

        for(int i = 0; i<100; ++i)
        {
            arr[i] = foo((c+d)*i);
            thread_sum += arr[i];
        }

        // cout << "Thread " << thread_no << " Output: " << thread_sum << "\n";

        my_mem_free(arr);
    }

    tend = omp_get_wtime(); // getting the end time
    ttaken = tend - tstart; // calculating the time taken

    cout << "Time taken for the main part: " << ttaken << "\n";

    return 0;

}