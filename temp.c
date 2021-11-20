#include <stdio.h>

struct sdshdr {
    int len;
    int free;
    char buf[];
};



int main() {
    int lenth = sizeof(struct sdshdr);
    printf("%d",lenth);
    return 0;
}