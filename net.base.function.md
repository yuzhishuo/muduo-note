## 创建socket
```
	#include <sys/types.h>
	#include <sys/socket.h>
	int socket（int domain , int type , int protocol）;
	
	domain = AF_INET
```
	1. 成功返回 sockfd 文件描述符号。 如果发生错误， socket()函数返回 –1 。全局变量 errno 将被设置为错误代码
	2. domain 选择哪一个协议族
	3. type 指定服务类型 SOCK_STREAM （tcp、 SOCK_DGRAM (udp
		a. 在 linux core 2.6.17 起， SOCK_NONBLOCK 和 SOCK_CLOEXEC 他们建立的是非阻塞的，以及SOCK_CLOEXEC fork调用创建子进程时，在子进程关闭此socket。在2.6.17版本之前 需要用额外的系统调用设置。 如 fcntl
	2. 最后， 只需要把 protocol设置为 0 。前两个参数已经决定了该协议，因此在几乎任意情况下将其设置为0

```cpp
#include <sys/types.h>
#include <sys/socket.h>
int bind (int sockfd , struct sockaddr *my_addr , int addrlen);
```
1. bind函数可以帮助你指定一个套接字使用的端口。
    
    有时候你并不一定要调用bind来建立网络连接。比如你只是想连接到一个远程主机上面进行通讯，你并不在乎你究竟是用的自己机器上的哪个端口 进行通讯（比如 Telnet），那么你可以简单的直接调用connect()函数，connect()将自动寻找 出本地机器上的一个未使用的端口，然后调用 bind()来将其 socket 绑定到那个端口上。
2. errno
	a. EACCESS 被绑定在知名端口，权限不足访问。
	b. EADDRINUSE 被绑定的地址正在使用中