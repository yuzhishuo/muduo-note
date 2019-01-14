
#SOCKET
```
struct sockaddr
{
	unsigned short sa_family; /* address族, AF_xxx */ 
	char sa_data[14];		  /* 14 bytes的协议地址 */ 
};
```

1. `sa_family` 一般来说，都是 “AFINET”。 
2. `sa_data` 包含了一些远程电脑的地址、端口和套接字的数目，它里面的数据是杂溶在一 切的。

#sockaddr_in
为了处理 `struct sockaddr`， 程序员建立了另外一个相似的结构`struct sockaddr_in`： 
```
#include <netinet/in.h>
struct sockaddr_in (“in” 代表 “Internet”) 

struct sockaddr_in
{ 
	short int sin_family; /* Internet地址族 */ 
	unsigned short int sin_port; /* 端口号 网络字节顺序。 令其为0时，其随机选择一个端口 */ 
	struct in_addr sin_addr; /* Internet地址 网络字节顺序。  INADDR_ANY（值不确定，0 or other[使用大小端转换]）使用自己的端口*/ 
	unsigned char sin_zero[8]; /* 添0（和struct sockaddr一样大小）*/ 
};
```
1. `sin_zero[8]` 是为了是两个结构在内存中具有相同的尺寸。
2. 使用 `sockaddr_in` 的时 候要把 `sin_zero` 全部设成零值（使用 `bzero()`或 `memset()`函数）。


```
#include <string.h>
extern void bzero（void *s, int n）;

struct in_addr 其定义如下： /* 因特网地址 (a structure for historical reasons) */

struct in_addr
{
	unsigned long s_addr;
};
```

#IP 地址转换

`#include <arpa/inet.h>`

你想 把你的 IP 存储到 ina 中。你可以使用的函数： `inet_addr("char[]")`;

1. 返回的地址已经是网络字节顺序了， 
2. 错误检查, 如果 inet_addr() 函数 执行错误，它将会返回 –1⋯⋯等等！(二进制的无符号整数值 –1 相当于 255.255.255.255 !! 一 个广播用的 IP 地址！)
如果你有一个 `struct in_addr` 并且你想把它代表的 IP 地址打印出来: `inet_ntoa(struct in_addr )`
3. `inet_ntoa()`返回一个字符指针，它指向一个定义在函数 `inet_ntoa()` 中的 `static`类型字符串。所以每次你调用 `inet_ntoa()`，都会改变最后一次调用 `inet_ntoa()` 函数时得到的结果。 

```
	#include <sys/types.h>
	#include <sys/socket.h>
	int socket（int domain , int type , int protocol）;
	
	domain = AF_INET
	type = SOCK_STREAM or SOCK_DGRAM
```
	1. 如果发生错误， socket()函数返回 –1 。全局变量 errno 将被设置为错误代码。
	2. 最后， 只需要把 protocol设置为 0 。

```
#include <sys/types.h>
#include <sys/socket.h>
int bind (int sockfd , struct sockaddr *my_addr , int addrlen);
```
1. bind函数可以帮助你指定一个套接字使用的端口。
    
    有时候你并不一定要调用bind来建立网络连接。比如你只是想连接到一个远程主机上面进行通讯，你并不在乎你究竟是用的自己机器上的哪个端口 进行通讯（比如 Telnet），那么你可以简单的直接调用connect()函数，connect()将自动寻找 出本地机器上的一个未使用的端口，然后调用 bind()来将其 socket 绑定到那个端口上。

