# pthsafe_linked_list
thread safe linked list with test code - inspired by [r-medina's ll](https://github.com/r-medina/ll) package.

## Install

You need to have **autotools** installed - or at least it is advised to be existing before install.

In that case if you've got **autotools** installed, then call ***autoreconf -imf*** in the root directory of the package - where you can find **configure.ac** file.

```bash
    % autoreconf -imf
```

If no errors occur, then you need to run ***configure*** script as follows:

```bash
    % ./configure
```

This script will create ***src/Makfile*** for you, which then should be used to build the test program.

The contents of the package directory will look like this (*see below*):

```bash
    ├── aclocal.m4
    ├── autom4te.cache
    │   ├── output.0
    │   ├── requests
    │   └── traces.0
    ├── compile
    ├── config.guess
    ├── config.h
    ├── config.h.in
    ├── config.log
    ├── config.status
    ├── config.sub
    ├── configure
    ├── configure.ac
    ├── configure~
    ├── depcomp
    ├── install-sh
    ├── missing
    ├── README.md
    ├── src
    │   ├── ll.c
    │   ├── ll.h
    │   ├── main.c
    │   ├── Makefile
    │   ├── Makefile.am
    │   ├── Makefile.in
    │   ├── sf.c
    │   └── sf.h
    ├── stamp-h1
    └── VERSION
```

From now it's quite simpe, you just go into ***src*** subdirectory, then call **make**.

## Test

If you have got **valgrind** tool, then you can check for the memory leaks, and inconsistencies.

```bash
(cd src && make) && valgrind -s --tool=drd --trace-rwlock=yes ./src/tsllist_demo && (cd src/ && make clean)
```

For me - on my machine - the test gives the following result:

```bash
    ==29872== drd, a thread error detector
    ==29872== Copyright (C) 2006-2020, and GNU GPL'd, by Bart Van Assche.
    ==29872== Using Valgrind-3.19.0 and LibVEX; rerun with -h for copyright info
    ==29872== Command: ./src/tsllist_demo
    ==29872== 
    PASS Test 1!
    PASS Test 2!
    PASS Test 3!
    PASS Test 4!
    PASS Test 5!
    PASS Test 6!
    PASS Test 7!
    PASS Test 8!
    PASS Test 9!
    PASS Test 10!
    PASS Test 11!
    PASS Test 12!
    PASS Test 13!
    (LIST:
    0 1 2 3 4 5 6), length: 7
    (LIST:
    1 2 3 4 5 6), length: 6
    (LIST:
    1 3 4 5 6), length: 5
    (LIST:
    1 3 5 6), length: 4
    (LIST:
    1 3 5 6), length: 4
    (LIST:
    1 5 6), length: 3
    (LIST:
    1 5 6), length: 3
    (LIST:
    3 1 5 6), length: 4
    (LIST:
    3 1 5 6 3), length: 5
    (LIST:
    1 5 6 3), length: 4
    (LIST:
    1 5 6), length: 3
    PASSED all 14 tests!
    ==29872== 
    ==29872== ERROR SUMMARY: 0 errors from 0 contexts (suppressed: 0 from 0)
```

## Hints

In general the examples of the linked lists used to deal with scalar data - as node value. It's ok for simplicity, and make easy to code and test without taking care of memory issues - other than default destroy action performed by ***free(3)*** system call. However it's likely not the case in "*real world*" usage of the linked lists. The nodes shall consist of embedded - dynamically allocated sub memory regions with required data encapsulated - data structures as well, which in turn is a possibility to get something wrong with memory usage as a negative impact - memory leaks, memory exhaustions etc.

All the docs and tutorials mention that you - the programmer - need to take care of the dynamically generated objects during the code runs. So, if you allocate memory for something, then it's kindly advised to be freed when that memory area is not used anymore. (***see more in free(3) manuals***)

If you plan to store some kind of objects in the linked list - which were dynamically created before storing them in the list -, it's recommended to attach a custom function - node destroy function - to handle node elimination when user would like to remove a node from the list, or when user decides to remove the entire list itself. Practically the function frees all memory allocated at node creation time. *See below the code snippet from **ll.h***

```C
    typedef struct _llist List_t;
    struct _llist {
        size_t size;                // size of the list
        Node_t *head;               // pointer to the first element of the list
        pthread_rwlock_t mtx;       // mutex for thread safety
        gen_func_t teardown;        // a function that is called every time a 
                                    // value is deleted with a pointer to that value
        gen_func_t dumper;          // a function that can print the values in a linked list
    };
```

The ***gen_func_t** teardown* function deals with nodes as such - *common type of node data will be destroyed* - ***freed*** - by this function. 

But what if the list is intended to be dealing with various types of nodes - either scalars, or other exotic structures even another linked lists?

I added extra **destroy**/***teardown*** function for the node type - the list element type in the linked list -, to be able to handle node data specific cleanup processes. *see below*:

```C
    // NODE TYPE
    typedef struct _node Node_t;
    struct _node {
        struct _node *next;
        void *data;
        pthread_rwlock_t mtx;       // rw mutex
        gen_func_t destroy;         // node data specific destructor function
    };
```

So, next time if the linked list wrapper attempt to store different type of data in the node list, then later on it can trigger node specific destructor if needed, and of course it is written and attached to the node at creation time.

```C
    ...
    if (node->destroy) {
        node->destroy(node->data);
    } else {
        list->teardown(node->data);
    }
    ...
```