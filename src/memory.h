#ifndef MEMORY_H
#define MEMORY_H

#include <stddef.h>



#define MEM_MULT 8 // how many times newsizeb should be multiplied?
                   // note: newsizeb * EDMEM_MULT may not always be the final size.

#define MEMRESIZE_ERROR -1

// returns the same pointer that is passed as
// argument(void* ptr) if memory resizing fails.
// but if the size is 0, then NULL is returned.
// 'esizeb' is the size of the element (in bytes).
// 'ptrsize' is the current number of elements.
// 'nsize_ptr' actual size that the memory is resized to is set here if its not NULL.
//             it is set to MEMRESIZE_ERROR if any errors happens.
void* m_resize_array
      (void* ptr, size_t esizeb, size_t ptrsize, size_t newsize, long int* nsize_ptr);




// safe reallocarray function
// windows don't have reallocarray this wrapper is used to avoid
// overflow in multiplication
void *m_safe_reallocarray(void *ptr, size_t nmemb, size_t size);

#endif
