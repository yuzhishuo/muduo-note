# IP 地址转换

`#include <arpa/inet.h>`

你想 把你的 IP 存储到 ina 中。你可以使用的函数： `in_addr_t inet_addr(const char* strptr)`;

1. 返回的地址已经是网络字节顺序了， 
2. 错误检查, 如果 `inet_addr` 函数 执行错误，它将会返回 –1⋯⋯等等！(二进制的无符号整数值 –1 相当于 255.255.255.255 !! 一 个广播用的 IP 地址！)
3. 如果你有一个 `struct in_addr` 并且你想把它代表的 IP 地址打印出来: `char* inet_ntoa(struct in_addr )` `inet_ntoa`返回一个字符指针，它指向一个定义在函数 `inet_ntoa()` 中的 `static`类型字符串。所以每次你调用 `inet_ntoa()`，都会改变最后一次调用 `inet_ntoa()` 函数时得到的结果。 