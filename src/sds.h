// 类型别名，用来指向 sdshdr 的 buf 属性
#include  <sys/types.h>
#include <stdarg.h>
typedef char *sds;

struct sdshdr {
    int len;
    int free;
    char buf[];
};

static inline size_t sdslen(const sds s) {
    // sizeof() 得到的是一个sdshdr的大小，而 s 是指向其中字符串的地址，减去了就是这个结构体最开始的地址 
    struct sdshdr *sh = (void*)(s-(sizeof(struct sdshdr)));
    return sh->len;
}

static inline size_t sdsavail(const sds s) {
    struct sdshdr *sh = (void*)(s-(sizeof(struct sdshdr)));
    return sh->free;    
}

sds sdsnewlen(const void *init, size_t initlen);
sds sdsnew(const char *init);