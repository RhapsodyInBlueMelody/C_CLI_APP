#include <alloca.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef struct Allocation{
	void *address;
	size_t size;
	struct Allocation *next;
} Allocation;

Allocation *allocations = NULL;

void add_allo(void *address, size_t size){
	Allocation *new_alloc = (Allocation *)malloc(sizeof(Allocation));
	new_alloc->address = address;
	new_alloc->size = size;
	new_alloc->next = allocations;
	allocations = new_alloc;
}

void remove_alloc(void *address){
	Allocation **currentAlloc = &allocations;
	while (*currentAlloc) {
		if ((*currentAlloc)->address == address) {
		Allocation *toFree = *currentAlloc;
		*currentAlloc = (*currentAlloc)->next;
		free(toFree);
		return;
		}
		currentAlloc = &(*currentAlloc)->next;
	}
}

void *memlloc(size_t size){
	void *ptr = malloc(size);
	if (ptr){
	add_allo(ptr, size);
	printf("Allocated %zu bytes at %p\n", size, ptr);
	}
	return ptr;
}

void *my_calloc(size_t num, size_t size) {
    void *ptr = calloc(num, size);
    if (ptr) {
        add_allo(ptr, num * size);
        printf("Allocated %zu bytes at %p\n", num * size, ptr);
    }
    return ptr;
}

void *my_realloc(void *ptr, size_t size) {
    if (ptr) {
        remove_alloc(ptr);
    }
    void *new_ptr = realloc(ptr, size);
    if (new_ptr) {
        add_allo(new_ptr, size);
        printf("Reallocated to %zu bytes at %p\n", size, new_ptr);
    }
    return new_ptr;
}

int is_freed(void *address) {
    Allocation *current = allocations;
    while (current) {
        if (current->address == address) {
            return 0;
        }
        current = current->next;
    }
    return 1; 
}

void fremlloc(void *ptr){
	if (is_freed(ptr)) {
	printf("Double Freed memory at %p\n", ptr);
	return;
	}
	remove_alloc(ptr);
	free(ptr);
}


void meleaks(){
	Allocation *current = allocations;
	while (current) {
	printf("Memory Leaks Detected: address=%p, size=%zu \n", current->address, current->size);
	current = current->next;
	}
}


int main() {
    char *str = memlloc(50);
    strcpy(str, "Hello, Memory Profiler!");
    fremlloc(str);
    fremlloc(str);

    meleaks();
    return 0;
}
