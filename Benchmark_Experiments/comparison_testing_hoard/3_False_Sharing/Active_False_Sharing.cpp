#include <iostream>
#include <omp.h>

// #include <cstdlib>

#define NUM_THREADS 8 
#define MAX_N 100000

using namespace std;

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

        int* arr = (int*)malloc(2*sizeof(int));
        for(int i = 0; i<MAX_N; ++i)
        {
            arr[0] += i;
            arr[1] = arr[0]*i;
        }
        free(arr);

        // cout << "Thread " << thread_no << " Output: Malloc and Free done\n";
    }

    tend = omp_get_wtime(); // getting the end time
    ttaken = tend - tstart; // calculating the time taken

    cout << "Time taken for the main part: " << ttaken << "\n";

    return 0;

}