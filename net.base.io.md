# send()、recv()函数
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

```cpp
#include <unistd.h>
close(sockfd); 
```

 1. 执行 close()之后，套接字将不会在允许进行读操作和写操作。任何有关对套接字描述 符进行读和写的操作都会接收到一个错误。
 2. close 将引用计数减为一只有，引用计数为0时候，才可真正的关闭连接。
 3. 一次 fork 默认将fd加一。

shutdown()的声明为： 

```cpp
#include <sys/socket.h> 
int shutdown(int sockfd, int how);
```

它的参数含义如下：
 1. sockfd 是一个你所想关闭的套接字描述符． 
 2. how 可以取下面的值。0 表示不允许以后数据的接收操；1 表示不允许以后数据的发送操作；2 表示和close()一样，不允许以后的任何操作（包括接收，发送数据）


```cpp
 /* 设定参数数值 */   opt = 1; len = sizeof(opt); 
 /* 设置套接字属性 */ setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,&opt,&len);
 ```