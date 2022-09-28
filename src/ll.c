#include "ll.h"
#include <stdbool.h>
/**
 * @name   create
 * @note   create and initialize linked list
 * @param  gen_func_t val_teardown: generic function to destroy list elements if their have no self destructor functions emmbedded into the list item
 * @retval 
 */
List_t* create(gen_func_t destroy) {
    List_t* list = (List_t*)calloc(1, sizeof(List_t));
    list->head = NULL;
    list->size = 0;
    list->destroy = destroy;
    pthread_rwlock_init(&list->mtx, NULL);
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
    RWLOCK(l_write, list->mtx);
    while (node != NULL) {
        RWLOCK(l_write, node->mtx);
        if (node->destroy) {
            node->destroy(node->data);
        } else {
            list->destroy(node->data);
        }
        RWUNLOCK(node->mtx);
        tmp = node;
        node = node->next;
        pthread_rwlock_destroy(&(tmp->mtx));
        // free(tmp);
        SafeFree(tmp);
        (list->size)--;
    }
    list->head = NULL;
    list->destroy = NULL;
    list->dumper = NULL;
    RWUNLOCK(list->mtx);
    pthread_rwlock_destroy(&(list->mtx));
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
    RWLOCK(lt, (*node)->mtx);
    Node_t *last;
    for (size_t i = n; i > 1; i--) {
        last = *node;
        *node = last->next;
        if (*node == NULL) { // happens when another thread deletes the end of a list
            RWUNLOCK(last->mtx);
            return -1;
        }
        RWLOCK(lt, (*node)->mtx);
        RWUNLOCK(last->mtx);
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
int insert_node_index(List_t* list, void *data, size_t index, gen_func_t destroy, gen_func_t dumper) {
    Node_t *node = (Node_t *)malloc(sizeof(Node_t));
    node->data = data;
    if (destroy) {
        node->destroy = destroy;
    }
    if (dumper) {
        node->dumper = dumper;
    }
    node->next = NULL;
    pthread_rwlock_init(&node->mtx, NULL);
    if (index == 0) {
        RWLOCK(l_write, list->mtx);
        node->next = list->head;
        list->head = node;
        RWUNLOCK(list->mtx);
    } else {
        Node_t *nth_node;
        if (ll_select_n_min_1(list, &nth_node, index, l_write)) {
            SafeFree(node);
            return -1;
        }
        node->next = nth_node->next;
        nth_node->next = node;
        RWUNLOCK(nth_node->mtx);
    }
    RWLOCK(l_write, list->mtx);
    (list->size)++;
    RWUNLOCK(list->mtx);
    return list->size; 
}
/**
 * @name   insert_node_first
 * @note   insert new node to the linked list head
 * @param  List_t* list: linked list
 * @param  void* val: data to be inserted into the list
 * @retval on error -1, otherwise the new size of the linked list
 */
int insert_node_first(List_t *list, void *val, gen_func_t destroy, gen_func_t dumper) {
    return insert_node_index(list, val, 0, destroy, dumper);
}
/**
 * @name   insert_node_last
 * @note   inserts new node at the end of the linked list
 * @param  List_t* list: linked list
 * @param  void *val: data to be inserted into the linked list
 * @retval on error -1, otherwise the new size of the linked list
 */
int insert_node_last(List_t *list, void *val, gen_func_t destroy, gen_func_t dumper) {
    return insert_node_index(list, val, list->size, destroy, dumper);
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
        RWLOCK(l_write, list->mtx);
        tmp = list->head;
        list->head = tmp->next;
    } else {
        Node_t *nth_node;
        if (ll_select_n_min_1(list, &nth_node, n, l_write)) // if that node doesn't exist
            return -1;
        tmp = nth_node->next;
        nth_node->next = nth_node->next == NULL ? NULL : nth_node->next->next;
        RWUNLOCK(nth_node->mtx);
        RWLOCK(l_write, list->mtx);
    }
    (list->size)--;
    RWUNLOCK(list->mtx);
    if (tmp->destroy) {
        tmp->destroy(tmp->data);
    } else {
        list->destroy(tmp->data);
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
int remove_node_search(List_t *list, condition_func_t cond, void *filter) {
    Node_t *last = NULL;
    Node_t *node = list->head;
    while ((node != NULL) && !(cond(node->data, filter))) {
        last = node;
        node = node->next;
    }
    if (node == NULL) {
        return -1;
    } else if (node == list->head) {
        RWLOCK(l_write, list->mtx);
        list->head = node->next;
        RWUNLOCK(list->mtx);
    } else {
        RWLOCK(l_write, last->mtx);
        last->next = node->next;
        RWUNLOCK(last->mtx);
    }
    if (node->destroy) {
        node->destroy(node->data);
    } else {
        list->destroy(node->data);
    }
    SafeFree(node);
    RWLOCK(l_write, list->mtx);
    (list->size)--;
    RWUNLOCK(list->mtx);
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

    RWUNLOCK(node->mtx);
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
        RWLOCK(l_read, node->mtx);
        fn(node->data);
        Node_t *old_node = node;
        node = node->next;
        RWUNLOCK(old_node->mtx);
    }
}
/**
 * @name   dump
 * @note   dumps out - output stream - the contents of the linked list
 * @param  List_t list: the linked list !!! NOTE: this is the instance, not a pointer to the list !!!
 * @retval None
 */
void dump(List_t *list) {
    if (NULL == list) return;
    if (list->size == 0) return;
    Node_t *node = list->head;
    while (node != NULL) {
        RWLOCK(l_read, node->mtx);
        if (node->dumper) {
            node->dumper(node->data);
        } else {
            if (list->dumper) {
                list->dumper(node->data);
            }
        }
        Node_t *old_node = node;
        node = node->next;
        RWUNLOCK(old_node->mtx);
    }
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
