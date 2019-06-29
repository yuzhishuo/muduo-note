```cpp
#include <sys/socket.h> 
int listen(int sockfd, int backlog);

// success 0
// fault -1 ， set errno
```

listen()函数的参数意义如下： 
1. sockfd 是一个套接字描述符，由socket()系统调用获得。 
2. backlog 是未经过处理的连接请求队列可以容纳的最大数目。

当连接数超过 backlog ，则客户端也将 产生 ECONNREFUSE 信息

linux 2.2 core， bakclog 的数目指的是 SYN_RCVD（办理按揭 ESTABLISHED（连接状态，2.2 之后特指完全连接状态。

处于 未连接的 socket 由 `/proc/sys/net/ipv4/tcp_max_syn_backlog` 内核参数定义

在每一个连入请求都要进入一个连入请求队列，等待 listen 的程序调用 accept()（accept()函数下面有介绍）函数来接受这个连接。
当系统还没有 调用 accept()函数的时候，如果有很多连接，那么本地能够等待的最大数目就是 backlog 的 数值。你可以将其设成 5 到 10之间的数值（推荐）。 

```cpp
#include<sys/types.h>
#include<sys/socket.h>
int accept(int sockfd, void *addr, int *addrlen); 
```
accept()函数的参数意义如下： 
1. sockfd 是正在 listen() 的一个套接字描述符。 
2. addr 一般是一个指向 struct sockaddr_in 结构的指针；
	里面存储着远程连接过来的 计算机的信息（比如远程计算机的 IP 地址和端口）。 
3. addrlen 是一个本地的整型数值，在它的地址传给 accept()前它的值应该是 sizeof(struct sockaddr_in)；accept()不会在 addr 中存储多余 addrlen bytes 大小的数据。如果 accept()函数在 addr 中存储的数据量不足 addrlen，则 accept()函数会改变 addrlen 的值来反 应这个情况。 