如何处理 accept(2) 返回的 EMFILE 错误

1. 调高进程文件描述符数目
2. 死等
3. 退出程序
4. 关闭监听套接字。那什么时候重新打开呢？
5. 如果是epo11模型，可以改用edge trigger。问题是如果漏掉了一次accept（2），程序再也不会收到新连接。
6. 准备一个空闲的文件描述符。遇到这种情况，先关闭这个空闲文件，获得一个文件描述符名额；再accept（2）拿到socket连接的文件描述符；随后立刻close（2），这样就优雅地断开了与客户端的连接；最后重新打开空闲文件，把“坑”填上，以备再次出现这种情况时使用。


请解释哪个最佳为什么？

6. 准备一个空闲的文件描述符。遇到这种情况，先关闭这个空闲文件，获得一个文件描述符名额；再accept（2）拿到socket连接的文件描述符；随后立刻close（2），这样就优雅地断开了与客户端的连接；最后重新打开空闲文件，把“坑”填上，以备再次出现这种情况时使用。

具体操作：
```c++
auto idlefd = open("/dev/null", o_RDONLY | O_CLOEXEC); // 一个空闲的文件描述符


// 处理过程
if(connfd == -1)
{
    if(errno == ENFILE)
    {
        close(idlefd);
        idlefd = accept(listenfd, NULL, NULL);
        close(idlefd);
        idlefd = open("/dev/null", o_RDONLY | O_CLOEXEC);
    }
}
```