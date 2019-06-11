#include <signal.h>
#include <sys/wait.h>

#include <poll.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <error.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <vector>
#include <iostream>
#include <arpa/inet.h>
#include <error.h>
#include<sys/types.h>
#include<sys/socket.h>

#define ERR_EXIT(n) \
    do \
    {   \
        perror(n);  \       
        exit(EXIT_FAILURE); \
    }while(0)

typedef std::vector<struct pollfd> PollFdList;

int main ()
{
    signal(SIGPIPE, SIG_IGN);
    signal(SIGCHLD, SIG_IGN);

    int listenfd;

    // socket(AF_INET, SOCK_STREAM, 0)
    if ((listenfd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP)) < 0)
        ERR_EXIT("SOCKET");
    
    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family =  AF_INET ;
    servaddr.sin_port = htons(5000);
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

    int on = 1;

    if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0 )
        ERR_EXIT("set sockopt");

    if (bind(listenfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0)
        ERR_EXIT("LISTEN");
    
    if(listen(listenfd, 5) < 0)
        ERR_EXIT("bind");
    
    struct pollfd pfd;
    pfd.fd = listenfd;
    pfd.events = POLLIN; 

    PollFdList pollfds;
    pollfds.push_back(pfd);

    int nready;

    int connfd;
    struct sockaddr_in peeraddr;
    socklen_t peerlen;

    while(1)
    {
        nready = poll(&*pollfds.begin(),pollfds.size(), /* 超时时间 */ -1);
        if( nready == -1 )
        {
            if(errno == EINTR)
                continue;
        }
        if( nready == 0)
        {
            continue;
        }
        // 判断是否有监听事件到来
        if( pollfds[0].revents & POLLIN)
        {
            peerlen = sizeof(peeraddr);
                    // 如果有则 接受返回一个套接字
            connfd = ::accept4(listenfd, (struct sockaddr*)&peeraddr, &peerlen,
                                SOCK_NONBLOCK | SOCK_CLOEXEC );

            if(connfd == -1)
            {
                ERR_EXIT("accept4");
            }
  
            std::cout << "aleard connect" << std::endl;
            // 加入到监听，连接
            pfd.fd = connfd;
            pfd.events = POLLIN;
            pfd.revents = 0;
            pollfds.push_back(pfd);
            --nready;

            if(nready == 0)
                continue;
        }

        // 遍历已经连接套接字
        for(auto it = pollfds.begin()+1; it < pollfds.end() and nready > 0; it++)
        {
            if(it->events && POLLIN)
            {
                --nready;
                connfd = it->fd;
                char buf[1024] = {0};
                int ret = read(connfd, buf, 1024);

                if(ret == -1)
                {
                    ERR_EXIT("read");
                }

                // 判断对方是否关闭套接字
                if(ret == 0)
                {
                    std::cout << "client close" << std::endl;
                    it = pollfds.erase(it);
                    --it;
                    close(connfd);
                    continue;
                }
                std::cout << buf;
                //echo
                write(connfd, buf, strlen(buf));
            }
        }
    }
}