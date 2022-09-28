#ifndef F732C60A_3DB7_4F51_BF40_12E25A0C6BA7
#define F732C60A_3DB7_4F51_BF40_12E25A0C6BA7
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <stdbool.h>

#include "sf.h"

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
typedef bool (*condition_func_t)(void *);

// LLIST FOR CLIENT LIST
typedef struct _node Node_t;
struct _node {
    struct _node *next;
    void *data;
    pthread_rwlock_t mtx;       // rw mutex
    gen_func_t destroy;
};

typedef struct _llist List_t;
struct _llist {
    size_t size;                // size of the list
    Node_t *head;               // pointer to the first element of the list
    pthread_rwlock_t mtx;       // mutex for thread safety
    gen_func_t val_teardown;    // a function that is called every time a value is deleted with a pointer to that value
    gen_func_t val_printer;     // a function that can print the values in a linked list
};


typedef struct _nsllist nsLList_t;

struct _nsllist {
    List_t* (*create)(gen_func_t val_teardown);                     // list create
    void (*destroy)(List_t*);
    int (*insert_node_index)(List_t*, void *, size_t);
    int (*insert_node_first)(List_t*, void *);
    int (*insert_node_last)(List_t*, void *);
    int (*remove_node_index)(List_t*, size_t index);
    int (*remove_node_first)(List_t*);
    int (*remove_node_search)(List_t*, condition_func_t cond);
    void *(*get_node_index)(List_t *list, size_t index);
    void *(*get_node_first)(List_t *list);
    void (*map)(struct _llist *list, gen_func_t fn);
    void (*dump)(struct _llist list);
    void (*no_node_teardown)(void *n);
};

extern struct _nsllist nsIODELList;

#endif /* F732C60A_3DB7_4F51_BF40_12E25A0C6BA7 */
