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

常数复杂度获取字符串长度，可以直接通过一个len的值获取

杜绝缓冲区溢出
