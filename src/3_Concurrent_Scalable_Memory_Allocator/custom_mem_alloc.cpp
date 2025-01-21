#include <iostream>
#include <sys/mman.h>
#include <unistd.h>
#include <climits>
#include <mutex>
#include <omp.h>

#include "custom_mem_alloc.h"

#define NUM_THREADS 8
#define INIT_PAGES 2

using namespace std;

struct book_keeping_node
{
    struct book_keeping_node* next;
    struct book_keeping_node* prev;
    size_t size_mem_blk;
    bool is_allocated;
};
typedef struct book_keeping_node book_keeping_node;

struct free_blk_list
{
    struct free_blk_list* next;
    struct free_blk_list* prev;
    book_keeping_node* ptr_bk_node;
};
typedef struct free_blk_list free_blk_list;

struct per_thread_heap
{
    book_keeping_node* head_bk_node;
    book_keeping_node* tail_bk_node;
    free_blk_list* head_free_l;
    int* last_address_mmap;
    mutex per_thread_mutex;
    int num_pages_alloc;


    per_thread_heap()
    {
        head_bk_node = NULL;
        tail_bk_node = NULL;
        head_free_l = NULL;
        last_address_mmap = NULL;
        num_pages_alloc = 0;
    }
};
typedef struct per_thread_heap per_thread_heap;

per_thread_heap arr_per_th_heap[NUM_THREADS];
bool init_th_heaps_done = false;
mutex init_heap_mutex;

inline size_t align_to_word_boundary(size_t orig_size) 
{
  return (orig_size + sizeof(intptr_t) - 1) & ~(sizeof(intptr_t) - 1);
}

inline size_t align_to_page_boundary(size_t orig_size)
{
    size_t page_size = (size_t)(sysconf(_SC_PAGESIZE));
    double num_pgs = orig_size/(double)(page_size);
    size_t new_size;

    //performing ceil operation
    if((num_pgs - (int)(num_pgs)) > 0)
    {
        new_size = (size_t)(((int)(num_pgs) + 1)*page_size);
    }
    else
    {
        new_size = (size_t)(((int)(num_pgs))*page_size);
    }

    return new_size;
}

free_blk_list* search_free_blk(size_t req_size, per_thread_heap* ptr_th_heap)
{
    free_blk_list* trav = ptr_th_heap->head_free_l;
    free_blk_list* best_fit_free_blk = NULL;
    size_t best_fit_size = SIZE_MAX;

    // implementing the best fit algorithm
    while(trav != NULL)
    {
        if(trav->ptr_bk_node->size_mem_blk >= req_size && trav->ptr_bk_node->size_mem_blk < best_fit_size)
        {
            best_fit_size = trav->ptr_bk_node->size_mem_blk;
            best_fit_free_blk = trav;
        }

        trav = trav->next;
    }

    return best_fit_free_blk;
}

void insert_at_front_free_list(book_keeping_node* free_bk_node, per_thread_heap* ptr_th_heap)
{
    free_bk_node->is_allocated = false;
    free_blk_list* new_free_blk = (free_blk_list*)malloc(sizeof(free_blk_list));
    new_free_blk->ptr_bk_node = free_bk_node;

    if(ptr_th_heap->head_free_l == NULL)
    {
        ptr_th_heap->head_free_l = new_free_blk;
        new_free_blk->next = NULL;
        new_free_blk->prev = NULL;
    }
    else
    {
        new_free_blk->next = ptr_th_heap->head_free_l;
        new_free_blk->prev = NULL;
        ptr_th_heap->head_free_l->prev = new_free_blk;
        ptr_th_heap->head_free_l = new_free_blk;
    }
}

void delete_blk_from_free_list(free_blk_list* del_blk, per_thread_heap* ptr_th_heap)
{
    del_blk->ptr_bk_node->is_allocated = true;

    //block to be deleted is the head node in the free list
    if(ptr_th_heap->head_free_l == del_blk)
    {
        if(del_blk->next != NULL)
        {
            del_blk->next->prev = NULL;
        }
        ptr_th_heap->head_free_l = del_blk->next;
    }
    else
    {
        if(del_blk->next != NULL)
        {
            del_blk->next->prev = del_blk->prev;
        }
        del_blk->prev->next = del_blk->next;
    }
    free(del_blk);
}

