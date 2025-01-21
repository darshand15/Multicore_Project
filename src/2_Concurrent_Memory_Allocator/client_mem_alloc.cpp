#include <iostream>

#include "custom_mem_alloc.h"

using namespace std;

int main()
{
    cout << "Start of main\n";
    display_free_list();
    display_mem_map();

    int* arr1 = (int*)my_mem_alloc(513*sizeof(int));
    cout << "After int mem1 alloc\n";
    display_free_list();
    display_mem_map();

    double* arr2 = (double*)my_mem_alloc(791*sizeof(double));
    cout << "After double mem2 alloc\n";
    display_free_list();
    display_mem_map();
    
    my_mem_free(arr1);
    my_mem_free(arr1);
    cout << "After int mem1 free\n";
    display_free_list();
    display_mem_map();

    int* arr3 = (int*)my_mem_alloc(51*sizeof(int));
    cout << "After int mem3 alloc\n";
    display_free_list();
    display_mem_map();

    my_mem_free(arr3);
    cout << "After int mem3 free\n";
    display_free_list();
    display_mem_map();

    my_mem_free(arr2);
    cout << "After int mem2 free\n";
    display_free_list();
    display_mem_map();

    int* arr4 = (int*)my_mem_alloc(351*sizeof(int));
    cout << "After int mem4 alloc\n";
    display_free_list();
    display_mem_map();

    my_mem_free(arr4);
    cout << "After int mem4 free\n";
    display_free_list();
    display_mem_map();

    

    return 0;
}