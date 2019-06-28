#include <sys/types.h>
#include <sys/epoll.h> // for epoll, 
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <algorithm>
#include <fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h> // inet_ntoa,
#include <unistd.h>
#include <vector>
#include <iostream>
#define ERR_EXIT(n)         \
    do                      \
    {                       \
        perror(n);          \
        exit(EXIT_FAILURE); \
    } while (0)

typedef std::vector<struct epoll_event> EventList;

int main()
{
    signal(SIGPIPE, SIG_IGN);
    signal(SIGCHLD, SIG_IGN);

    int listenfd;

    // socket(AF_INET, SOCK_STREAM, 0)
    if ((listenfd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP)) < 0)
        ERR_EXIT("SOCKET");

    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(5000);
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

    int on = 1;

    if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0)
        ERR_EXIT("set sockopt");

    if (bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
        ERR_EXIT("LISTEN");

    if (listen(listenfd, 5) < 0)
        ERR_EXIT("bind");

    std::vector<int> clients;
    int epollfd = epoll_create1(EPOLL_CLOEXEC);

    struct epoll_event event;
    event.data.fd = listenfd;
    event.events = EPOLLIN /* | EPOLLET */;
    epoll_ctl(epollfd, EPOLL_CTL_ADD, listenfd, &event);

    EventList events{16};

    int nready;
    int connfd;
    struct sockaddr_in peeraddr;
    socklen_t peerlen;

    while (true)
    {
        nready = epoll_wait(epollfd, &*events.begin(), events.size(), -1);

        if (nready == -1)
        {
            if (errno == EINTR)
                continue;
            ERR_EXIT("epoll wait");
        }
        if (nready == 0)
            continue;
        if (nready == events.size())
            events.resize(events.size() * 2);

        for (int i = 0; i < events.size(); ++i)
        {
            if (events[i].data.fd == listenfd)
            {
                peerlen = sizeof(peeraddr);
                connfd = ::accept4(listenfd, (struct sockaddr*)&peeraddr, &peerlen,
                                SOCK_CLOEXEC |SOCK_NONBLOCK);
                if(connfd == -1)
                {
                    // EMFILE PROCESS
                    if(errno == EMFILE)
                    {
                        continue;
                    }
                    else
                    {
                        ERR_EXIT("epoll accept4");
                    }
                }

                std::cout << "ip:" << inet_ntoa(peeraddr.sin_addr) << "prot:" << ntohs(peeraddr.sin_port) << std::endl;

                clients.push_back(connfd);
                event.data.fd = connfd;
                event.events = EPOLLIN;
                epoll_ctl(epollfd, EPOLL_CTL_ADD, connfd, &event);
            }else if( events[i].events & EPOLLIN)
            {
                connfd =  events[i].data.fd;
                if(connfd < 0) continue;
                
                char buf[20] = {0};
                int ret = read(connfd, buf, 20);

                if (ret == -1)
                {
                    ERR_EXIT("read");
                }

                // 判断对方是否关闭套接字
                if (ret == 0)
                {
                    std::cout << "client close" << std::endl;
                    close(connfd);
                    epoll_ctl(epollfd, EPOLL_CTL_DEL, connfd, &event);
                    clients.erase(std::remove(clients.begin(), clients.end(), connfd), clients.end());
                    continue;
                }
                std::cout << buf;
                //echo
                write(connfd, buf, strlen(buf));
            }
        }
        
    }
}