#include "sf.h"
void saferFree(void **pp) {
    if (pp != NULL && *pp != NULL) {
        free(*pp);
        *pp = NULL;
    }
    return;
}
/**
 * @name   SafeFree
 * @note   wrapper frees HEAP allocated memory block
 * @param  void* p: the memory pointer
 * @retval None
 */
void SafeFree(void *p) {
    saferFree((void **)&p);
}