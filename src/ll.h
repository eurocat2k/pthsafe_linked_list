#ifndef F732C60A_3DB7_4F51_BF40_12E25A0C6BA7
#define F732C60A_3DB7_4F51_BF40_12E25A0C6BA7
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <stdbool.h>

// for locking and unlocking rwlocks along with `locktype_t`
#ifndef RWLOCK
#define RWLOCK(lt, lk) ((lt) == l_read)                   \
                           ? pthread_rwlock_rdlock(&(lk)) \
                           : pthread_rwlock_wrlock(&(lk))
#endif // !RWLOCK

#ifndef RWUNLOCK
#define RWUNLOCK(lk) pthread_rwlock_unlock(&(lk));
#endif // !RWUNLOCK
// 

/* type definitions */
typedef enum locktype {
    l_read,
    l_write
} locktype_t;


// GENERAL DESTROYER FUNCTION
typedef void (*gen_func_t)(void *);
typedef bool (*condition_func_t)(void *, void*);

// NODE TYPE
typedef struct _node Node_t;
struct _node {
    struct _node *next;
    void *data;
    pthread_rwlock_t mtx;       // rw mutex
    gen_func_t destroy;
    gen_func_t dumper;
};
// LLIST TYPE
typedef struct _llist List_t;
struct _llist {
    size_t size;                // size of the list
    Node_t *head;               // pointer to the first element of the list
    pthread_rwlock_t mtx;       // mutex for thread safety
    gen_func_t destroy;    // a function that is called every time a value is deleted with a pointer to that value
    gen_func_t dumper;     // a function that can print the values in a linked list
};
typedef struct _nsllist nsLList_t;
struct _nsllist {
    List_t* (*create)(gen_func_t teardown); // list create
    void (*destroy)(List_t*);               // list destroy
    int (*insert_node_index)(List_t*, void *, size_t, gen_func_t, gen_func_t);  // insert node with self destructor, and self dumper function at index
    int (*insert_node_first)(List_t*, void *, gen_func_t, gen_func_t);          // insert node with self destructor, and self dumper function at index 0
    int (*insert_node_last)(List_t*, void *, gen_func_t, gen_func_t);           // insert node with self destructor, and self dumper function at list size
    int (*remove_node_index)(List_t*, size_t index);                            // remove node from list at index
    int (*remove_node_first)(List_t*);                                          // remove node from list at index 0
    int (*remove_node_search)(List_t*, condition_func_t cond, void *filter);    // remove node from list where the first occurrence of filter matches with node data
    void *(*get_node_index)(List_t *list, size_t index);                        // returns node in the list at index
    void *(*get_node_first)(List_t *list);                                      // returns the first node - list head node
    void (*map)(struct _llist *list, gen_func_t fn);                            // executes function on each nodes in the list
    void (*dump)(struct _llist *list);                                          // dumps node elements - using generic dumper function or node's self dumper if any
    void (*no_node_teardown)(void *n);                                          // dummy
};

extern struct _nsllist nsIODELList;

#endif /* F732C60A_3DB7_4F51_BF40_12E25A0C6BA7 */
