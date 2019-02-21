// Use of this source code is governed by a BSD-style license
// that can be found in the License file.
//
// Author: Shuo Chen (chenshuo at chenshuo dot com)

#include <muduo/base/Thread.h>
#include <muduo/base/CurrentThread.h>
#include <muduo/base/Exception.h>
#include <muduo/base/Logging.h>

#include <boost/static_assert.hpp>
#include <boost/type_traits/is_same.hpp>
#include <boost/weak_ptr.hpp>

#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/prctl.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <linux/unistd.h>

namespace muduo
{
namespace CurrentThread
{
  // __thread 扩展关键字
  // cached
  __thread int t_cachedTid = 0;
  __thread char t_tidString[32];
  __thread int t_tidStringLength = 6;
  __thread const char* t_threadName = "unknown";
  const bool sameType = boost::is_same<int, pid_t>::value;
  BOOST_STATIC_ASSERT(sameType);
}

namespace detail
{

pid_t gettid()
{
  // 获取线程ttid ::syscall(SYS_gettid)
  return static_cast<pid_t>(::syscall(SYS_gettid));
}

void afterFork()
{
  muduo::CurrentThread::t_cachedTid = 0;
  muduo::CurrentThread::t_threadName = "main";
  CurrentThread::tid();
  // no need to call pthread_atfork(NULL, NULL, &afterFork);
}

class ThreadNameInitializer
{
 public:
  ThreadNameInitializer()
  {
    muduo::CurrentThread::t_threadName = "main";
    CurrentThread::tid();
    pthread_atfork(NULL, NULL, &afterFork);
  }
};

ThreadNameInitializer init;

struct ThreadData
{
  typedef muduo::Thread::ThreadFunc ThreadFunc;
  ThreadFunc func_;
  string name_;
  pid_t* tid_;

  CountDownLatch* latch_;
  // 为了保证 Thread::start() 之后 Thread::tid() 立刻能拿到正确的值。如果没有 latch_，会有 race condition，
  // 即调用 Thread::tid() 的时候线程还没有启动，结果返回初值 0。
  // https://www.zhihu.com/question/294270506/answer/489820748

  // 场景如下：

 /* 
   map<pid_t, unique_ptr<Thread>> threads;
  Thread* thr = new Thread(...);
  thr->start();
  threads[thr->tid()].reset(thr);
  LOG_INFO << "Started thread: " << thr->tid();
 */



  ThreadData(const ThreadFunc& func,
             const string& name,
             pid_t* tid,
             CountDownLatch* latch)
    : func_(func),
      name_(name),
      tid_(tid),
      latch_(latch)
  { }

  void runInThread()
  {
    *tid_ = muduo::CurrentThread::tid();
    tid_ = NULL;
    latch_->countDown();
    latch_ = NULL;

    muduo::CurrentThread::t_threadName = name_.empty() ? "muduoThread" : name_.c_str();
    // 为进程设置名字...
    //  #include <sys/prctl.h>
    ::prctl(PR_SET_NAME, muduo::CurrentThread::t_threadName);
    try
    {
      func_();
      muduo::CurrentThread::t_threadName = "finished";
    }
    catch (const Exception& ex)
    {
      muduo::CurrentThread::t_threadName = "crashed";
      fprintf(stderr, "exception caught in Thread %s\n", name_.c_str());
      fprintf(stderr, "reason: %s\n", ex.what());
      fprintf(stderr, "stack trace: %s\n", ex.stackTrace());
      abort();
    }
    catch (const std::exception& ex)
    {
      muduo::CurrentThread::t_threadName = "crashed";
      fprintf(stderr, "exception caught in Thread %s\n", name_.c_str());
      fprintf(stderr, "reason: %s\n", ex.what());
      abort();
    }
    catch (...)
    {
      muduo::CurrentThread::t_threadName = "crashed";
      fprintf(stderr, "unknown exception caught in Thread %s\n", name_.c_str());
      throw; // rethrow
    }
  }
};

void* startThread(void* obj)
{
  ThreadData* data = static_cast<ThreadData*>(obj);
  data->runInThread();
  delete data;
  return NULL;
}

}
}

using namespace muduo;

void CurrentThread::cacheTid()
{
  if (t_cachedTid == 0)
  {
    t_cachedTid = detail::gettid();
    t_tidStringLength = snprintf(t_tidString, sizeof t_tidString, "%5d ", t_cachedTid);
  }
}

bool CurrentThread::isMainThread()
{
  // 判断是否是主线程
  return tid() == ::getpid();
}

void CurrentThread::sleepUsec(int64_t usec)
{
  struct timespec ts = { 0, 0 };
  ts.tv_sec = static_cast<time_t>(usec / Timestamp::kMicroSecondsPerSecond);
  ts.tv_nsec = static_cast<long>(usec % Timestamp::kMicroSecondsPerSecond * 1000);

  // 毫秒睡
  ::nanosleep(&ts, NULL);
}

AtomicInt32 Thread::numCreated_;

Thread::Thread(const ThreadFunc& func, const string& n)
  : started_(false),
    joined_(false),
    pthreadId_(0),
    tid_(0),
    func_(func),
    name_(n),
    latch_(1)
{
  setDefaultName();
}

#ifdef __GXX_EXPERIMENTAL_CXX0X__
// 你看 muduo ,规避了 多值传参的问题, 什么都由你自己弄,我先乘的接口就是这样的
Thread::Thread(ThreadFunc&& func, const string& n)
  : started_(false),
    joined_(false),
    pthreadId_(0),
    tid_(0),
    // 定义了这个宏也就是移动语意
    func_(std::move(func)),
    name_(n),
    latch_(1)
{
  setDefaultName();
}

#endif

Thread::~Thread()
{
  if (started_ && !joined_)
  {
    // 如果线程即开始但是没有被加入，则默认分离线程
    pthread_detach(pthreadId_);
  }
}

void Thread::setDefaultName()
{
  int num = numCreated_.incrementAndGet();
  if (name_.empty())
  {
    char buf[32];
    snprintf(buf, sizeof buf, "Thread%d", num);
    name_ = buf;
  }
}

void Thread::start()
{
  // 防止重复调用
  assert(!started_);
  started_ = true;
  // FIXME: move(func_)
  detail::ThreadData* data = new detail::ThreadData(func_, name_, &tid_, &latch_);
  if (pthread_create(&pthreadId_, NULL, &detail::startThread, data))
  {
    started_ = false;
    delete data; // or no delete?
    LOG_SYSFATAL << "Failed in pthread_create";
  }
  else
  {
    // 此处体现了 latch_ 的作用，其作用在于，解决时间创建成功即拿到 线程id
    latch_.wait();
    assert(tid_ > 0);
  }
}

int Thread::join()
{
  assert(started_);
  assert(!joined_);
  joined_ = true;
  return pthread_join(pthreadId_, NULL);
}