```
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

```
#include <sys/socket.h> 
int listen(int sockfd, int backlog);
```
listen()函数的参数意义如下： 
1. sockfd 是一个套接字描述符，由socket()系统调用获得。 
2. backlog 是未经过处理的连接请求队列可以容纳的最大数目。

在每一个连入请求都要进入一个连入请求队列，等待 listen 的程序调用 accept()（accept()函数下面有介绍）函数来接受这个连接。
当系统还没有 调用 accept()函数的时候，如果有很多连接，那么本地能够等待的最大数目就是 backlog 的 数值。你可以将其设成 5 到 10之间的数值（推荐）。 

```
int accept(int sockfd, void *addr, int *addrlen); 
```
accept()函数的参数意义如下： 
1. sockfd 是正在 listen() 的一个套接字描述符。 
2. addr 一般是一个指向 struct sockaddr_in 结构的指针；
	里面存储着远程连接过来的 计算机的信息（比如远程计算机的 IP 地址和端口）。 
3. addrlen 是一个本地的整型数值，在它的地址传给 accept()前它的值应该是 sizeof(struct sockaddr_in)；accept()不会在 addr 中存储多余 addrlen bytes 大小的数据。如果 accept()函数在 addr 中存储的数据量不足 addrlen，则 accept()函数会改变 addrlen 的值来反 应这个情况。 

#send()、recv()函数
```
#include <sys/types.h>
#include <sys/socket.h>f
int send(int sockfd, const void *msg, int len, int flags); 
```
send 的参数含义如下： 
1. sockfd 是代表你与远程程序连接的套接字描述符。 
2. msg是一个指针，指向你想发送的信息的地址。 
3. len 是你想发送信息的长度。
4. flags 发送标记。一般都设为 0（你可以查看 send 的man pages 来获得其他的参数 值并且明白各个参数所代表的含义）。 

send()函数在调用后会返回它  ！！真正发送数据的长度。 
注意：send() 所发送的数据可能少于你给它的参数所指定的长度！ 
因为如果你给 send()的参数中包含的数据的长度远远大于 send()所能一次发送的数据，
则 send()函数 只发送它所能发送的最大数据长度，
然后它相信你会把剩下的数据再次调用它来进行第二次发送
```
#include <sys/types.h>
#include <sys/socket.h>
int recv(int sockfd, void *buf, int len, unsigned int flags）; 
```
recv()的参数含义如下： 
1. sockfd 是你要读取数据的套接字描述符。 
2. buf 是一个指针，指向你能存储数据的内存缓存区域。 
3. len 是缓存区的最大尺寸。
4. flags 是 recv() 函数的一个标志，一般都为0 （具体的其他数值和含义请参考 recv() 的 man pages）。 

***recv的返回值为以接受的数据的大小，最大不超过len，则需要循环去读取传递过来的信息，直到返回的数据大小小于len。***

# 面对udp的 sendto() 和 recvfrom() 函数

    进行无连接的 UDP 通讯时使用的。使用这两个函数，则数据会在没有 建立过任何连接的网络上传输。
    因为数据报套接字无法对远程主机进行连接，想想我们在 发送数据前需要知道些什么呢？ 对了！是远程主机的 IP 地址和端口！ 
```
#include <sys/types.h>
#include <sys/socket.h>
int sendto（int sockfd, const void *msg, int len, unsigned int flags, const struct sockaddr *to, int tolen）;
```
```
#include <sys/types.h>
#include <sys/socket.h>
int recvfrom(int sockfd, void *buf, int len, unsigned int flags struct sockaddr *from, int *fromlen); 
```

# close()和 shutdown()函数


     #include <unistd.h>
     close(sockfd); 

 1. 执行 close()之后，套接字将不会在允许进行读操作和写操作。任何有关对套接字描述 符进行读和写的操作都会接收到一个错误。

## shutdown()的声明为： 


     #include <sys/socket.h> 
     int shutdown(int sockfd, int how);


它的参数含义如下：
 1. sockfd 是一个你所想关闭的套接字描述符． 
 2. how 可以取下面的值。0 表示不允许以后数据的接收操；1 表示不允许以后数据的发送操作；2 表示和close()一样，不允许以后的任何操作（包括接收，发送数据）


 /* 设定参数数值 */ opt = 1; len = sizeof(opt); 
 /* 设置套接字属性 */ setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,&opt,&len);