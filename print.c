#include "print.h"
#include <stdio.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <unistd.h>
#include <limits.h>
#include <string.h>
#include <sys/types.h>

static void mode_string(mode_t m, char *s) {
    s[0] = S_ISREG(m) ? '-' : S_ISDIR(m) ? 'd' : S_ISLNK(m) ? 'l' :
           S_ISCHR(m) ? 'c' : S_ISBLK(m) ? 'b' : S_ISFIFO(m) ? 'p' :
           S_ISSOCK(m) ? 's' : '?';
    s[1]=(m&S_IRUSR)?'r':'-'; s[2]=(m&S_IWUSR)?'w':'-'; s[3]=(m&S_IXUSR)?'x':'-';
    s[4]=(m&S_IRGRP)?'r':'-'; s[5]=(m&S_IWGRP)?'w':'-'; s[6]=(m&S_IXGRP)?'x':'-';
    s[7]=(m&S_IROTH)?'r':'-'; s[8]=(m&S_IWOTH)?'w':'-'; s[9]=(m&S_IXOTH)?'x':'-';
    if (m&S_ISUID) s[3]=(m&S_IXUSR)?'s':'S';
    if (m&S_ISGID) s[6]=(m&S_IXGRP)?'s':'S';
    if (m&S_ISVTX) s[9]=(m&S_IXOTH)?'t':'T';
    s[10]='\0';
}

static void human(unsigned long long n, char *buf, size_t sz) {
    const char *u="BKMGTPE";
    double x=(double)n; int i=0;
    while (x>=1024.0 && i<6) { x/=1024.0; i++; }
    if (i==0) snprintf(buf,sz,"%llu",n);
    else if (x>=10) snprintf(buf,sz,"%.0f%c",x,u[i]);
    else snprintf(buf,sz,"%.1f%c",x,u[i]);
}

char type_suffix(mode_t m) {
    if (S_ISDIR(m)) return '/';
    if (S_ISLNK(m)) return '@';
    if (S_ISFIFO(m)) return '|';
    if (S_ISSOCK(m)) return '=';
    if (S_ISREG(m) && (m & (S_IXUSR|S_IXGRP|S_IXOTH))) return '*';
    if (S_ISCHR(m) || S_ISBLK(m)) return '%';
    return '\0';
}

void print_file(FileInfo *f, size_t wi, size_t wb, size_t wn, size_t ws) {
    char namebuf[PATH_MAX + 2];
    const char *display = f->name;
    if (opt_q && !opt_w) {
        size_t i, n=strlen(display);
        if (n >= sizeof(namebuf)-1) n=sizeof(namebuf)-2;
        for (i=0;i<n;i++) namebuf[i]=(display[i] >= 32 && display[i] != 127) ? display[i] : '?';
        namebuf[n]='\0'; display=namebuf;
    }
    if (opt_i) printf("%*llu ", (int)wi, (unsigned long long)f->st.st_ino);
    if (opt_s) {
        if (opt_h) {
            char bbuf[64];
            unsigned long long bytes = (unsigned long long)f->st.st_blocks * 512ULL;
            human(bytes, bbuf, sizeof(bbuf));
            printf("%*s ", (int)wb, bbuf);
        } else if (opt_k) {
            unsigned long long kb = ((unsigned long long)f->st.st_blocks + 1ULL) / 2ULL;
            printf("%*llu ", (int)wb, kb);
        } else {
            printf("%*llu ", (int)wb, (unsigned long long)f->st.st_blocks);
        }
    }
    if (opt_l) {
        char mode[11], timebuf[64], sizebuf[64];
        mode_string(f->st.st_mode, mode);
        struct passwd *pw = opt_n ? NULL : getpwuid(f->st.st_uid);
        struct group *gr = opt_n ? NULL : getgrgid(f->st.st_gid);
        const char *owner = pw ? pw->pw_name : NULL;
        const char *group = gr ? gr->gr_name : NULL;
        if (!owner) { static char b[32]; snprintf(b,sizeof(b),"%u",(unsigned)f->st.st_uid); owner=b; }
        if (!group) { static char b[32]; snprintf(b,sizeof(b),"%u",(unsigned)f->st.st_gid); group=b; }
        unsigned long long sz=(unsigned long long)f->st.st_size;
        if (opt_h) human(sz,sizebuf,sizeof(sizebuf)); else snprintf(sizebuf,sizeof(sizebuf),"%llu",sz);
        time_t tt = opt_u ? f->st.st_atime : (opt_c ? f->st.st_ctime : f->st.st_mtime);
        struct tm *tmv=localtime(&tt);
        strftime(timebuf,sizeof(timebuf),"%b %e %H:%M",tmv);
        printf("%s %*lu %-8s %-8s %*s %s ", mode, (int)wn,(unsigned long)f->st.st_nlink,
               owner, group, (int)ws, sizebuf, timebuf);
    }
    printf("%s", display);
    if (opt_F) { char c=type_suffix(f->st.st_mode); if(c) putchar(c); }
    if (opt_l && S_ISLNK(f->st.st_mode)) {
        char link[PATH_MAX+1]; ssize_t n=readlink(f->path,link,sizeof(link)-1);
        if(n>=0){ link[n]='\0'; printf(" -> %s",link); }
    }
    putchar('\n');
}
