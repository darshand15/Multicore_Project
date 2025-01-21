#include <iostream>
#include <sys/mman.h>
#include <unistd.h>
#include <climits>

#include "custom_mem_alloc.h"

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

book_keeping_node* head_bk_node = NULL;
book_keeping_node* tail_bk_node = NULL;
free_blk_list* head_free_l = NULL;
int* last_address_mmap = NULL;

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

free_blk_list* search_free_blk(size_t req_size)
{
    free_blk_list* trav = head_free_l;
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

void insert_at_front_free_list(book_keeping_node* free_bk_node)
{
    free_bk_node->is_allocated = false;
    free_blk_list* new_free_blk = (free_blk_list*)malloc(sizeof(free_blk_list));
    new_free_blk->ptr_bk_node = free_bk_node;

    if(head_free_l == NULL)
    {
        head_free_l = new_free_blk;
        new_free_blk->next = NULL;
        new_free_blk->prev = NULL;
    }
    else
    {
        new_free_blk->next = head_free_l;
        new_free_blk->prev = NULL;
        head_free_l->prev = new_free_blk;
        head_free_l = new_free_blk;
    }
}

void delete_blk_from_free_list(free_blk_list* del_blk)
{
    del_blk->ptr_bk_node->is_allocated = true;

    //block to be deleted is the head node in the free list
    if(head_free_l == del_blk)
    {
        if(del_blk->next != NULL)
        {
            del_blk->next->prev = NULL;
        }
        head_free_l = del_blk->next;
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

free_blk_list* find_free_blk_of_bk_node(book_keeping_node* bk_node)
{
    free_blk_list* trav = head_free_l;
    while(trav != NULL && trav->ptr_bk_node != bk_node)
    {
        trav = trav->next;
    }
    return trav;
}

free_blk_list* merge_free_blks(book_keeping_node* bk_node)
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
        free_blk = find_free_blk_of_bk_node(bk_node->prev);

        bk_node->prev->size_mem_blk += bk_node->size_mem_blk;
        bk_node->prev->next = bk_node->next;
        if(bk_node->next != NULL)
        {
            bk_node->next->prev = bk_node->prev;
        }

        if(tail_bk_node == bk_node)
        {
            tail_bk_node = bk_node->prev;
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

        free_blk_list* free_blk_bk_next = find_free_blk_of_bk_node(bk_node->next);
        
        bk_node->size_mem_blk += bk_node->next->size_mem_blk;

        if(tail_bk_node == bk_node->next)
        {
            tail_bk_node = bk_node;
        }

        bk_node->next = bk_node->next->next;
        if(bk_node->next != NULL)
        {
            bk_node->next->prev = bk_node;
        }

        if(prev_is_free)
        {
            delete_blk_from_free_list(free_blk_bk_next);
        }
        else
        {
            free_blk_bk_next->ptr_bk_node = bk_node;
            bk_node->is_allocated = false;
            free_blk = free_blk_bk_next;
        }

        next_is_free = true;
    }

    return free_blk;
}

void split_rem_mem_free_blk(free_blk_list* free_blk, size_t required_size)
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

    if(tail_bk_node == free_blk->ptr_bk_node)
    {
        tail_bk_node = split_bk_node;
    }

    insert_at_front_free_list(split_bk_node);

}

void split_rem_mem_bk(book_keeping_node* orig_bk_node, size_t required_size)
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

    if(tail_bk_node == orig_bk_node)
    {
        tail_bk_node = split_bk_node;
    }

    insert_at_front_free_list(split_bk_node);

}

int unmap(free_blk_list* free_blk, size_t unmap_size)
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
    if(head_bk_node == bk_node)
    {
        head_bk_node = bk_node->next;
    }

    if(tail_bk_node == bk_node)
    {
        tail_bk_node = bk_node->prev;
    }

    if(last_address_mmap == (int*)(bk_node))
    {
        last_address_mmap = (int*)(bk_node) - bk_node->size_mem_blk;
    }

    // modify free list accordingly
    delete_blk_from_free_list(free_blk);

    return munmap((void*)(bk_node), unmap_size);

}

void* my_mem_alloc(size_t size)
{
    if(size <= 0)
    {
        return NULL;
    }

    size_t required_size = align_to_word_boundary(size + sizeof(book_keeping_node));
    free_blk_list* free_blk = search_free_blk(required_size);
    void* ret = NULL;

    // set is_alloc for free_blk

    if(free_blk != NULL)
    {
        free_blk->ptr_bk_node->is_allocated = true;
        split_rem_mem_free_blk(free_blk, required_size);
        ret = (void*)((char*)(free_blk->ptr_bk_node) + sizeof(book_keeping_node));
        delete_blk_from_free_list(free_blk);
        return ret;
    }

    size_t req_page_size = align_to_page_boundary(required_size);
    
    void* new_mmap_mem = mmap(last_address_mmap, req_page_size, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    if(new_mmap_mem == MAP_FAILED)
    {
        return NULL;
    }

    book_keeping_node* bk_node = (book_keeping_node*)(new_mmap_mem);
    bk_node->next = NULL;
    bk_node->prev = tail_bk_node;
    bk_node->size_mem_blk = req_page_size;
    bk_node->is_allocated = true;

    if(head_bk_node == NULL)
    {
        head_bk_node = bk_node;
    }
    
    if(tail_bk_node != NULL)
    {
        tail_bk_node->next = bk_node;
    }
    tail_bk_node = bk_node;

    split_rem_mem_bk(bk_node, required_size);

    last_address_mmap = (int*)((char*)(new_mmap_mem) + req_page_size);

    return (void*)((char*)(bk_node) + sizeof(book_keeping_node));
}

void my_mem_free(void* ptr)
{
    if(ptr == NULL)
    {
        return;
    }

    book_keeping_node* bk_node_free = (book_keeping_node*)((char*)(ptr) - sizeof(book_keeping_node));
    
    // node was already freed previously
    if(bk_node_free->is_allocated == false)
    {
        return;
    }

    // cases where we should unmap
    size_t page_size = (size_t)(sysconf(_SC_PAGESIZE));
    uintptr_t addr_bk = (uintptr_t)(bk_node_free);

    free_blk_list* ret_free_blk = merge_free_blks(bk_node_free);

    if(ret_free_blk == NULL)
    {
        insert_at_front_free_list(bk_node_free);
        ret_free_blk = head_free_l;
    }

    if((bk_node_free->size_mem_blk >= page_size) && ((addr_bk%page_size) == 0))
    {
        split_rem_mem_free_blk(ret_free_blk, ((bk_node_free->size_mem_blk) / page_size) * page_size);
        unmap(ret_free_blk, ((bk_node_free->size_mem_blk) / page_size) * page_size); // unmap with the size argument
    }


}

void display_free_list()
{
    cout << "\nFree List: \n";
    free_blk_list* trav = head_free_l;
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

}

void display_mem_map()
{
    cout << "\nMemory Mapping: \n";
    cout << "Size of book keeping node: " << sizeof(book_keeping_node) << "\n";

    book_keeping_node* trav = head_bk_node;

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
}
