#include <iostream>

#include "custom_mem_alloc.h"

using namespace std;

int main()
{
    cout << "Start of main\n";
    display_free_list();
    display_mem_map();

    int* arr1 = (int*)my_mem_alloc(5096);
    cout << "After int mem1 alloc\n";
    display_free_list();
    display_mem_map();

    int* arr2 = (int*)my_mem_alloc(96);
    cout << "After int mem2 alloc\n";
    display_free_list();
    display_mem_map();

    my_mem_free(arr1);
    cout << "After int mem1 free\n";
    display_free_list();
    display_mem_map();

    

    return 0;
}