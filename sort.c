#include "sort.h"
#include <string.h>
#include <time.h>

int cmp_name(const void *a, const void *b) {
    const FileInfo *x=a,*y=b;
    return strcmp(x->name,y->name);
}
int cmp_size(const void *a, const void *b) {
    const FileInfo *x=a,*y=b;
    if (x->st.st_size < y->st.st_size) return 1;
    if (x->st.st_size > y->st.st_size) return -1;
    return strcmp(x->name,y->name);
}
int cmp_time(const void *a, const void *b) {
    const FileInfo *x=a,*y=b;
    time_t tx=opt_u?x->st.st_atime:(opt_c?x->st.st_ctime:x->st.st_mtime);
    time_t ty=opt_u?y->st.st_atime:(opt_c?y->st.st_ctime:y->st.st_mtime);
    if(tx<ty) return 1;
    if(tx>ty) return -1;
    return strcmp(x->name,y->name);
}
