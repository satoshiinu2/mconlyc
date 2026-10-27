#include "core/vec.h"

int main(void) {
    VEC(int) values = VEC_INIT(int);
    VEC_PUSH(values, 10);
    VEC_PUSH(values, 20);
    VEC_PUSH(values, 30);
    VEC_PUSH(values, 40);
    size_t capacity = values.cap;
    size_t index = 1;
    VEC_SWAP_REMOVE(values, index++);
    assert(index == 2);
    assert(values.len == 3 && values.cap == capacity);
    assert(values.data[0] == 10 && values.data[1] == 40 && values.data[2] == 30);
    VEC_SWAP_REMOVE(values, 2);
    assert(values.len == 2 && values.data[1] == 40);
    VEC_SWAP_REMOVE(values, 0);
    assert(values.len == 1 && values.data[0] == 40);
    VEC_SWAP_REMOVE(values, 0);
    assert(values.len == 0 && values.cap == capacity);
    VEC_FREE(values);
    return 0;
}
