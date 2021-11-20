## Redis
### data-struct
#### SDS

每个`sds.h/sdshdr` 结构表示一个SDS值：

```c
strcut sdshdr {
    // 记录 buf 数组中已使用字节的数量
    // 等于 SDS 所保存字符串的长度
    int len;

    // 记录 buf 数组中未使用字节的数量
    int free;

    // 字节数组，用于保存字符串
    char buf[];
}
```

值得注意的是在存储中len中是不把\0算在内的，是自动添加的。

##### SDS 与 C 字符串的区别

* 常数复杂度获取字符串长度，可以直接通过一个len的值获取

* 杜绝缓冲区溢出  在修改内容的时候传统的C语言的字符串是存在溢出的可能的，但是在SDS中是存在一个len的值进行修改，在自带的API中sdcat中会自动检查拼写。
* 减少修改字符串时带来的内存重新分配次数   
  *  空间预分配 用于优化SDS的字符串增长操作：当SDS的API对一个SDS进行修改，并且需要对SDS进行空间扩展的时候，不但会为SDS预先分配修改所必须要的空间，还会为SDS分配额外的未使用的空间 
    * 如果对 SDS 进行修改之后， SDS 的长度（也即是 `len` 属性的值）将小于 `1 MB` ， 那么程序分配和 `len` 属性同样大小的未使用空间， 这时 SDS `len` 属性的值将和 `free` 属性的值相同。 举个例子， 如果进行修改之后， SDS 的 `len` 将变成 `13` 字节， 那么程序也会分配 `13` 字节的未使用空间， SDS 的 `buf` 数组的实际长度将变成 `13 + 13 + 1 = 27` 字节（额外的一字节用于保存空字符）。
    * 如果对 SDS 进行修改之后， SDS 的长度将大于等于 `1 MB` ， 那么程序会分配 `1 MB` 的未使用空间。 举个例子， 如果进行修改之后， SDS 的 `len` 将变成 `30 MB` ， 那么程序会分配 `1 MB` 的未使用空间， SDS 的 `buf` 数组的实际长度将为 `30 MB + 1 MB + 1 byte` 。
  * 惰性空间释放 惰性空间释放用于优化SDS的字符串缩短操作：当 SDS 的 API 需要缩短 SDS 保存的字符串时， 程序并不立即使用内存重分配来回收缩短后多出来的字节， 而是使用 `free` 属性将这些字节的数量记录起来， 并等待将来使用。
* 二进制安全 因为有了len这个值于是支持了SDS可以存储二进制的数据。
* 兼容部分C字符函数 可以使用部分c字符的库函数。

下面是阅读源码的一点收获

###### `sdsnewlen`

```c
sds sdsnewlen(const void *init, size_t initlen);
```

第一个参数其实就是传进去的字符串，第二则是类似于字符串的长度

在其中Redis对内存分配的处理方式是使用对init的值是否为空进行区分，如果不为为空就采用，`zmalloc`其中的内容，我也看了一下

[这是我发现的一个感觉讲的还算比较清晰的文章](https://blog.csdn.net/guodongxiaren/article/details/44747719)

在这篇文章里面，我发现这个`zmalloc` 函数实际上是提供了一种自定义内存管理函数处理8字节对齐的设计思路和实现原理。

所谓字长就是CPU一次能读取数据的二进制位数，对齐起到作用主要是可以缩短需要的总线周期

下面是`zmalloc`

```c
void *ptr = malloc(size+PREFIX_sIZE);
```

其中size是我们需要分配的内存大小，多分配的八个是用来存储size的值。

如果分配失败就进行错误打印并且终止程序

下面就是宏的条件编译

其中的`update_zmalloc_stat_alloc` 则是提供了一个向右对齐的作用，让内存分配变成8的倍数。里面的位操作有点奇特。

下面就是一些基本的初始化的工作，我想我应该是看懂了

###### `sdsnew()`

这里面就是传参的时候没有传入长度，然后在函数内部进行处理，最后的出传入参数的长度，然后再返回的时候再次调用`sdsnewlen()`

###### `sdslen`

在这个函数中，传入的是一个指向sdshdr的指针，这是我们获取长度，就需要减去一个struct的长度，的到sdshdr结构体，然后获取len。可以类推的是free的获取方式，也是如此。

#### 链表

##### 链表的组成

```c
typedef struct listNode {

    // 前置节点
    struct listNode *prev;

    // 后置节点
    struct listNode *next;

    // 节点的值
    void *value;

} listNode;

typedef struct list {

    // 表头节点
    listNode *head;

    // 表尾节点
    listNode *tail;

    // 链表所包含的节点数量
    unsigned long len;

    // 节点值复制函数
    void *(*dup)(void *ptr);

    // 节点值释放函数
    void (*free)(void *ptr);

    // 节点值对比函数
    int (*match)(void *ptr, void *key);

} list;
```

* 双端：获取前置节点和后置节点的时间复杂度降低
* 无环：表头节点的prev和表尾节点的next都是为NULL
* 带表头指针和表尾指针
* 带链表长度计数器
* 多态