free_blk_list* find_free_blk_of_bk_node(book_keeping_node* bk_node, per_thread_heap* ptr_th_heap)
{
    free_blk_list* trav = ptr_th_heap->head_free_l;
    while(trav != NULL && trav->ptr_bk_node != bk_node)
    {
        trav = trav->next;
    }
    return trav;
}

free_blk_list* merge_free_blks(book_keeping_node* bk_node, per_thread_heap* ptr_th_heap)
{
    bool prev_is_free = false;
    bool next_is_free = false;

    free_blk_list* free_blk = NULL;

    if((bk_node->prev != NULL) && (bk_node->prev->is_allocated == false) && (bk_node == ((book_keeping_node*)((char*)(bk_node->prev) + bk_node->prev->size_mem_blk))))
    {
        // cout << "Prev Case:\n";
        // cout << "add1: " << bk_node->prev + bk_node->size_mem_blk << "\nadd2: " << bk_node << "\n";
        // cout << "Diff of add: " << bk_node - bk_node->prev << "\n";
        // book_keeping_node* bk2 = (book_keeping_node*)((char*)(bk_node->prev) + bk_node->prev->size_mem_blk);
        // cout << "add1: " << bk2 << "\nadd2: " << bk_node << "\n"; 
        free_blk = find_free_blk_of_bk_node(bk_node->prev, ptr_th_heap);

        bk_node->prev->size_mem_blk += bk_node->size_mem_blk;
        bk_node->prev->next = bk_node->next;
        if(bk_node->next != NULL)
        {
            bk_node->next->prev = bk_node->prev;
        }

        if(ptr_th_heap->tail_bk_node == bk_node)
        {
            ptr_th_heap->tail_bk_node = bk_node->prev;
        }

        bk_node = bk_node->prev;
        prev_is_free = true;
    }

    if((bk_node->next != NULL) && (bk_node->next->is_allocated == false) && (((book_keeping_node*)((char*)(bk_node) + bk_node->size_mem_blk)) == bk_node->next))
    {
        // cout << "Next Case:\n";
        // cout << "add1: " << bk_node->next << "\nadd2: " << bk_node + bk_node->size_mem_blk << "\n";
        // cout << "Diff of add: " << bk_node->next - bk_node << "\n";
        // book_keeping_node* bk2 = (book_keeping_node*)((char*)(bk_node) + bk_node->size_mem_blk);
        // cout << "add1: " << bk2 << "\nadd2: " << bk_node->next << "\n";

        free_blk_list* free_blk_bk_next = find_free_blk_of_bk_node(bk_node->next, ptr_th_heap);
        
        bk_node->size_mem_blk += bk_node->next->size_mem_blk;

        if(ptr_th_heap->tail_bk_node == bk_node->next)
        {
            ptr_th_heap->tail_bk_node = bk_node;
        }

        bk_node->next = bk_node->next->next;
        if(bk_node->next != NULL)
        {
            bk_node->next->prev = bk_node;
        }

        if(prev_is_free)
        {
            delete_blk_from_free_list(free_blk_bk_next, ptr_th_heap);
        }
        else
        {
            free_blk_bk_next->ptr_bk_node = bk_node;
            bk_node->is_allocated = false;
            free_blk = free_blk_bk_next;
        }

        next_is_free = true;
    }

    // return (prev_is_free || next_is_free);
    return free_blk;
}

