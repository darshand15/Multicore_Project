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

    int* arr0 = (int*)malloc(sizeof(int));
    int* arr1 = (int*)malloc(sizeof(int));
    int* arr2 = (int*)malloc(sizeof(int));
    int* arr3 = (int*)malloc(sizeof(int));
    int* arr4 = (int*)malloc(sizeof(int));
    int* arr5 = (int*)malloc(sizeof(int));
    int* arr6 = (int*)malloc(sizeof(int));
    int* arr7 = (int*)malloc(sizeof(int));

    #pragma omp parallel
    {
        int thread_no = omp_get_thread_num();

        if(thread_no == 0)
        {
            free(arr0);
        }
        else if(thread_no == 1)
        {
            free(arr1);
        }
        else if(thread_no == 2)
        {
            free(arr2);
        }
        else if(thread_no == 3)
        {
            free(arr3);
        }
        else if(thread_no == 4)
        {
            free(arr4);
        }
        else if(thread_no == 5)
        {
            free(arr5);
        }
        else if(thread_no == 6)
        {
            free(arr6);
        }
        else if(thread_no == 7)
        {
            free(arr7);
        }

        int* arr = (int*)malloc(sizeof(int));
        for(int i = 0; i<MAX_N; ++i)
        {
            arr[0] += i*i;
        }
        free(arr);

        // cout << "Thread " << thread_no << " Output: Malloc and Free done\n";
    }

    tend = omp_get_wtime(); // getting the end time
    ttaken = tend - tstart; // calculating the time taken

    cout << "Time taken for the main part: " << ttaken << "\n";

    return 0;

}