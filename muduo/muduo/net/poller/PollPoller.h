// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)
//
// This is an internal header file, you should not include this.

#ifndef MUDUO_NET_POLLER_POLLPOLLER_H
#define MUDUO_NET_POLLER_POLLPOLLER_H

#include <muduo/net/Poller.h>

#include <vector>

struct pollfd;

namespace muduo
{
namespace net
{

///
/// IO Multiplexing with poll(2).
///
class PollPoller : public Poller
{
 public:

  PollPoller(EventLoop* loop);
  virtual ~PollPoller();

  virtual Timestamp poll(int timeoutMs, ChannelList* activeChannels);
  virtual void updateChannel(Channel* channel);
  virtual void removeChannel(Channel* channel);

 private:
  void fillActiveChannels(int numEvents,
                          ChannelList* activeChannels) const;

		//int poll(struct pollfd fd[], nfds_t nfds, int timeout);
		//参数：
		//	1）第一个参数:一个结构数组, struct pollfd结构如下：
		//		struct pollfd 
		//		{
		//			int fd;          //文件描述符
		//			short events;    //请求的事件
		//			short revents;   //返回的事件
		//		};
  typedef std::vector<struct pollfd> PollFdList;
  PollFdList pollfds_;
};

}
}
#endif  // MUDUO_NET_POLLER_POLLPOLLER_H
