// Use of this source code is governed by a BSD-style license
// that can be found in the License file.
//
// Author: Shuo Chen (chenshuo at chenshuo dot com)

#ifndef MUDUO_BASE_MUTEX_H
#define MUDUO_BASE_MUTEX_H

#include <muduo/base/CurrentThread.h>
#include <boost/noncopyable.hpp>
#include <assert.h>
#include <pthread.h>

#ifdef CHECK_PTHREAD_RETURN_VALUE

#ifdef NDEBUG

// 我们编写代码，经常需要c和c++混合使用，为了使 C 代码和 C++ 代码保持互相兼容的过程调用接口，
// 需要在 C++ 代码里加上 extern “C” 作为符号声明的一部分，为了简化，从而定义了
// 上面的两个宏方面我们使用。
__BEGIN_DECLS
extern void __assert_perror_fail (int errnum,
                                  const char *file,
                                  unsigned int line,
                                  const char *function)
    __THROW __attribute__ ((__noreturn__));
__END_DECLS
#endif
// __builtin_expect 允许程序员将最有可能执行的分支告诉编译器
// 这个指令的写法为：__builtin_expect(EXP, N)。意思是：EXP==N的概率很大。
// 一般的使用方法是将__builtin_expect指令封装为LIKELY和UNLIKELY宏。这两个宏的写法如下。
// 
// #define LIKELY(x) __builtin_expect(!!(x), 1)   x很可能为真
// #define UNLIKELY(x) __builtin_expect(!!(x), 0) x很可能为假
#define MCHECK(ret) ({ __typeof__ (ret) errnum = (ret);         \
                       if (__builtin_expect(errnum != 0, 0))    \
                         __assert_perror_fail (errnum, __FILE__, __LINE__, __func__);})

#else  // CHECK_PTHREAD_RETURN_VALUE

#define MCHECK(ret) ({ __typeof__ (ret) errnum = (ret);         \
                       assert(errnum == 0); (void) errnum;})

#endif // CHECK_PTHREAD_RETURN_VALUE

namespace muduo
{

// Use as data member of a class, eg.
//
// class Foo
// {
//  public:
//   int size() const;
//
//  private:
//   mutable MutexLock mutex_;
//   std::vector<int> data_; // GUARDED BY mutex_
// };
class MutexLock : boost::noncopyable
{
 public:
  MutexLock()
    : holder_(0)
  {
    MCHECK(pthread_mutex_init(&mutex_, NULL));
  }

  ~MutexLock()
  {
    assert(holder_ == 0);
    MCHECK(pthread_mutex_destroy(&mutex_));
  }

  // must be called when locked, i.e. for assertion
  bool isLockedByThisThread() const
  {
    return holder_ == CurrentThread::tid();
  }

  void assertLocked() const
  {
    assert(isLockedByThisThread());
  }

  // internal usage

// 此处 提供了一个场景 用于 判断 这个锁只能用于在一个线程内使用。 先锁，在解锁 解锁的时候lock继续执行，将当前线程的id复制给此锁。
  void lock()
  {
    MCHECK(pthread_mutex_lock(&mutex_));
    assignHolder();
  }

  void unlock()
  {
    unassignHolder();
    MCHECK(pthread_mutex_unlock(&mutex_));
  }

  pthread_mutex_t* getPthreadMutex() /* non-const */
  {
    return &mutex_;
  }

 private:
  friend class Condition;

  // 因为C++类MutexLock在封装了pthread_mutex_t实例的同时，还额外引入了一个状态成员MutexLock::holder_。
  // MutexLock::lock、MutexLock::unlock等方法的内部代码会小心维护MutexLock::holder_与pthread_mutex_t实例内部状态的一致性。
  // 但当你使用pthread_cond_wait的时候，你必须把MutexLock内部的pthread_mutex_t实例暴露出来。
  // 显然pthread_cond_wait内部会修改pthread_mutex_t实例的状态，那么此时MutexLock::holder_与pthread_mutex_t实例内部状态的一致性就被破坏了，
  // 所以需要在调用pthread_cond_wait的前后添加一些代码去相应的修改MutexLock::holder_，也就是分别调用MutexLock::unassignHolder和MutexLock::assignHolder。
  // MutexLock::UnassignGuard类的作用，就是利用RAII简化对MutexLock::unassignHolder和MutexLock::assignHolder的调用。
  class UnassignGuard : boost::noncopyable
  {
   public:
    UnassignGuard(MutexLock& owner)
      : owner_(owner)
    {
      // 和MutexLock调用是相反的
      owner_.unassignHolder();
    }

    ~UnassignGuard()
    {
      owner_.assignHolder();
    }

   private:
    MutexLock& owner_;
  };

  void unassignHolder()
  {
    holder_ = 0;
  }

  void assignHolder()
  {
    holder_ = CurrentThread::tid();
  }

  pthread_mutex_t mutex_;
  pid_t holder_;
};

// Use as a stack variable, eg.
// int Foo::size() const
// {
//   MutexLockGuard lock(mutex_);
//   return data_.size();
// }
class MutexLockGuard : boost::noncopyable
{
 public:
  explicit MutexLockGuard(MutexLock& mutex)
    : mutex_(mutex)
  {
    mutex_.lock();
  }

  ~MutexLockGuard()
  {
    mutex_.unlock();
  }

 private:

  MutexLock& mutex_;
};

}

// Prevent misuse like:
// MutexLockGuard(mutex_);
// A tempory object doesn't hold the lock for long!
#define MutexLockGuard(x) error "Missing guard object name"

#endif  // MUDUO_BASE_MUTEX_H