void split_rem_mem_free_blk(free_blk_list* free_blk, size_t required_size, per_thread_heap* ptr_th_heap)
{
    size_t rem_mem = free_blk->ptr_bk_node->size_mem_blk - required_size;

    if(rem_mem < (align_to_word_boundary(1 + sizeof(book_keeping_node))))
    {
        return;
    }

    free_blk->ptr_bk_node->size_mem_blk = required_size;

    book_keeping_node* split_bk_node = (book_keeping_node*)((char*)(free_blk->ptr_bk_node) + required_size);
    split_bk_node->is_allocated = false;
    split_bk_node->next = free_blk->ptr_bk_node->next;
    split_bk_node->prev = free_blk->ptr_bk_node;
    split_bk_node->size_mem_blk = rem_mem;

    if(free_blk->ptr_bk_node->next != NULL)
    {
        free_blk->ptr_bk_node->next->prev = split_bk_node;
    }

    free_blk->ptr_bk_node->next = split_bk_node;

    if(ptr_th_heap->tail_bk_node == free_blk->ptr_bk_node)
    {
        ptr_th_heap->tail_bk_node = split_bk_node;
    }

    insert_at_front_free_list(split_bk_node, ptr_th_heap);

}

void split_rem_mem_bk(book_keeping_node* orig_bk_node, size_t required_size, per_thread_heap* ptr_th_heap)
{
    size_t rem_mem = orig_bk_node->size_mem_blk - required_size;

    if(rem_mem < (align_to_word_boundary(1 + sizeof(book_keeping_node))))
    {
        return;
    }

    orig_bk_node->size_mem_blk = required_size;

    book_keeping_node* split_bk_node = (book_keeping_node*)((char*)(orig_bk_node) + required_size);
    split_bk_node->is_allocated = false;
    split_bk_node->next = orig_bk_node->next;
    split_bk_node->prev = orig_bk_node;
    split_bk_node->size_mem_blk = rem_mem;

    if(orig_bk_node->next != NULL)
    {
        orig_bk_node->next->prev = split_bk_node;
    }

    orig_bk_node->next = split_bk_node;

    if(ptr_th_heap->tail_bk_node == orig_bk_node)
    {
        ptr_th_heap->tail_bk_node = split_bk_node;
    }

    insert_at_front_free_list(split_bk_node, ptr_th_heap);

}

int unmap(free_blk_list* free_blk, size_t unmap_size, per_thread_heap* ptr_th_heap)
{
    book_keeping_node* bk_node = free_blk->ptr_bk_node;

    if(bk_node->prev != NULL)
    {
        bk_node->prev->next = bk_node->next;
    }
    if(bk_node->next != NULL)
    {
        bk_node->next->prev = bk_node->prev;
    }

    //need to change head of bk if required
    if(ptr_th_heap->head_bk_node == bk_node)
    {
        ptr_th_heap->head_bk_node = bk_node->next;
    }

    if(ptr_th_heap->tail_bk_node == bk_node)
    {
        ptr_th_heap->tail_bk_node = bk_node->prev;
    }

    if(ptr_th_heap->last_address_mmap == (int*)(bk_node))
    {
        ptr_th_heap->last_address_mmap = (int*)(bk_node) - bk_node->size_mem_blk;
    }

    // modify free list accordingly
    delete_blk_from_free_list(free_blk, ptr_th_heap);

    return munmap((void*)(bk_node), unmap_size);

}

