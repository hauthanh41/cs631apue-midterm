#ifndef LS_H
#define LS_H

#include <sys/stat.h>
#include <stddef.h>

typedef struct {
    char *name;
    char *path;
    struct stat st;
} FileInfo;

extern int opt_A, opt_a, opt_c, opt_d, opt_F, opt_f, opt_h;
extern int opt_i, opt_k, opt_l, opt_n, opt_q, opt_R, opt_r;
extern int opt_S, opt_s, opt_t, opt_u, opt_w;

void list_path(const char *path, int multiple, int depth);
void print_file(FileInfo *f, size_t width_inode, size_t width_blocks,
                size_t width_nlink, size_t width_size);
char type_suffix(mode_t mode);

#endif
