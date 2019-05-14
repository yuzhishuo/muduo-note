 ```cpp
#include <sys/types.h> 
#include <sys/socket.h>
int connect (int sockfd, struct sockaddr *serv_addr, int addrlen);
connect()的三个参数意义如下：
```
1. sockfd ：套接字文件描述符，由socket()函数返回的。 
2. serv_addr 是一个存储远程计算机的IP地址和端口信息的结构。 
3. addrlen 应该是 sizeof(struct sockaddr)。

参数说明： 
1. sockfd 是由socket()函数返回的套接字描述符。 
2. my_addr 是一个指向 struct sockaddr 的指针，包含有关你的地址的信息：名称、 端口和 IP地址。 
3. addrlen 可以设置为 sizeof(struct sockaddr)。 

一定要检测 connect()的返回值：
如果发生了错误（比如无法连接到远程主 机，或是远程主机的指定端口无法进行连接等）它将会返回错误值 –1 。 
全局变量 errno 将会存储错误代码。 
**你可以查看 connect的  [error 信息](https://blog.csdn.net/xingzhi2014/article/details/14028411)**