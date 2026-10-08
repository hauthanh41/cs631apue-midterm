#ifndef PRINT_H
#define PRINT_H
#include "ls.h"
void print_file(FileInfo *f, size_t width_inode, size_t width_blocks,
                size_t width_nlink, size_t width_size);
#endif
