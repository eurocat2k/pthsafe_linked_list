#include "ll.h"
#include <stdbool.h>
/**
 * @name   create
 * @note   create and initialize linked list
 * @param  gen_func_t teardown: generic function to destroy list elements if their have no self destructor functions emmbedded into the list item
 * @retval 
 */
List_t* create(gen_func_t teardown) {
    List_t* list = (List_t*)calloc(1, sizeof(List_t));
    list->head = NULL;
    list->size = 0;
    list->teardown = teardown;
    // pthread_rwlock_init(&list->mtx, NULL);
#ifdef HAVE_LIBPTHREAD
    list->mtx = PTHREAD_RWLOCK_INITIALIZER;
#endif
    return list;
}
/**
 * @name   destroy
 * @note   destroys linked list
 * @param  List_t* list: the linked list to be destroyed
 * @retval None
 */
void destroy(List_t *list) {
    Node_t *node = list->head;
    Node_t *tmp;
#ifdef HAVE_LIBPTHREAD
    RWLOCK(l_write, list->mtx);
#endif
    while (node != NULL) {
#ifdef HAVE_LIBPTHREAD
        RWLOCK(l_write, node->mtx);
#endif
        if (node->destroy) {
            node->destroy(node->data);
        } else {
            list->teardown(node->data);
        }
#ifdef HAVE_LIBPTHREAD
        RWUNLOCK(node->mtx);
#endif
        tmp = node;
        node = node->next;
#ifdef HAVE_LIBPTHREAD
        pthread_rwlock_destroy(&(tmp->mtx));
#endif
        // free(tmp);
        SafeFree(tmp);
        (list->size)--;
    }
    list->head = NULL;
    list->teardown = NULL;
    list->dumper = NULL;
#ifdef HAVE_LIBPTHREAD
    RWUNLOCK(list->mtx);
    pthread_rwlock_destroy(&(list->mtx));
#endif
    // free(list);
    SafeFree(list);
}
/**
 * @name   ll_select_n_min_1
 * @note Actually selects the n - 1th element. Inserting and deleting at the front of a
 * list do NOT really depend on this.
 * @param  list - the linked list
 * @param  node - a pointer to set when the node is found
 * @param  n - the index
 * @retval 0 if successful, -1 otherwise
 */
int ll_select_n_min_1(List_t *list, Node_t **node, size_t n, locktype_t lt) {
    if (n == 0) return 0;
    // n > 0
    *node = list->head;
    if (*node == NULL) // if head is NULL, but we're trying to go past it,
        return -1;     // we have a problem
#ifdef HAVE_LIBPTHREAD
    RWLOCK(lt, (*node)->mtx);
#endif
    Node_t *last;
    for (size_t i = n; i > 1; i--) {
        last = *node;
        *node = last->next;
        if (*node == NULL) { // happens when another thread deletes the end of a list
#ifdef HAVE_LIBPTHREAD
            RWUNLOCK(last->mtx);
#endif
            return -1;
        }
#ifdef HAVE_LIBPTHREAD
        RWLOCK(lt, (*node)->mtx);
        RWUNLOCK(last->mtx);
#endif
    }
    return 0;
}
/**
 * @name   insert_node_index
 * @note   Inserts a value at the nth position of a linked list
 * @param  List_t* list: linked list
 * @param  void* data: data to be inserted into the linked list
 * @param  size_t index: index of the linked list the new data to be inserted
 * @retval 
 */
int insert_node_index(List_t* list, void *data, size_t index) {
    Node_t *node = (Node_t *)malloc(sizeof(Node_t));
    node->data = data;
    node->next = NULL;
#ifdef HAVE_LIBPTHREAD
    node->mtx = PTHREAD_RWLOCK_INITIALIZER;
#endif
    if (index == 0) {
#ifdef HAVE_LIBPTHREAD
        RWLOCK(l_write, list->mtx);
#endif
        node->next = list->head;
        list->head = node;
#ifdef HAVE_LIBPTHREAD
        RWUNLOCK(list->mtx);
#endif
    } else {
        Node_t *nth_node;
        if (ll_select_n_min_1(list, &nth_node, index, l_write)) {
            SafeFree(node);
            return -1;
        }
        node->next = nth_node->next;
        nth_node->next = node;
#ifdef HAVE_LIBPTHREAD
        RWUNLOCK(nth_node->mtx);
#endif
    }
#ifdef HAVE_LIBPTHREAD
    RWLOCK(l_write, list->mtx);
#endif
    (list->size)++;
#ifdef HAVE_LIBPTHREAD
    RWUNLOCK(list->mtx);
#endif
    return list->size; 
}
/**
 * @name   insert_node_first
 * @note   insert new node to the linked list head
 * @param  List_t* list: linked list
 * @param  void* val: data to be inserted into the list
 * @retval on error -1, otherwise the new size of the linked list
 */
int insert_node_first(List_t *list, void *val) {
    return insert_node_index(list, val, 0);
}
/**
 * @name   insert_node_last
 * @note   inserts new node at the end of the linked list
 * @param  List_t* list: linked list
 * @param  void *val: data to be inserted into the linked list
 * @retval on error -1, otherwise the new size of the linked list
 */
