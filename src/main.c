#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include "ll.h"

void num_teardown(void *n) {
    *(int *)n *= -1; // just so we can visually inspect removals afterwards
}

void num_printer(void *n) {
    printf(" %d", *(int *)n);
}

bool num_equals_3(void *n) {
    return *(int *)n == 3;
}

int main() {
    int *_n; // for storing returned ones
    int test_count = 1;
    int fail_count = 0;
    int a = 0;
    int b = 1;
    int c = 2;
    int d = 3;
    int e = 4;
    int f = 5;
    int g = 6;
    int h = 3;
    int i = 3;
    // Initialize
    List_t *list = nsIODELList.create(num_teardown);
    list->dumper = num_printer;
    // insert node
    nsIODELList.insert_node_first(list, &c, num_teardown, num_printer); // 2 in front
    // get node
    _n = (int *)nsIODELList.get_node_first(list);
    // test 1
    if (!(*_n == c)) {
        fprintf(stderr, "FAIL Test %d: Expected %d, but got %d.\n", test_count, c, *_n);
        fail_count++;
    } else
        fprintf(stderr, "PASS Test %d!\n", test_count);
    test_count++;
    // check list length
    if (list->size != 1) {
        fprintf(stderr, "FAIL Test %d: Expected %d, but got %zu.\n", test_count, 1, list->size);
        fail_count++;
    } else
        fprintf(stderr, "PASS Test %d!\n", test_count);
    test_count++;
    // insert first test
    nsIODELList.insert_node_first(list, &b, num_teardown, num_printer); // 1 in front
    nsIODELList.insert_node_first(list, &a, num_teardown, num_printer); // 0 in front -> 0, 1, 2
    _n = (int *)nsIODELList.get_node_first(list);
    if (!(*_n == a)) {
        fprintf(stderr, "FAIL Test %d: Expected %d, but got %d.\n", test_count, a, *_n);
        fail_count++;
    } else
        fprintf(stderr, "PASS Test %d!\n", test_count);
    test_count++;

    if (!(list->size == 3)) {
        fprintf(stderr, "FAIL Test %d: Expected %d, but got %zu.\n", test_count, 3, list->size);
        fail_count++;
    } else
        fprintf(stderr, "PASS Test %d!\n", test_count);
    test_count++;
    // insert nodes at end
    nsIODELList.insert_node_last(list, &d, num_teardown, num_printer); // 3 in back
    nsIODELList.insert_node_last(list, &e, num_teardown, num_printer); // 4 in back
    nsIODELList.insert_node_last(list, &f, num_teardown, num_printer); // 5 in back
    _n = (int *)nsIODELList.get_node_index(list, 5);
    if (!(*_n == f)) {
        fprintf(stderr, "FAIL Test %d: Expected %d, but got %d.\n", test_count, f, *_n);
        fail_count++;
    } else
        fprintf(stderr, "PASS Test %d!\n", test_count);
    test_count++;

    if (!(list->size == 6)) {
        fprintf(stderr, "FAIL Test %d: Expected %d, but got %zu.\n", test_count, 6, list->size);
        fail_count++;
    } else
        fprintf(stderr, "PASS Test %d!\n", test_count);
    test_count++;
    // rewrite data at index
    nsIODELList.insert_node_index(list, &g, 6, num_teardown, num_printer); // 6 at index 6 -> 0, 1, 2, 3, 4, 5, 6
    int _i;
    for (_i = 0; _i < (int)list->size; _i++) { // O(n^2) test lol
        _n = (int *)nsIODELList.get_node_index(list, _i);
        if (!(*_n == _i)) {
            fail_count++;
            fprintf(stderr, "FAIL Test %d: Expected %d, but got %d.\n", 1, _i, *_n);
        } else
            fprintf(stderr, "PASS Test %d!\n", test_count);
        test_count++;
    }
    // dump now
    nsIODELList.dump(list);
    // remove first
    nsIODELList.remove_node_first(list);                // (LL: 1 2 3 4 5 6), length: 6
    nsIODELList.dump(list);
    // 
    nsIODELList.remove_node_index(list, 1);             // (LL: 1 3 4 5 6),   length: 5
    nsIODELList.dump(list);
    // 
    nsIODELList.remove_node_index(list, 2);             // (LL: 1 3 5 6),     length: 4
    nsIODELList.dump(list);
    // 
    nsIODELList.remove_node_index(list, 5);             // (LL: 1 3 5 6),     length: 4; does nothing
    nsIODELList.dump(list);
    // 
    nsIODELList.remove_node_search(list, num_equals_3); // (LL: 1 5 6),       length: 3
    nsIODELList.dump(list);
    // 
    nsIODELList.remove_node_search(list, num_equals_3); // (LL: 1 5 6),       length: 3
    nsIODELList.dump(list);
    // 
    nsIODELList.insert_node_first(list, &h);            // (LL: 3 1 5 6),     length: 5
    nsIODELList.dump(list);
    // 
    nsIODELList.insert_node_last(list, &i);             // (LL: 3 1 5 6 3),   length: 5
    nsIODELList.dump(list);
    // 
    nsIODELList.remove_node_search(list, num_equals_3); // (LL: 1 5 6 3),     length: 4
    nsIODELList.dump(list);
    // 
    nsIODELList.remove_node_search(list, num_equals_3); // (LL: 1 5 6),     length: 3
    nsIODELList.dump(list);
    // destroy list
    nsIODELList.destroy(list);
    if (fail_count) {
        fprintf(stderr, "FAILED %d tests of %d.\n", fail_count, test_count);
        return fail_count;
    }

    fprintf(stderr, "PASSED all %d tests!\n", test_count);
    return 0;
}