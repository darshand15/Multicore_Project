#include <iostream>
#include <omp.h>
#include <unistd.h>

#include <cstdlib>

#define NUM_THREADS 8
#define ARR_SIZE 1000
#define ARR_2_SIZE 1000

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

        int** arr = (int**)malloc(ARR_SIZE*(sizeof(int*)));
        for(int i = 0; i<ARR_SIZE; ++i)
        {
            arr[i] = (int*)malloc((i+1)*ARR_2_SIZE*(sizeof(int)));
        }

        for(int i = 0; i<ARR_SIZE; i+=2)
        {
            free(arr[i]);
        }

        int** arr2 = (int**)malloc(ARR_SIZE*(sizeof(int*)));
        for(int i = 0; i<ARR_SIZE; ++i)
        {
            arr2[i] = (int*)malloc((i+1)*ARR_2_SIZE*(sizeof(int)));
        }

        free(arr);

        // cout << "Thread " << thread_no << " Output: Malloc and Free done\n";
    }

    tend = omp_get_wtime(); // getting the end time
    ttaken = tend - tstart; // calculating the time taken

    cout << "Time taken for the main part: " << ttaken << "\n";

    sleep(100);

    return 0;

}