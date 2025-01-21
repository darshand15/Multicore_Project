#include <iostream>
#include <omp.h>

// #include <cstdlib>
#include "custom_mem_alloc.h"

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

    int* arr0 = (int*)my_mem_alloc(sizeof(int));
    int* arr1 = (int*)my_mem_alloc(sizeof(int));
    int* arr2 = (int*)my_mem_alloc(sizeof(int));
    int* arr3 = (int*)my_mem_alloc(sizeof(int));
    int* arr4 = (int*)my_mem_alloc(sizeof(int));
    int* arr5 = (int*)my_mem_alloc(sizeof(int));
    int* arr6 = (int*)my_mem_alloc(sizeof(int));
    int* arr7 = (int*)my_mem_alloc(sizeof(int));

    #pragma omp parallel
    {
        int thread_no = omp_get_thread_num();

        if(thread_no == 0)
        {
            my_mem_free(arr0);
        }
        else if(thread_no == 1)
        {
            my_mem_free(arr1);
        }
        else if(thread_no == 2)
        {
            my_mem_free(arr2);
        }
        else if(thread_no == 3)
        {
            my_mem_free(arr3);
        }
        else if(thread_no == 4)
        {
            my_mem_free(arr4);
        }
        else if(thread_no == 5)
        {
            my_mem_free(arr5);
        }
        else if(thread_no == 6)
        {
            my_mem_free(arr6);
        }
        else if(thread_no == 7)
        {
            my_mem_free(arr7);
        }

        int* arr = (int*)my_mem_alloc(sizeof(int));
        for(int i = 0; i<MAX_N; ++i)
        {
            arr[0] += i*i;
        }
        my_mem_free(arr);

        // cout << "Thread " << thread_no << " Output: Malloc and Free done\n";
    }

    tend = omp_get_wtime(); // getting the end time
    ttaken = tend - tstart; // calculating the time taken

    cout << "Time taken for the main part: " << ttaken << "\n";

    return 0;

}