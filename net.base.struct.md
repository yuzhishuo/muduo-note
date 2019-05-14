# SOCKET
```cpp
#include<bits/socket.h>
struct sockaddr
{
	unsigned short sa_family; /* address族, AF_xxx */ 
	char sa_data[14];		  /* 14 bytes的协议地址 */ 
};
```

1. `sa_family` AF_UNIX UNIX，UNIX域本地地址族；AF_INET, TCP/IPv4 地址族； AF_INET6, TCP/IPv6 地址族； 地址族和协议族定义相同
2. `sa_data` 包含了一些远程电脑的地址、端口和套接字的数目，它里面的数据是杂溶在一 切的。

由于`sa_data` 只有14 字节，无法存放大多数地址族的数据，因此 `linux` 提供了新的通用 `socket` 地址结构体

```cpp
#include<bits/socket.h>

struct sockaddr_storage
{
	sa_family_t sa_family;
	unsigned long int 	__ss_align;
	char __ss_padding[128 - sizeof(__ss_align)];	/* 内存对齐 */
};

```

专用的 `socket` 地址

## sockaddr_un
```cpp
#include<sys/un.h>

struct sockaddr_un
{
	sa_family_t sin_family; 	/* 地址族： AF_UNIX */
	char sun_path[108];			/* 文件路径名 */
}
```

## sockaddr_in
为了处理 `struct sockaddr`， 程序员建立了另外一个相似的结构`struct sockaddr_in`： 
```cpp
#include <netinet/in.h>
// struct sockaddr_in (“in” 代表 “Internet”) 

struct sockaddr_in
{ 
	short int sin_family; /* 地址族： AF_INET */ 
	unsigned short int sin_port; /* 端口号 令其为0时，其随机选择一个端口*/ 
	struct in_addr sin_addr; /* ipv4地址 INADDR_ANY（值不确定，0 or other[使用大小端转换]）使用自己的端口*/ 
	unsigned char sin_zero[8]; /* 添0（和struct sockaddr一样大小）*/ 
};

struct in_addr
{
	u_int32_t s_addr;	/* ipv4地址 */
};
```
1. `sin_zero[8]` 是为了是两个结构在内存中具有相同的尺寸。
2. 使用 `sockaddr_in` 的时 候要把 `sin_zero` 全部设成零值（使用 `bzero()`或 `memset()`函数）。


```cpp
#include <string.h>
extern void bzero（void *s, int n）;
```
