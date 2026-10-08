#define _XOPEN_SOURCE 700
#include "ls.h"
#include "sort.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>
#include <limits.h>
#include <sys/stat.h>

int opt_A,opt_a,opt_c,opt_d,opt_F,opt_f,opt_h,opt_i,opt_k,opt_l,opt_n,opt_q,opt_R,opt_r;
int opt_S,opt_s,opt_t,opt_u,opt_w;

static int visible(const char *n) {
    if (n[0] != '.') return 1;
    if (opt_a) return 1;
    if (opt_A && strcmp(n,".") && strcmp(n,"..")) return 1;
    return 0;
}
static char *joinpath(const char *a,const char *b) {
    size_t n=strlen(a)+strlen(b)+2; char *p=malloc(n);
    if(!p) { perror("malloc"); exit(1); }
    snprintf(p,n,"%s%s%s",a, (a[0] && a[strlen(a)-1]=='/')?"":"/", b);
    return p;
}
static void free_files(FileInfo *f,size_t n){ for(size_t i=0;i<n;i++){free(f[i].name);free(f[i].path);} free(f); }

static void widths(FileInfo *f,size_t n,size_t *wi,size_t *wb,size_t *wn,size_t *ws){
    *wi=*wb=*wn=*ws=1;
    for(size_t i=0;i<n;i++){
        char b[64];
        snprintf(b,sizeof(b),"%llu",(unsigned long long)f[i].st.st_ino); size_t z=strlen(b); if(z>*wi)*wi=z;
        snprintf(b,sizeof(b),"%llu",(unsigned long long)f[i].st.st_blocks); z=strlen(b); if(z>*wb)*wb=z;
        snprintf(b,sizeof(b),"%lu",(unsigned long)f[i].st.st_nlink); z=strlen(b); if(z>*wn)*wn=z;
        if(!opt_h){snprintf(b,sizeof(b),"%llu",(unsigned long long)f[i].st.st_size);z=strlen(b);if(z>*ws)*ws=z;}
    }
}
static void list_dir(const char *path,int multiple,int depth){
    DIR *d=opendir(path);
    if(!d){fprintf(stderr,"ls: %s: %s\n",path,strerror(errno)); return;}
    FileInfo *arr=NULL; size_t n=0,cap=0; struct dirent *de;
    while((de=readdir(d))){
        if(!visible(de->d_name)) continue;
        if(n==cap){cap=cap?cap*2:32;arr=realloc(arr,cap*sizeof(*arr));if(!arr){perror("realloc");closedir(d);exit(1);}}
        arr[n].name=strdup(de->d_name); arr[n].path=joinpath(path,de->d_name);
        int rc=(opt_l||S_ISLNK(de->d_type))?lstat(arr[n].path,&arr[n].st):stat(arr[n].path,&arr[n].st);
        if(rc<0){fprintf(stderr,"ls: %s: %s\n",arr[n].path,strerror(errno));free(arr[n].name);free(arr[n].path);continue;}
        n++;
    }
    closedir(d);
    if(!opt_f){ if(opt_S) qsort(arr,n,sizeof(*arr),cmp_size); else if(opt_t) qsort(arr,n,sizeof(*arr),cmp_time); else qsort(arr,n,sizeof(*arr),cmp_name); }
    if(opt_r && n>1){for(size_t i=0,j=n-1;i<j;i++,j--){FileInfo t=arr[i];arr[i]=arr[j];arr[j]=t;}}
    if(multiple || depth>0) printf("%s:\n",path);
    size_t wi,wb,wn,ws; widths(arr,n,&wi,&wb,&wn,&ws);
    if(opt_s && n && isatty(STDOUT_FILENO)){unsigned long long total=0;for(size_t i=0;i<n;i++)total+=arr[i].st.st_blocks;printf("total %llu\n",total);}
    for(size_t i=0;i<n;i++) print_file(&arr[i],wi,wb,wn,ws);
    if(opt_R){
        for(size_t i=0;i<n;i++) if(S_ISDIR(arr[i].st.st_mode) && strcmp(arr[i].name,".") && strcmp(arr[i].name,"..")){
            printf("\n"); list_dir(arr[i].path,multiple,depth+1);
        }
    }
    free_files(arr,n);
}
void list_path(const char *path,int multiple,int depth){
    struct stat st;
    if(lstat(path,&st)<0){fprintf(stderr,"ls: %s: %s\n",path,strerror(errno));return;}
    if(opt_d || !S_ISDIR(st.st_mode)){
        FileInfo f={strdup(path),strdup(path),st}; size_t wi,wb,wn,ws; widths(&f,1,&wi,&wb,&wn,&ws);
        print_file(&f,wi,wb,wn,ws); free(f.name);free(f.path); return;
    }
    list_dir(path,multiple,depth);
}
static int isdir_operand(const char *p){struct stat s;return lstat(p,&s)==0 && S_ISDIR(s.st_mode) && !opt_d;}
int main(int argc,char **argv){
    int c;
    while((c=getopt(argc,argv,"AacdFfhiklnqRrSstuw"))!=-1){
        switch(c){
            case 'A':opt_A=1;break; case 'a':opt_a=1;break; case 'c':opt_c=1;break;
            case 'd':opt_d=1;break; case 'F':opt_F=1;break; case 'f':opt_f=1;break;
            case 'h':opt_h=1;opt_k=0;break; case 'i':opt_i=1;break; case 'k':opt_k=1;opt_h=0;break;
            case 'l':opt_l=1;break; case 'n':opt_n=1;opt_l=1;break; case 'q':opt_q=1;opt_w=0;break;
            case 'R':opt_R=1;break; case 'r':opt_r=1;break; case 'S':opt_S=1;break; case 's':opt_s=1;break;
            case 't':opt_t=1;break; case 'u':opt_u=1;break; case 'w':opt_w=1;opt_q=0;break;
            default: fprintf(stderr,"usage: ls [-AacdFfhiklnqRrSstuw] [file ...]\n"); return 1;
        }
    }
    if(opt_f) { opt_S=opt_t=opt_r=0; }
    int n=argc-optind;
    if(n==0){list_path(".",0,0);return 0;}
    int multi=n>1;
    for(int i=optind;i<argc;i++){ if(i>optind && isdir_operand(argv[i])) putchar('\n'); list_path(argv[i],multi,0); }
    return 0;
}
