#ifndef custom_mem_alloc_h
#define custom_mem_alloc_h

#include <stddef.h>

void* my_mem_alloc(size_t size);

void my_mem_free(void* ptr);

void display_free_list();
void display_mem_map();


#endif