void init_th_heaps()
{
    size_t page_size = (size_t)(sysconf(_SC_PAGESIZE));
    size_t required_size = INIT_PAGES*page_size;
    per_thread_heap* ptr_th_heap = NULL;

    for(int i = 0; i<NUM_THREADS; ++i)
    {
        ptr_th_heap = &(arr_per_th_heap[i]);
        ptr_th_heap->per_thread_mutex.lock();
    
        void* new_mmap_mem = mmap(NULL, required_size, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
        if(new_mmap_mem == MAP_FAILED)
        {
            ptr_th_heap->per_thread_mutex.unlock();
            return;
        }

        ptr_th_heap->num_pages_alloc = INIT_PAGES;

        book_keeping_node* bk_node = (book_keeping_node*)(new_mmap_mem);
        bk_node->next = NULL;
        bk_node->prev = NULL;
        bk_node->size_mem_blk = required_size;
        bk_node->is_allocated = false;

        ptr_th_heap->head_bk_node = bk_node;
        ptr_th_heap->tail_bk_node = bk_node;

        insert_at_front_free_list(bk_node, ptr_th_heap);

        ptr_th_heap->last_address_mmap = (int*)((char*)(new_mmap_mem) + required_size);
    
        ptr_th_heap->per_thread_mutex.unlock();
    }
}

void* my_mem_alloc(size_t size)
{
    if(size <= 0)
    {
        return NULL;
    }

    init_heap_mutex.lock();
    if(init_th_heaps_done == false)
    {
        init_th_heaps();
        init_th_heaps_done = true;
    }
    init_heap_mutex.unlock();

    size_t required_size = align_to_word_boundary(size + sizeof(book_keeping_node));

    int thread_no = omp_get_thread_num();
    per_thread_heap* ptr_th_heap = &(arr_per_th_heap[thread_no]);

    ptr_th_heap->per_thread_mutex.lock();
    
    free_blk_list* free_blk = search_free_blk(required_size, ptr_th_heap);
    void* ret = NULL;

    // set is_alloc for free_blk

    if(free_blk != NULL)
    {
        free_blk->ptr_bk_node->is_allocated = true;
        split_rem_mem_free_blk(free_blk, required_size, ptr_th_heap);
        ret = (void*)((char*)(free_blk->ptr_bk_node) + sizeof(book_keeping_node));
        delete_blk_from_free_list(free_blk, ptr_th_heap);

        ptr_th_heap->per_thread_mutex.unlock();
        return ret;
    }

    size_t req_page_size = align_to_page_boundary(required_size);
    
    void* new_mmap_mem = mmap(ptr_th_heap->last_address_mmap, req_page_size, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    if(new_mmap_mem == MAP_FAILED)
    {
        ptr_th_heap->per_thread_mutex.unlock();
        return NULL;
    }

    size_t page_size = (size_t)(sysconf(_SC_PAGESIZE));
    ptr_th_heap->num_pages_alloc = ptr_th_heap->num_pages_alloc + (req_page_size/page_size);

    book_keeping_node* bk_node = (book_keeping_node*)(new_mmap_mem);
    bk_node->next = NULL;
    bk_node->prev = ptr_th_heap->tail_bk_node;
    bk_node->size_mem_blk = req_page_size;
    bk_node->is_allocated = true;

    if(ptr_th_heap->head_bk_node == NULL)
    {
        ptr_th_heap->head_bk_node = bk_node;
    }
    
    if(ptr_th_heap->tail_bk_node != NULL)
    {
        ptr_th_heap->tail_bk_node->next = bk_node;
    }
    ptr_th_heap->tail_bk_node = bk_node;

    split_rem_mem_bk(bk_node, required_size, ptr_th_heap);

    ptr_th_heap->last_address_mmap = (int*)((char*)(new_mmap_mem) + req_page_size);
    
    ptr_th_heap->per_thread_mutex.unlock();

    return (void*)((char*)(bk_node) + sizeof(book_keeping_node));
}

void my_mem_free(void* ptr)
{
    if(ptr == NULL)
    {
        return;
    }

    book_keeping_node* bk_node_free = (book_keeping_node*)((char*)(ptr) - sizeof(book_keeping_node));

    int thread_no = omp_get_thread_num();
    bool node_is_found = true;
    per_thread_heap* ptr_th_heap = &(arr_per_th_heap[thread_no]);
    ptr_th_heap->per_thread_mutex.lock();

    //search through the current thread's heap to verify if it contains the memory to be freed
    book_keeping_node* trav = ptr_th_heap->head_bk_node;
    while((trav != NULL) && (trav != bk_node_free))
    {
        trav = trav->next;
    }

    // node not found in current thread's heap
    if(trav == NULL)
    {
        node_is_found = false;
        ptr_th_heap->per_thread_mutex.unlock();
        for(int i = 0; i<NUM_THREADS; ++i)
        {
            if(i != thread_no)
            {
                ptr_th_heap = &(arr_per_th_heap[i]);
                ptr_th_heap->per_thread_mutex.lock();

                trav = ptr_th_heap->head_bk_node;
                while((trav != NULL) && (trav != bk_node_free))
                {
                    trav = trav->next;
                }

                if(trav != NULL)
                {
                    node_is_found = true;
                    break;
                }
                else
                {
                    ptr_th_heap->per_thread_mutex.unlock();
                }
            }
        }
    }

    if(node_is_found == false)
    {
        // pointer passed to free is invalid and is not the start of any previously allocated memory
        return;
    }
    
    // node was already freed previously
    if(bk_node_free->is_allocated == false)
    {
        ptr_th_heap->per_thread_mutex.unlock();
        return;
    }

    // cases where we should unmap
    size_t page_size = (size_t)(sysconf(_SC_PAGESIZE));
    uintptr_t addr_bk = (uintptr_t)(bk_node_free);

    free_blk_list* ret_free_blk = merge_free_blks(bk_node_free, ptr_th_heap);


    // bool ret_merge = merge_free_blks(bk_node_free, ptr_th_heap);

    if(ret_free_blk == NULL)
    {
        insert_at_front_free_list(bk_node_free, ptr_th_heap);
        ret_free_blk = ptr_th_heap->head_free_l;
    }

    if((bk_node_free->size_mem_blk >= page_size) && ((addr_bk%page_size) == 0))
    {
        int unmap_num_pgs = (bk_node_free->size_mem_blk) / page_size;
        if(ptr_th_heap->num_pages_alloc - unmap_num_pgs >= INIT_PAGES)
        {
            split_rem_mem_free_blk(ret_free_blk, ((bk_node_free->size_mem_blk) / page_size) * page_size, ptr_th_heap);
            unmap(ret_free_blk, ((bk_node_free->size_mem_blk) / page_size) * page_size, ptr_th_heap); // unmap with the size argument
            ptr_th_heap->num_pages_alloc -= unmap_num_pgs;
        }
    }

    ptr_th_heap->per_thread_mutex.unlock();
}

void display_free_list()
{
    for(int i = 0; i<NUM_THREADS; ++i)
    {
        per_thread_heap* ptr_th_heap = &(arr_per_th_heap[i]);
        ptr_th_heap->per_thread_mutex.lock();

        cout << "\nFree List for thread " << i << ": \n";
        free_blk_list* trav = ptr_th_heap->head_free_l;
        while(trav != NULL)
        {
            cout << "-------------------------------------------------\n";
            cout << "Allocation Status: ";

            if(trav->ptr_bk_node->is_allocated)
            {
                cout << "Allocated";
            }
            else
            {
                cout << "Free";
            }
            cout << "\n";
            cout << "Size of Mem Blk: " << trav->ptr_bk_node->size_mem_blk << "\n";
            cout << "Address of Mem Blk: " << trav->ptr_bk_node << "\n";
            
            trav = trav->next;
        }
        cout << "-------------------------------------------------\n";

        ptr_th_heap->per_thread_mutex.unlock();
    }
}

void display_mem_map()
{
    for(int i = 0; i<NUM_THREADS; ++i)
    {
        per_thread_heap* ptr_th_heap = &(arr_per_th_heap[i]);
        ptr_th_heap->per_thread_mutex.lock();

        cout << "\nMemory Mapping for thread " << i << ": \n";
        cout << "Number of pages mapped to heap of thread " << i << ": " << ptr_th_heap->num_pages_alloc << "\n";
        cout << "Size of book keeping node: " << sizeof(book_keeping_node) << "\n";

        book_keeping_node* trav = ptr_th_heap->head_bk_node;

        while(trav != NULL)
        {
            cout << "=================================================\n";
            cout << "Allocation Status: ";

            if(trav->is_allocated)
            {
                cout << "Allocated";
            }
            else
            {
                cout << "Free";
            }
            cout << "\n";
            cout << "Size of Mem Blk: " << trav->size_mem_blk << "\n";
            cout << "Address of Mem Blk: " << trav << "\n";

            trav = trav->next;
        }
        cout << "=================================================\n";

        ptr_th_heap->per_thread_mutex.unlock();
    }
}