int insert_node_last(List_t *list, void *val) {
    return insert_node_index(list, val, list->size);
}
/**
 * @name   remove_node_index
 * @note   removes node from linked list at index
 * @param  List_t* list: linked list
 * @param  size_t n: index
 * @retval on error -1, otherwise the new size of the linked list
 */
int remove_node_index(List_t *list, size_t n) {
    Node_t *tmp;
    if (n == 0) {
#ifdef HAVE_LIBPTHREAD
        RWLOCK(l_write, list->mtx);
#endif
        tmp = list->head;
        list->head = tmp->next;
    } else {
        Node_t *nth_node;
        if (ll_select_n_min_1(list, &nth_node, n, l_write)) // if that node doesn't exist
            return -1;
        tmp = nth_node->next;
        nth_node->next = nth_node->next == NULL ? NULL : nth_node->next->next;
#ifdef HAVE_LIBPTHREAD
        RWUNLOCK(nth_node->mtx);
        RWLOCK(l_write, list->mtx);
#endif
    }
    (list->size)--;
#ifdef HAVE_LIBPTHREAD
    RWUNLOCK(list->mtx);
#endif
    if (tmp->destroy) {
        tmp->destroy(tmp->data);
    } else {
        list->teardown(tmp->data);
    }
    SafeFree(tmp);
    return list->size;
}
/**
 * @name   remove_node_first
 * @note   removes node from linked list head
 * @param  List_t* list: linked list
 * @retval on error -1, otherwise the new size of the linked list
 */
int remove_node_first(List_t *list) {
    return remove_node_index(list, 0);
}
/**
 * @name   remove_node_search
 * @note   removes node from linked list if cond function - processing the node data - returns true
 * @param  *list: 
 * @param  *: 
 * @retval 
 */
int remove_node_search(List_t *list, condition_func_t cond) {
    Node_t *last = NULL;
    Node_t *node = list->head;
    while ((node != NULL) && !(cond(node->data))) {
        last = node;
        node = node->next;
    }
    if (node == NULL) {
        return -1;
    } else if (node == list->head) {
#ifdef HAVE_LIBPTHREAD
        RWLOCK(l_write, list->mtx);
#endif
        list->head = node->next;
#ifdef HAVE_LIBPTHREAD
        RWUNLOCK(list->mtx);
#endif
    } else {
#ifdef HAVE_LIBPTHREAD
        RWLOCK(l_write, last->mtx);
#endif
        last->next = node->next;
#ifdef HAVE_LIBPTHREAD
        RWUNLOCK(last->mtx);
#endif
    }
    if (node->destroy) {
        node->destroy(node->data);
    } else {
        list->teardown(node->data);
    }
    SafeFree(node);
#ifdef HAVE_LIBPTHREAD
    RWLOCK(l_write, list->mtx);
#endif
    (list->size)--;
#ifdef HAVE_LIBPTHREAD
    RWUNLOCK(list->mtx);
#endif
    return list->size;
}
/**
 * @name   get_node_index
 * @note   returns node from linked list at index
 * @param  List_t* list: linked list
 * @param  size_t n: index
 * @retval Node_t *node
 */
void *get_node_index(List_t *list, size_t n) {
    Node_t *node;
    if (ll_select_n_min_1(list, &node, n + 1, l_read))
        return NULL;
#ifdef HAVE_LIBPTHREAD
    RWUNLOCK(node->mtx);
#endif
    return node->data;
}
/**
 * @name   get_node_index
 * @note   returns node from linked list at index 0
 * @param  List_t* list: linked list
 * @retval Node_t *node
 */
void *get_node_first(List_t *list) {
    return get_node_index(list, 0);
}
/**
 * @name   map
 * @note   calls a function on the value of every element of a linked list
 * @param  List_t* list: linked list
 * @param  gen_func_t fn: the function executed with its arguments - if any - on each node
 * @retval None
 */
void map(List_t *list, gen_func_t fn) {
    Node_t *node = list->head;
    while (node != NULL) {
#ifdef HAVE_LIBPTHREAD
        RWLOCK(l_read, node->mtx);
#endif
        fn(node->data);
        Node_t *old_node = node;
        node = node->next;
#ifdef HAVE_LIBPTHREAD
        RWUNLOCK(old_node->mtx);
#endif
    }
}
/**
 * @name   dump
 * @note   dumps out - output stream - the contents of the linked list
 * @param  List_t list: the linked list !!! NOTE: this is the instance, not a pointer to the list !!!
 * @retval None
 */
void dump(List_t list) {
    if (list.dumper == NULL)
        return;
    printf("(LIST:\n");
    map(&list, list.dumper);
    printf("), length: %zu\n", list.size);
}
/**
 * @name   no_node_teardown
 * @note   do nothing function
 * @param  void *n: dummy data
 * @retval None
 */
void no_node_teardown(void *n) {
    return;
}
/**
 * @name   nsIODELList
 * @note   namespaced object of all the methods above, see struct _nsllist declaration
 */
struct _nsllist nsIODELList = {
    create,
    destroy,
    insert_node_index,
    insert_node_first,
    insert_node_last,
    remove_node_index,
    remove_node_first,
    remove_node_search,
    get_node_index,
    get_node_first,
    map,
    dump,
    no_node_teardown
};
