#include <iostream>
#include <omp.h>

#include <cstdlib>

// #define NUM_THREADS 1 // change for different runs
#define MAX_N 100000

using namespace std;

int main(int argc, char* argv[])
{
    double tstart = 0.0;
    double tend = 0.0;
    double ttaken = 0.0;

    int num_threads = atoi(argv[1]);

    omp_set_num_threads(num_threads);

    tstart = omp_get_wtime(); //getting the start time

    int num_obj = MAX_N/num_threads;

    #pragma omp parallel
    {
        int thread_no = omp_get_thread_num();

        int** arr = (int**)malloc(num_obj*(sizeof(int*)));
        for(int i = 0; i<num_obj; ++i)
        {
            arr[i] = (int*)malloc(3*sizeof(int));
        }

        for(int i = 0; i<num_obj; ++i)
        {
            free(arr[i]);
        }

        free(arr);

        // cout << "Thread " << thread_no << " Output: Malloc and Free done\n";
    }

    tend = omp_get_wtime(); // getting the end time
    ttaken = tend - tstart; // calculating the time taken

    cout << "Time taken for the main part: " << ttaken << "\n";

    return 0;

}