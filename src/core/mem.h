#pragma once

#include <stddef.h>

void *malloc_or_exit(size_t byte);
void *realloc_or_exit(void *ptr, size_t byte);