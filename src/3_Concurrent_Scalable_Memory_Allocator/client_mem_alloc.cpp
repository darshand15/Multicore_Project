#include <iostream>
#include <omp.h>

#define NUM_THREADS 8

#include "custom_mem_alloc.h"

using namespace std;

int main()
{
    cout << "Start of main\n";

    #pragma omp single
    {
        display_free_list();
        display_mem_map();
    }

    int *arr1 = NULL;
    double *arr2 = NULL;
    int* arr3 = NULL;

    omp_set_num_threads(NUM_THREADS);

    #pragma omp parallel
    {
        int thread_no = omp_get_thread_num();
        if(thread_no == 0)
        {
            arr1 = (int*)my_mem_alloc(513*sizeof(int));
            arr2 = (double*)my_mem_alloc(791*sizeof(double));
        }
        else if(thread_no == 1)
        {
            arr3 = (int*)my_mem_alloc(51*sizeof(int));
            my_mem_free(arr3);
        }
        else
        {
            int* arr4 = (int*)my_mem_alloc(351*sizeof(int));
        }
    }

    #pragma omp single
    {
        display_free_list();
        display_mem_map();
    }

    #pragma omp parallel
    {
        int thread_no = omp_get_thread_num();
        if(thread_no == 0)
        {
            my_mem_free(arr3);
        }
        else
        {
            my_mem_free(arr1);
            my_mem_free(arr2);
        }
    }

    #pragma omp single
    {
        display_free_list();
        display_mem_map();
    }
    

    return 0;
}